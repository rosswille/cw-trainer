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
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>
#include <errno.h>

#include "config.h"

#define TTY			"/dev/tty"
//#define BAUD			B115200

static int tty;
static struct termios term_save;

int tty_init(void)
{
	struct termios term;

	tty = open(TTY, O_RDWR | O_NOCTTY /* | O_NONBLOCK */);
	assert(tty != -1);

	if (tcgetattr(tty, &term) != 0) {
		DPRINTF("tcgetattr error\n");
		return 1;
	}
	term_save = term;	/* save original settings */

	/* input modes */
	term.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | INPCK | ISTRIP | INLCR | IGNCR |
		ICRNL | IXON | IXOFF);
	term.c_iflag |= IGNPAR;

	/* output modes */
	term.c_oflag &= ~(OPOST | ONLCR | OCRNL | ONOCR | ONLRET | OFILL | OFDEL | NLDLY |
		 CRDLY | TABDLY | BSDLY | VTDLY | FFDLY);

	/* control modes */
	term.c_cflag &= ~(CSIZE | PARENB | PARODD | HUPCL | CRTSCTS);
	term.c_cflag |= CREAD | CS8 | CSTOPB | CLOCAL;

	/* local modes */
	term.c_lflag &= ~(ISIG | ICANON | IEXTEN | ECHO);
	term.c_lflag |= NOFLSH;

#if 0
	/* set baud rate */
	cfsetospeed(&term, BAUD);
	cfsetispeed(&term, BAUD);
#endif

	/* set new device settings */
	if (tcsetattr(tty, TCSANOW, &term) != 0) {
		DPRINTF("tcsetattr error\n");
		return 2;
	}

	return 0;
}

void tty_fini(void)
{
	tcsetattr(tty, TCSANOW, &term_save);
}

int tty_read(unsigned char *buf, int len)
{
	int n;
	int rc = len;

	while (len) {
		n = read(tty, buf, len);
		if (n == -1) {
			rc = (errno == EAGAIN) ? 0 : -1;
			break;
		}
		len -= n;
		buf += n;
		if (len > 0)
			usleep(100);
	}

	return rc;
}

#ifdef MAIN
int main(int argc, char *argv[])
{
	unsigned char tmp_buf[16];
	int c;
	int n;
	int rc;

	rc = tty_init();
	assert(rc == 0);

	while (1) {
		n = tty_read(tmp_buf, 1);
		if (n == 0) {
			usleep(20000);
			continue;
		}
		else if (n != 1) {
			printf("tty_read returned %d [errno=%d]\n", n, errno);
			perror("read");
			break;
		}
		c = tmp_buf[0];
		if (isprint(c))
			printf("'%c' [%02x]\r\n", c, c);
		else
			printf("??? [%02x]\r\n", c);
		if (c == '\033')
			break;
	}
	tty_fini();
	close(tty);

	return 0;
}
#endif
