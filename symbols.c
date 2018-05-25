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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/stat.h>
#include <assert.h>

#include "config.h"
#include "morse.h"
#include "alsa.h"
#include "symbols.h"

#define N_SQ	64

struct symbol_struct dit_symbol;
struct symbol_struct dah_symbol;
struct symbol_struct gap_symbol;
struct symbol_struct bad_symbol;

static void generate_symbol(struct symbol_struct *p, int units, int silent_flag)
{
	double a;
	double da;
	double volume;
	short *pcm;
	int samples;
	int rise_sample;
	int fall_sample;
	int i;

	da = 2.0 * M_PI * settings.tone / settings.sample_rate;

	samples = units * settings.sample_rate * UNIT_MS_FROM_WPM(settings.wpm) / 1000.0 + 0.5;
	rise_sample = settings.sample_rate * settings.rise_ms / 1000.0 + 0.5;
	fall_sample = samples - rise_sample;

	pcm = calloc(samples, settings.n_chans * sizeof(short));
	assert(pcm);

	if (silent_flag)
		goto done;

	volume = settings.volume * 32000.0;
	a = 0.0;
	for (i = 0; i < samples; i++) {
		double s;

		s = volume * sin(a);

		if (i < rise_sample)
			s *= (double)i / (double)rise_sample;
		else if (i >= fall_sample)
			s *= (double)((samples - 1) - i) / (double)rise_sample;

		pcm[i] = s;

		a += da;
		while (a > (2.0 * M_PI))
			a -= 2.0 * M_PI;
	}
done:
	p->pcm = pcm;
	p->samples = samples;
	p->units = units;
}

static void generate_bad_symbol(const char *fn)
{
	FILE *fp;
	char hdr[44];
	struct stat sb;
	short *pcm;
	int samples;
	int len;
	int n;
	int rc;

	rc = stat(fn, &sb);
	assert(rc == 0);

	len = sb.st_size;
	assert(len >= sizeof(hdr));

	fp = fopen(fn, "r");
	assert(fp);

	n = fread(hdr, 1, sizeof(hdr), fp);
	assert(n == sizeof(hdr));
	len -= n;

	pcm = malloc(len);
	assert(pcm);

	n = fread(pcm, 1, len, fp);
	assert(n == len);
	samples = len / ((int)sizeof(short) * 2);

	bad_symbol.pcm = pcm;
	bad_symbol.samples = samples;
	bad_symbol.units = 0;

	fclose(fp);
}

static void free_symbols(void)
{
	if (gap_symbol.pcm) {
		free(gap_symbol.pcm);
		gap_symbol.pcm = NULL;
	}
	if (dah_symbol.pcm) {
		free(dah_symbol.pcm);
		dah_symbol.pcm = NULL;
	}
	if (dit_symbol.pcm) {
		free(dit_symbol.pcm);
		dit_symbol.pcm = NULL;
	}
}

static int generate_symbols(void)
{
	free_symbols();

	generate_symbol(&dit_symbol, 1, 0);
	generate_symbol(&dah_symbol, 3, 0);
	generate_symbol(&gap_symbol, 1, 1);
	generate_bad_symbol("wrong.wav");

	return 0;
}

int symbols_create(void)
{
	dit_symbol.pcm = NULL;
	dah_symbol.pcm = NULL;
	gap_symbol.pcm = NULL;

	generate_symbols();

#if 0
	FILE *fp;
	int n;

	fp = fopen("dit.raw", "w");
	assert(fp);
	n = fwrite(dit_symbol.pcm, n_chans * sizeof(short), dit_symbol.samples, fp);
	assert(n == dit_symbol.samples);
	fclose(fp);

	fp = fopen("dah.raw", "w");
	assert(fp);
	n = fwrite(dah_symbol.pcm, n_chans * sizeof(short), dah_symbol.samples, fp);
	assert(n == dah_symbol.samples);
	fclose(fp);

	fp = fopen("gap.raw", "w");
	assert(fp);
	n = fwrite(gap_symbol.pcm, n_chans * sizeof(short), gap_symbol.samples, fp);
	assert(n == gap_symbol.samples);
	fclose(fp);
#endif
	return 0;
}

void symbols_destroy(void)
{
	free_symbols();
}

/*
 * Choose a symbol randomly based on the symbol weights.
 */
int symbol_chooser(void)
{
	int i;
	float sum;
	float rand;

	/* find the total weight */
	sum = 0.0;
	for (i = 0; i < n_cw; i++) {
		/* negative weights are not allowed */
		if (cw[i].weight < 0.0)
			cw[i].weight = 0.0;
		sum += cw[i].weight;
	}

	/*
	 * If the weights of all symbols are zero,
	 * choose any symbol at random.
	 */
	if (sum == 0.0)
		return (lrand48() % n_cw);

	/* pick a random point within the weight range */
	rand = drand48() * sum;

	/* locate the symbol containing that point */
	sum = 0.0;
	for (i = 0; i < n_cw; i++) {
		sum += cw[i].weight;
		if (rand < sum)
			break;
	}
	/*
	 * There's a slight chance of exceeding the range due
	 * to rounding, but it's not statistally significant.
	 */
	if (i >= n_cw)
		i = n_cw - 1;

	return i;
}
