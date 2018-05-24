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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <alsa/asoundlib.h>
#include <math.h>

#include "config.h"
#include "sym-queue.h"
#include "threads.h"
#include "alsa.h"

#ifdef DEBUG

#if (VERBOSITY == 0)
#define SND_SETUP(fn, err, val)		do {} while (0)
#define SND_IO(fn, err, val)		do {} while (0)
#endif

#if (VERBOSITY == 1)
#define SND_SETUP(fn, err, val)		do {if (err < 0) snd_show(#fn, err, (int)val);} while (0)
#define SND_IO(fn, err, val)		do {if (err < 0) snd_show(#fn, err, (int)val);} while (0)
#endif

#if (VERBOSITY == 2)
#define SND_SETUP(fn, err, val)		snd_show(#fn, err, (int)val)
#define SND_IO(fn, err, val)		do {if (err < 0) snd_show(#fn, err, (int)val);} while (0)
#endif

#if (VERBOSITY >= 3)
#define SND_SETUP(fn, err, val)		snd_show(#fn, err, (int)val)
#define SND_IO(fn, err, val)		snd_show(#fn, err, (int)val)
#endif

#define SND_ERR(fn, err, val)		snd_err(#fn, err, (int)val)
#define SND_IGN_VAL			(-1234)  // nonsense value

static void snd_show(const char *fn, int err, int val)
{
	if (err == 0) {
		if (val == SND_IGN_VAL)
			fprintf(stderr, "%s() Success\n", fn);
		else
			fprintf(stderr, "%s(%d) Success\n", fn, val);
	}
	else if (err < 0) {
		if (val == SND_IGN_VAL)
			fprintf(stderr, "Error: %s() err=%d (%s)\n", fn, err, snd_strerror(err));
		else
			fprintf(stderr, "Error: %s(%d) err=%d (%s)\n", fn, val, err, snd_strerror(err));
	}
	else {
		if (val == SND_IGN_VAL)
			fprintf(stderr, "%s() returned=%d\n", fn, err);
		else
			fprintf(stderr, "%s(%d) returned=%d\n", fn, val, err);
	}
}
#else

#define SND_SETUP(fn, rc, val)		do {} while (0)
#define SND_IO(fn, rc, val)		do {} while (0)

#endif

#define ALSADEV			"hw:0,3"
#define SUB_DIR_EXACT		0

#define QUEUE_SLOTS		4
#define QUEUE_SIZE		(QUEUE_SLOTS * PERIOD_SIZE)

#ifdef QUEUE_STATS
#define QSTAT_HIST(_q)		(_q).qs.level_cnts[(_q).len / PERIOD_SIZE]++
#define QSTAT_OVERRUN(_q)	(_q).qs.overrun_cnt++
#define QSTAT_UNDERRUN(_q)	(_q).qs.underrun_cnt++
#else
#define QSTAT_HIST(_q)		do {} while (0)
#define QSTAT_OVERRUN(_q)	do {} while (0)
#define QSTAT_UNDERRUN(_q)	do {} while (0)
#endif

#define QINCP(_q, _p)		do {(_q)._p += PERIOD_SIZE; \
				    if (((_q)._p - (_q).buf) == QUEUE_SIZE) \
					(_q)._p = (_q).buf;} \
				while (0)
#define QINCLEN(_q)		do {(_q).len += PERIOD_SIZE; QSTAT_HIST(_q);} while (0)
#define QDECLEN(_q)		do {(_q).len -= PERIOD_SIZE; QSTAT_HIST(_q);} while (0)

#define XRUN_NONE		0
#define XRUN_OVERRUN		1
#define XRUN_UNDERRUN		2

static snd_pcm_t *pdev;

static pthread_mutex_t play_lock = PTHREAD_MUTEX_INITIALIZER;

#ifdef QUEUE_STATS
struct queue_stats {
	unsigned int level_cnts[QUEUE_SLOTS];
	unsigned int underrun_cnt;
	unsigned int overrun_cnt;
	unsigned int xrun_cnt;
};
#endif

struct queue_struct {
	pthread_mutex_t lock;
	pthread_cond_t wakeup;
	unsigned char buf[QUEUE_SIZE];
	unsigned char *head;
	unsigned char *tail;
	unsigned int len;
#ifdef QUEUE_STATS
	struct queue_stats qs;
#endif
};

static struct queue_struct pq;		/* playback queue */

#ifdef QUEUE_STATS
static void print_qstats(struct queue_stats *qs)
{
	int i;

	printf("  queue-level histogram:\n");
	for (i = 0; i < QUEUE_SLOTS; i++) {
		printf("       level%d: %d\n", i, qs->level_cnts[i]);
	}
	printf("    underruns: %d\n", qs->underrun_cnt);
	printf("     overruns: %d\n", qs->overrun_cnt);
	printf("      hw_xrun: %d\n", qs->xrun_cnt);
	printf("\n");
}
#endif

