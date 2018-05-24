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

#ifndef _SYMBOLS_H_
#define _SYMBOLS_H_

struct symbol_struct {
	short *pcm;
	int samples;
	int units;
};

extern struct symbol_struct dit_symbol;
extern struct symbol_struct dah_symbol;
extern struct symbol_struct gap_symbol;
extern struct symbol_struct bad_symbol;

extern int symbols_create(void);
extern void symbols_destroy(void);

#endif
