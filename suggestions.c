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



struct suggestion;

struct suggestion {
	struct suggestion *next;

	char *string;
	int weight;

	char key;	
};


static struct suggestion *suggestions;
static int total_weight;



void reset_suggestions(void)
{
	struct suggestion *ptr;
	ptr = suggestions;
	while (ptr) {
		struct suggestion *next;
		next = ptr->next;
		free(ptr->string);
		free(ptr);
		ptr = next;
	}
	suggestions = NULL;
	total_weight = 0;
}

void add_suggestion(char *text, int weight, char key, void *func)
{
	struct suggestion *new;

	new = malloc(sizeof(struct suggestion));
	if (!new)
		return;
	memset(new, 0, sizeof(struct suggestion));
	new->string = strdup(text);
	new->weight = weight;
	new->key = key;
	new->next = suggestions;
	suggestions = new;
	total_weight += weight;
}

void pick_suggestion(void)
{
	int value, running = 0;
	struct suggestion *ptr;

	if (total_weight==0 || suggestions==NULL) {
		/* zero suggestions */
		show_suggestion("");
		return;
	}
	
	value = rand() % total_weight;
	ptr = suggestions;
	while (ptr) {
		running += ptr->weight;
		if (running > value) {
			show_suggestion(ptr->string);
			return;
		}
		ptr = ptr->next;
	}
	show_suggestion("You have found a PowerTOP bug. Congratulations.");
}
