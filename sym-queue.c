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

#include <pthread.h>
#include <string.h>
#include <assert.h>

#include "config.h"
#include "alsa.h"
#include "symbols.h"
#include "sym-queue.h"

#define N_SQ	64

#define LOCK(_s)	pthread_mutex_lock(&(_s).lock)
#define UNLOCK(_s)	pthread_mutex_unlock(&(_s).lock)

#define SQ_INCR(_c)	sq._c = (sq._c + 1) & (N_SQ - 1)
#define SQ_DECR(_c)	sq._c = (sq._c - 1) & (N_SQ - 1)

struct sqe_struct {
	struct symbol_struct *sym;
	char *buf;
	int remain;
};

struct sq_struct {
	struct sqe_struct sqe[N_SQ];
	int head;
	int tail;
	int entries;
	int empty;
	pthread_mutex_t lock;
} sq;

int sq_init(void)
{
	int i;

	pthread_mutex_init(&sq.lock, NULL);

	LOCK(sq);

	for (i = 0; i < N_SQ; i++) {
		sq.sqe[i].sym = NULL;
	}
	sq.head = sq.tail = 0;
	sq.entries = 0;
	sq.empty = N_SQ;

	UNLOCK(sq);

	return 0;
}

void sq_fini(void)
{
}

/*
 * This adds a gap symbol to the queue.
 * CALLER MUST BE HOLDING THE SQ LOCK!!!
 */

static void _sq_put_gap(void)
{
	struct sqe_struct *tail;

	assert(sq.empty > 0);

	tail = &sq.sqe[sq.tail];
	tail->sym = &gap_symbol;
	tail->buf = (void *)gap_symbol.pcm;
	tail->remain = gap_symbol.samples * FRAME_SIZE;

	SQ_INCR(tail);
	sq.entries++;
	sq.empty--;
}

void sq_put(struct symbol_struct *sp)
{
	struct sqe_struct *tail;

	LOCK(sq);

	assert(sq.empty > 0);

	tail = &sq.sqe[sq.tail];
	tail->sym = sp;
	tail->buf = (void *)sp->pcm;
	tail->remain = sp->samples * FRAME_SIZE;

	SQ_INCR(tail);
	sq.entries++;
	sq.empty--;

	UNLOCK(sq);
}

static inline void _sq_drop(void)
{
	if (sq.entries) {
		SQ_INCR(head);
		sq.entries--;
		sq.empty++;
	}
}

#if 0	// unused
static void sq_drop(void)
{
	LOCK(sq);

	_sq_drop();

	UNLOCK(sq);
}
#endif

void get_period(unsigned char *buf, int len)
{
	struct sqe_struct *head;
	int n;

	LOCK(sq);

	while (len) {
		/* put gap into an empty queue */
		if (sq.entries == 0)
			_sq_put_gap();

		/* point to head of the queue */
		head = &sq.sqe[sq.head];
		n = (head->remain < len) ? head->remain : len;
		memcpy(buf, head->buf, n);
		buf += n;
		head->buf += n;
		head->remain -= n;
		len -= n;

		/* drop entry if it has been exhausted */
		if (head->remain == 0)
			_sq_drop();
	}

	UNLOCK(sq);
}
