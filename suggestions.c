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



char suggestion_key;
suggestion_func *suggestion_activate; 

struct suggestion;

struct suggestion {
	struct suggestion *next;

	char *string;
	int weight;

	char key;
	char *keystring;	

	suggestion_func *func;
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
		free(ptr->keystring);
		free(ptr);
		ptr = next;
	}
	suggestions = NULL;
	total_weight = 0;
}

void add_suggestion(char *text, int weight, char key, char *keystring, suggestion_func *func)
{
	struct suggestion *new;

	if (!text)
		return;

	new = malloc(sizeof(struct suggestion));
	if (!new)
		return;
	memset(new, 0, sizeof(struct suggestion));
	new->string = strdup(text);
	new->weight = weight;
	new->key = key;
	if (keystring)
		new->keystring = strdup(keystring);
	new->next = suggestions;
	new->func = func;
	suggestions = new;
	total_weight += weight;
}

void pick_suggestion(void)
{
	int value, running = 0;
	struct suggestion *ptr;

	strcpy(status_bar_slots[9],"");
	suggestion_key = 255;
	suggestion_activate = NULL;

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
			if (ptr->keystring)
				strcpy(status_bar_slots[9],ptr->keystring);
			suggestion_key = ptr->key;
			suggestion_activate = ptr->func;
			show_suggestion(ptr->string);
			return;
		}
		ptr = ptr->next;
	}
	show_suggestion("You have found a PowerTOP bug. Congratulations.");
}
