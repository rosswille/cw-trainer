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
#include <linux/limits.h>

#include "config.h"
#include "morse.h"

#define CONFIG_FILE	".cw-trainer.conf"

char config_path[PATH_MAX];

static char *get_config_path(void)
{
	char *home;

	home = getenv("HOME");
	if (home)
		snprintf(config_path, sizeof(config_path), "%s/%s", home, CONFIG_FILE);
	else
		strcpy(config_path, CONFIG_FILE);

	return config_path;
}

static void normalize_weights(void)
{
	int i;
	int n;
	float sum;
	float scale;

	n = 0;
	sum = 0.0;
	for (i = 0; i < n_cw; i++) {
		if (cw[i].weight < 0.0)
			cw[i].weight = 0.0;
		if (cw[i].weight != 0.0) {
			sum += cw[i].weight;
			n++;
		}
	}
	if (n) {
		scale = (float)n / sum;
		for (i = 0; i < n_cw; i++) {
			if (cw[i].weight != 0.0)
				cw[i].weight *= scale;
		}
	}
	else {
		for (i = 0; i < n_cw; i++) {
			cw[i].weight = 1.0;
		}
	}
}

void config_read(void)
{
	FILE *fp;
	char line[128];
	int i;

	fp = fopen(get_config_path(), "r");
	if (fp == NULL)
		return;

	while (fgets(line, sizeof(line), fp)) {
		char *p;

		if ((line[0] == '#') || (line[0] == ' ') || (line[0] == '\n'))
			continue;
		p = strchr(line, ' ');
		if (!p)
			continue;
		*p++ = '\0';
		for (i = 0; i < n_cw; i++) {
			if (strcmp(cw[i].symbol, line) == 0) {
				cw[i].weight = atof(p);
				break;
			}
		}
	}
	fclose(fp);

	normalize_weights();
}

void config_write(void)
{
	FILE *fp;
	int i;

	normalize_weights();

	fp = fopen(get_config_path(), "w");
	if (fp == NULL) {
		fprintf(stderr, "Cannot write config file %s\r\n", get_config_path());
		return;
	}

	for (i = 0; i < n_cw; i++) {
		if (cw[i].weight > 0.0)
			fprintf(fp, "%-4s%0.5f\n", cw[i].symbol, cw[i].weight);
		else
			fprintf(fp, "%-4s0\n", cw[i].symbol);
	}
	fclose(fp);
}

