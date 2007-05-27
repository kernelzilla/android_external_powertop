/*
 * Copyright 2007, Intel Corporation
 *
 * This file is part of PowerTOP
 *
 * This program file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program in a file named COPYING; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * Authors:
 * 	Arjan van de Ven <arjan@linux.intel.com>
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <dirent.h>

#include "powertop.h"

char process_to_kill[1024];

void do_kill(void)
{
	char line[2048];
	sprintf(line, "killall %s &> /dev/null", process_to_kill);
	system(line);
}

void suggest_process_death(char *process_match, char *tokill, struct line *lines, int linecount, double minwakeups, char *comment, int weight)
{
	int i;

	for (i = 0; i < linecount; i++) {
		if (strstr(lines[i].string, process_match)) {
			char hotkey_string[300];
			sprintf(hotkey_string, " K - kill %s ", tokill);
			strcpy(process_to_kill, tokill);
			if (minwakeups < lines[i].count)
				add_suggestion(comment, weight, 'K' , hotkey_string, do_kill);
			break;
		}
	}
	fflush(stdout);
}
