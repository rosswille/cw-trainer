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

#include "morse.h"

/*
 * International Morse Code
 *
 * A DIT is 1 unit.
 * A DAH is 3 units.
 * The space between symbols (DITs and DAHs) is 1 unit.
 * The space between letters is 3 units.
 * The space between words is 7 units.
 *
 * Words per Minute (WPM): "PARIS" is a standard "word".
 *   P    A  R   I  S
 *   .--. .- .-. .. ...|
 *   2441323324132132217 = 50 units
 *
 * Unit(ms) = 1200 / Speed(wpm)
 */

struct cw_struct cw[] = {
	{"A",	".-",		1.0},
	{"B",	"-...",		1.0},
	{"C",	"-.-.",		1.0},
	{"D",	"-..",		1.0},
	{"E",	".",		1.0},
	{"F",	"..-.",		1.0},
	{"G",	"--.",		1.0},
	{"H",	"....",		1.0},
	{"I",	"..",		1.0},
	{"J",	".---",		1.0},
	{"K",	"-.-",		1.0},
	{"L",	".-..",		1.0},
	{"M",	"--",		1.0},
	{"N",	"-.",		1.0},
	{"O",	"---",		1.0},
	{"P",	".--.",		1.0},
	{"Q",	"--.-",		1.0},
	{"R",	".-.",		1.0},
	{"S",	"...",		1.0},
	{"T",	"-",		1.0},
	{"U",	"..-",		1.0},
	{"V",	"...-",		1.0},
	{"W",	".--",		1.0},
	{"X",	"-..-",		1.0},
	{"Y",	"-.--",		1.0},
	{"Z",	"--..",		1.0},
	{"1",	".----",	1.0},
	{"2",	"..---",	1.0},
	{"3",	"...--",	1.0},
	{"4",	"....-",	1.0},
	{"5",	"....",		1.0},
	{"6",	"-....",	1.0},
	{"7",	"--...",	1.0},
	{"8",	"---..",	1.0},
	{"9",	"----.",	1.0},
	{"0",	"-----",	1.0},
	{"-",	"-...-",	1.0},
	{".",	".-.-.-",	1.0},
	{",",	"--..--",	1.0},
	{"/",	"-..-.",	1.0},
	{"?",	"..--..",	1.0},
	{"AR",	".-.-.",	1.0},
	{"SK",	"...-.-",	1.0},
};

const int n_cw = sizeof(cw) / sizeof(cw[0]);
