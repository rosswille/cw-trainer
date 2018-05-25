/*
 * Copyright (C) 2018 by Ross Wille. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * COPYING file for more details.
 */

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <assert.h>

#include "config.h"
#include "alsa.h"
#include "morse.h"
#include "sym-queue.h"
#include "symbols.h"
#include "threads.h"
#include "tty.h"

struct settings_struct settings;
int run_flag;
int help_flag;

static pthread_t alsa_thread;
static pthread_t worker_thread;

static int worker_init(void)
{
	return 0;
}

static void worker_fini(void)
{
}

static void *worker_task(void *cookie)
{
	usleep(THREAD_STARTUP_DELAY_US);

	while (run_flag) {
		sleep(1);
	}
	return NULL;
}

static int start_threads(void)
{
	pthread_attr_t io_attr;
	pthread_attr_t wk_attr;
	struct sched_param io_param;
	struct sched_param wk_param;
	int rc;

	pthread_attr_init(&io_attr);
	pthread_attr_getschedparam(&io_attr, &io_param);
	io_param.sched_priority = IO_PRIORITY;
	pthread_attr_setschedpolicy(&io_attr, IO_SCHED);
	pthread_attr_setschedparam(&io_attr, &io_param);

	pthread_attr_init(&wk_attr);
	pthread_attr_getschedparam(&wk_attr, &wk_param);
	wk_param.sched_priority = WORK_PRIORITY;
	pthread_attr_setschedpolicy(&wk_attr, WK_SCHED);
	pthread_attr_setschedparam(&wk_attr, &wk_param);

	do {
		rc = pthread_create(&worker_thread, &wk_attr, &worker_task, (void *)2);
		if (rc) break;

		rc = pthread_create(&alsa_thread, &io_attr, &alsa_task, (void *)3);
		if (rc) break;

		rc = pthread_setschedparam(worker_thread, WK_SCHED, &wk_param);
		if (rc) break;

		rc = pthread_setschedparam(alsa_thread, IO_SCHED, &io_param);
		if (rc) break;
	} while (0);

	return rc;
}

static void join_threads(void)
{
	pthread_join(worker_thread, NULL);
	pthread_join(alsa_thread, NULL);
}

static void queue_cw(int index)
{
	char *p;

	for (p = cw[index].cw; *p; p++) {
		switch (*p) {
		case '.':
			sq_put(&gap_symbol);
			sq_put(&dit_symbol);
			break;
		case '-':
			sq_put(&gap_symbol);
			sq_put(&dah_symbol);
			break;
		default:
			break;
		}
	}
}

static void show_help(void)
{
	printf("cw-trainer [options...]\n");
	printf("  -c, --channels=#\n\t\tNumber of audio channels [default=%d]\n\n", settings.n_chans);
	printf("  -D, --device=NAME\n\t\tSelect PCM by name [default=%s]\n\n", settings.alsadev);
	printf("  -h, --help\n\t\tHelp: show syntax\n\n");
	printf("  -r, --rise=#\n\t\tRise time (milliseconds) [default=%0.1lf]\n\n", settings.rise_ms);
	printf("  -s, --sample-rate=#<hz>\n\t\tSample rate [default=%0.0lf]\n\n", settings.sample_rate);
	printf("  -t, --tone=#<hz>\n\t\tTone frequency [default=%0.0lf]\n\n", settings.tone);
	printf("  -v, --volume=#\n\t\tVolume, 0.0 to 1.0 [default=%0.1lf]\n\n", settings.volume);
	printf("  -w, --wpm=#\n\t\tWords per Minute [default=%0.1lf]\n\n", settings.wpm);
}

int main(int argc, char *argv[])
{
	unsigned char kbd_buf[16];
	int sym;
	int n;
	int c;

	help_flag = 0;
	strcpy(settings.alsadev, "hw:0,0");
	settings.wpm = 13.0;
	settings.tone = 800.0;
	settings.volume = 0.8;
	settings.rise_ms = 5.0;
	settings.sample_rate = 48000;
	settings.n_chans = 2;

	config_read();

	while (1) {
		static struct option long_options[] = {
			{"channels", required_argument, 0, 'c'},
			{"device", required_argument, 0, 'D'},
			{"help", no_argument, 0, 'h'},
			{"rise", required_argument, 0, 'r'},
			{"sample-rate", required_argument, 0, 's'},
			{"tone", required_argument, 0, 't'},
			{"volume", required_argument, 0, 'v'},
			{"wpm", required_argument, 0, 'w'},
			{0, 0, 0, 0}
		};
		int option_index = 0;

		c = getopt_long(argc, argv, "D:ht:v:w:", long_options, &option_index);
		if (c == -1)
			break;

		switch(c) {
		case 0:
			if (long_options[option_index].flag)
				break;
			printf("option %s", long_options[option_index].name);
			if (optarg)
				printf(" with arg %s\n", optarg);
			printf("\n");
			break;
		case 'c':
			settings.n_chans = atoi(optarg);
			break;
		case 'D':
			strncpy(settings.alsadev, optarg, sizeof(settings.alsadev));
			break;
		case 'h':
			help_flag = 1;
			break;
			
		case 'r':
			settings.rise_ms = atof(optarg);
			break;
		case 's':
			settings.sample_rate = atoi(optarg);
			break;
		case 't':
			settings.tone = atof(optarg);
			break;
		case 'v':
			settings.volume = atof(optarg);
			break;
		case 'w':
			settings.wpm = atof(optarg);
			break;
		default:
			printf("invalid option: %c\n", c);
			exit(1);
		}
	}
	if (help_flag) {
		show_help();
		exit(0);
	}

	srand48(time(NULL) ^ (getpid() << 16));

	symbols_create();
	tty_init();
	sq_init();
	alsa_init();
	worker_init();

	run_flag = 1;
	start_threads();

	while (run_flag) {
		sym = symbol_chooser();
		DPRINTF("Chose symbol '%s' Weight=%0.5f\r\n", cw[sym].symbol, cw[sym].weight);
again:		queue_cw(sym);

		while (1) {
			n = tty_read(kbd_buf, 1);
			if (n == 0) {
				printf("TTY timeout\r\n");
				usleep(20000);
				continue;
			}
			else if (n < 0) {
				printf("TTY error\r\n");
				run_flag = 0;
			}
			break;
		}
		c = kbd_buf[0];
		if ((c == '\033') || (c == '\003')) {
			run_flag = 0;
			printf("Quitting\r\n");
			break;
		}
		else if (c == ' ') {
			cw[sym].weight *= AGAIN_SCALE;
			goto again;
		}

		if (toupper(c) == cw[sym].symbol[0]) {
			cw[sym].weight *= RIGHT_SCALE;
			printf("Right! %s\r\n", cw[sym].symbol);
		}
		else {
			cw[sym].weight *= WRONG_SCALE;
			sq_put(&bad_symbol);
			printf("Wrong! %s\r\n", cw[sym].symbol);
		}
	}

	join_threads();

	worker_fini();
	alsa_fini();
	sq_fini();
	tty_fini();
	symbols_destroy();

	config_write();

	return 0;
}
