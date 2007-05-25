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


static char wireless_nic[32];


static int need_wireless_suggest(char *iface)
{
	FILE *file;
	char line[1024];
	int ret = 0;

	sprintf(line, "/sbin/iwpriv %s get_power 2> /dev/null", iface);
	file = popen(line, "r");
	if (!file)
		return 0;
	while (!feof(file)) {
		memset(line, 0, 1024);
		if (fgets(line, 1023, file)==NULL)
			break;
		if (strstr(line, "Power save level: 6 (AC)")) {
			ret = 1;
			break;
		}
	}
	pclose(file);
	return ret;
}


void find_wireless_nic(void) 
{
	FILE *file;

	file = popen("/sbin/iwpriv -a 2> /dev/null", "r");
	if (!file)
		return;
	while (!feof(file)) {
		char line[1024];
		memset(line, 0, 1024);
		if (fgets(line, 1023, file)==NULL)
			break;
		if (strstr(line, "get_power:Power save level")) {
			char *c;
			c = strchr(line, ' ');
			if (c) *c = 0;
			strcpy(wireless_nic, line);
		}
	}
	pclose(file);
}

void activate_wireless_suggestion(void)
{
	char line[1024];
	sprintf(line, "/sbin/iwpriv %s set_power 5", wireless_nic);
	system(line);
}
void suggest_wireless_powersave(void)
{
	char sug[1024];
	if (strlen(wireless_nic)==0)
		find_wireless_nic();
	if (need_wireless_suggest(wireless_nic)) {
		sprintf(sug, _("Suggestion: Enable wireless power saving mode by executing the following command:\n "
			       " iwpriv %s set_power 5 \n"
			       "This will sacrifice network performance slightly to save power."), wireless_nic);
		add_suggestion(sug, 20, 'W', _(" W - Enable wireless power saving "), activate_wireless_suggestion);
	}
}
