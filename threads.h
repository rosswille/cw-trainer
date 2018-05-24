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

#ifndef _THREADS_H_
#define _THREADS_H_

#define LOCK(_s)		pthread_mutex_lock(&(_s).lock)
#define UNLOCK(_s)		pthread_mutex_unlock(&(_s).lock)
#define LOCKP(_p)		pthread_mutex_lock(&(_p)->lock)
#define UNLOCKP(_p)		pthread_mutex_unlock(&(_p)->.lock)

#define IO_SCHED		SCHED_FIFO
#define WK_SCHED		SCHED_RR

#define IO_PRIORITY		45
#define WORK_PRIORITY		40

#define THREAD_STARTUP_DELAY_US	50000

#endif
