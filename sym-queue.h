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

#ifndef _SYM_QUEUE_H_
#define _SYM_QUEUE_H_

#include "symbols.h"

extern int sq_init(void);
extern void sq_fini(void);
extern void sq_put(struct symbol_struct *sp);
extern struct sqe_struct *q_get(void);
extern void get_period(unsigned char *buf, int len);

#endif
