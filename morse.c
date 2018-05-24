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

const struct cw_encoding_struct cw_encoding[] = {
	{"A",	".-"},
	{"B",	"-..."},
	{"C",	"-.-."},
	{"D",	"-.."},
	{"E",	"."},
	{"F",	"..-."},
	{"G",	"--."},
	{"H",	"...."},
	{"I",	".."},
	{"J",	".---"},
	{"K",	"-.-"},
	{"L",	".-.."},
	{"M",	"--"},
	{"N",	"-."},
	{"O",	"---"},
	{"P",	".--."},
	{"Q",	"--.-"},
	{"R",	".-."},
	{"S",	"..."},
	{"T",	"-"},
	{"U",	"..-"},
	{"V",	"...-"},
	{"W",	".--"},
	{"X",	"-..-"},
	{"Y",	"-.--"},
	{"Z",	"--.."},
	{"1",	".----"},
	{"2",	"..---"},
	{"3",	"...--"},
	{"4",	"....-"},
	{"5",	"...."},
	{"6",	"-...."},
	{"7",	"--..."},
	{"8",	"---.."},
	{"9",	"----."},
	{"0",	"-----"},
	{"-",	"-...-"},
	{".",	".-.-.-"},
	{",",	"--..--"},
	{"/",	"-..-."},
	{"?",	"..--.."},
	{"AR",	".-.-."},
	{"SK",	"...-.-"},
};

const int n_cw = sizeof(cw_encoding) / sizeof(cw_encoding[0]);
