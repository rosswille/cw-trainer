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

#ifndef _MORSE_H_
#define _MORSE_H_

#define UNIT_MS_FROM_WPM(_wpm)	(1200.0 / (_wpm))

struct cw_struct {
	char *symbol;
	char *cw;
	float weight;
};

extern struct cw_struct cw[];
extern const int n_cw;

#endif