static int alsa_setup_hw(snd_pcm_t *adev)
{
	snd_pcm_hw_params_t *hw_params;
	int rc = 0;

	do {
		snd_pcm_hw_params_alloca(&hw_params);

		rc = snd_pcm_hw_params_any(adev, hw_params);
		SND_SETUP(snd_pcm_hw_params_any, rc, SND_IGN_VAL);
		if (rc) break;

		rc = snd_pcm_hw_params_set_periods_integer(adev, hw_params);
		SND_SETUP(snd_pcm_hw_params_set_periods_integer, rc, SND_IGN_VAL);
		if (rc) break;

		rc = snd_pcm_hw_params_set_access(adev, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
		SND_SETUP(snd_pcm_hw_params_set_access, rc, SND_PCM_ACCESS_RW_INTERLEAVED);
		if (rc) break;

		rc = snd_pcm_hw_params_set_format(adev, hw_params, SND_PCM_FORMAT_S16_LE);
		SND_SETUP(snd_pcm_hw_params_set_format, rc, SND_PCM_FORMAT_S16_LE);
		if (rc) break;

		rc = snd_pcm_hw_params_set_rate(adev, hw_params, SAMPLE_RATE, SUB_DIR_EXACT);
		SND_SETUP(snd_pcm_hw_params_set_rate, rc, SAMPLE_RATE);
		if (rc) break;

		rc = snd_pcm_hw_params_set_channels(adev, hw_params, CHANNELS);
		SND_SETUP(snd_pcm_hw_params_set_channels, rc, CHANNELS);
		if (rc) break;

		rc = snd_pcm_hw_params_set_period_size(adev, hw_params, FRAMES_PER_PERIOD, SUB_DIR_EXACT);
		SND_SETUP(snd_pcm_hw_params_set_period_size, rc, FRAMES_PER_PERIOD);
		if (rc) break;

		rc = snd_pcm_hw_params_set_periods(adev, hw_params, PERIODS_PER_BUFFER, SUB_DIR_EXACT);
		SND_SETUP(snd_pcm_hw_params_set_periods, rc, FRAMES_PER_BUFFER);
		if (rc) break;

		rc = snd_pcm_hw_params(adev, hw_params);
		SND_SETUP(snd_pcm_hw_params, rc, SND_IGN_VAL);
		if (rc) break;
	} while (0);

#ifdef DEBUG
	{
		snd_pcm_uframes_t frames;
		int dir;
		int val;
		unsigned int time;

		val = snd_pcm_hw_params_can_sync_start(hw_params);
		DPRINTF("snd_pcm_hw_params_can_sync_start: %d\n", val);

		snd_pcm_hw_params_get_period_size(hw_params, &frames, &dir);
		DPRINTF("snd_pcm_hw_params_get_period_size: %d, dir=%d\n", (int)frames, dir);

		snd_pcm_hw_params_get_period_time(hw_params, &time, &dir);
		DPRINTF("snd_pcm_hw_params_get_period_time: %u uS (%0.2f Hz), dir=%d\n",
			time, US_TO_HZ(time), dir);

		snd_pcm_hw_params_get_buffer_size(hw_params, &frames);
		DPRINTF("snd_pcm_hw_params_get_buffer_size: %d\n", (int)frames);

		snd_pcm_hw_params_get_buffer_time(hw_params, &time, &dir);
		DPRINTF("snd_pcm_hw_params_get_buffer_time: %u uS (%0.2f Hz), dir=%d\n",
			time, US_TO_HZ(time), dir);
	}
#endif
	return rc;
}

static int alsa_setup_sw(snd_pcm_t *adev)
{
	snd_pcm_sw_params_t *sw_params;
	int rc = 0;

	do {
		snd_pcm_sw_params_alloca(&sw_params);

		rc = snd_pcm_sw_params_current(adev, sw_params);
		SND_SETUP(snd_pcm_sw_params_current, rc, SND_IGN_VAL);
		if (rc) break;

		rc = snd_pcm_sw_params_set_start_threshold(adev, sw_params, 0);
		// was FRAMES_PER_PERIOD
		SND_SETUP(snd_pcm_sw_params_set_start_threshold, rc, 0);
		if (rc) break;

		rc = snd_pcm_sw_params_set_stop_threshold(adev, sw_params, FRAMES_PER_BUFFER);
		SND_SETUP(snd_pcm_sw_params_set_stop_threshold, rc, FRAMES_PER_BUFFER);
		if (rc) break;

		rc = snd_pcm_sw_params_set_silence_size(adev, sw_params, 0);
		SND_SETUP(snd_pcm_sw_params_set_silence_size, rc, 0);
		if (rc) break;

		// don't set silence_threshold when silence_size = 0
		//snd_pcm_sw_params_set_silence_threshold(adev, sw_params, FRAMES_PER_BUFFER);

		rc = snd_pcm_sw_params_set_avail_min(adev, sw_params, FRAMES_PER_PERIOD);
		SND_SETUP(snd_pcm_sw_params_set_avail_min, rc, FRAMES_PER_PERIOD);
		if (rc) break;

		rc = snd_pcm_sw_params(adev, sw_params);
		SND_SETUP(snd_pcm_sw_params, rc, SND_IGN_VAL);
		if (rc) break;

	} while (0);

	return rc;
}

static int alsa_setup(void)
{
	int rc = 0;

	do {
		rc = snd_pcm_open(&pdev, ALSADEV, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
		SND_SETUP(snd_pcm_open, rc, SND_PCM_STREAM_PLAYBACK);
		if (rc < 0) break;
		rc = snd_pcm_nonblock(pdev, 0);
		SND_SETUP(snd_pcm_nonblock, rc, 0);

		rc = alsa_setup_hw(pdev);
		if (rc < 0) break;

		rc = alsa_setup_sw(pdev);
		if (rc < 0) break;

#if 0
		rc = snd_pcm_prepare(pdev);
		SND_SETUP(snd_pcm_prepare, rc, SND_IGN_VAL);
		if (rc < 0) break;
#endif
	} while (0);

	return rc;
}

static int alsa_start(void)
{
	int rc;

	rc = snd_pcm_prepare(pdev);
	SND_SETUP(snd_pcm_prepare, rc, SND_IGN_VAL);

	return rc;
}

static int alsa_stop(void)
{
	int rc;

	rc = snd_pcm_drop(pdev);
	return rc;
}

static int alsa_restart(void)
{
	alsa_stop();
	alsa_start();

	return 0;
}

static int alsa_close(void)
{
	snd_pcm_close(pdev);

	return 0;
}

static int alsa_xrun_recovery(void)
{
	static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
	int rc;

	if (pthread_mutex_trylock(&lock) != 0) {
		/* already locked */
		DPRINTF("already in recovery\n");
		usleep(1000);
		return 1;
	}
	/* recovery lock held */

	pthread_mutex_lock(&play_lock);

	rc = alsa_restart();

	pthread_mutex_unlock(&play_lock);

	pthread_mutex_unlock(&lock);

	return rc;
}

static void queue_init(struct queue_struct *q)
{
	memset(q, 0, sizeof(*q));
	pthread_mutex_init(&q->lock, NULL);
	pthread_cond_init(&q->wakeup, NULL);
	q->head = q->buf;
	q->tail = q->buf;
}

static void queue_destroy(struct queue_struct *q)
{
	pthread_mutex_destroy(&q->lock);
	pthread_cond_destroy(&q->wakeup);
	q->head = NULL;
	q->tail = NULL;
	q->len = 0;
}

int alsa_init(void)
{
	int rc;

	queue_init(&pq);

	rc = alsa_setup();
	if (rc < 0) {
		fprintf(stderr, "unable to setup alsa: %s\n", snd_strerror(rc));
		return -1;
	}

	rc = alsa_start();
	if (rc < 0) {
		fprintf(stderr, "unable to start alsa: %s\n", snd_strerror(rc));
		return -1;
	}
	return 0;
}

void alsa_fini(void)
{
	alsa_stop();
	alsa_close();
	queue_destroy(&pq);
}

void *alsa_task(void *cookie)
{
	usleep(THREAD_STARTUP_DELAY_US);

	while (run_flag) {
		int xrun;
		int rc;

		pthread_mutex_lock(&play_lock);
		rc = snd_pcm_wait(pdev, SND_PCM_TIMEOUT_MS);
		pthread_mutex_unlock(&play_lock);
		SND_IO(snd_pcm_wait-p, rc, SND_PCM_TIMEOUT_MS);
		if (rc == 0)
			DPRINTF("snd_pcm_wait(playback): timeout\n");

		LOCK(pq);

		if (pq.len == 0) {
			get_period(pq.head, PERIOD_SIZE);
			QINCP(pq, tail);
			QINCLEN(pq);
		}

		xrun = 0;
		pthread_mutex_lock(&play_lock);
		rc = snd_pcm_writei(pdev, pq.head, FRAMES_PER_PERIOD);
		pthread_mutex_unlock(&play_lock);
		SND_IO(snd_pcm_writei, rc, FRAMES_PER_PERIOD);
		if (rc == -EPIPE) {
			xrun = XRUN_UNDERRUN;
			pq.qs.xrun_cnt++;
			alsa_xrun_recovery();
		}

		/* dequeue period */
		if (pq.len) {
			QINCP(pq, head);
			QDECLEN(pq);
		}

		UNLOCK(pq);

		if (xrun)
			DPRINTF("alsa_task: xrun\n");
	}
	return NULL;
}
