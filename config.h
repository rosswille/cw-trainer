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

#ifndef _CONFIG_H_
#define _CONFIG_H_

/*
 * Turn on debug printing
 */
//#define DEBUG		1

/*
 * Sets verbosity of ALSA calls (when DEBUG is enabled)
 *
 * VERBOSITY=0 shows no ALSA calls
 * VERBOSITY=1 shows only ALSA calls that return an error
 * VERBOSITY=2 shows ALSA setup calls plus errors
 * VERBOSITY=3 shows all ALSA calls (hundreds per second)
 */
#define VERBOSITY	1

/*
 * maintain queue-related statistics
 */
#define QUEUE_STATS	1

#define N_ARRAY(_a)	((int)(sizeof(_a) / sizeof((_a)[0])))

#ifdef DEBUG
#include <stdio.h>
#define DPRINTF(fmt, args...)   	fprintf(stderr, fmt, ##args)
#else
#define DPRINTF(args...)      	  	do {} while (0)
#endif

int run_flag;

struct settings_struct {
	char alsadev[32];
	double wpm;
	double tone;
	double volume;
	double rise_ms;
	double sample_rate;
	int n_chans;
};

extern struct settings_struct settings;

#endif
