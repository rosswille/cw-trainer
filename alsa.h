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

#ifndef _ALSA_H_
#define _ALSA_H_

#define SAMPLE_RATE		48000
#define SND_PCM_TIMEOUT_MS	30
#define FORMAT			SND_PCM_FORMAT_S16_LE
#define CHANNELS		2
#define SAMPLE_SIZE		((int)sizeof(short))
#define FRAME_SIZE		(SAMPLE_SIZE * CHANNELS)
#define FRAMES_PER_PERIOD	(6 * 160)
#define PERIODS_PER_BUFFER	2
#define FRAMES_PER_BUFFER	(FRAMES_PER_PERIOD * PERIODS_PER_BUFFER)
#define PERIOD_SIZE		(FRAMES_PER_PERIOD * FRAME_SIZE)
#define BUFFER_SIZE		(FRAMES_PER_BUFFER * FRAME_SIZE)

#define US_TO_HZ(_t)		(1.0e6 / (float)(_t))

extern int alsa_init(void);
extern void alsa_fini(void);
extern void *alsa_task(void *cookie);

#endif
