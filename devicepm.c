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
#include <assert.h>

#include "powertop.h"

void activate_runtime_suspend(void)
{
	DIR *dir;
	struct dirent *dirent;
	FILE *file;
	char filename[PATH_MAX];

	dir = opendir("/sys/bus/pci/devices");
	if (!dir)
		return;

	while ((dirent = readdir(dir))) {
		if (dirent->d_name[0]=='.')
			continue;

		sprintf(filename, "/sys/bus/pci/devices/%s/power/control", dirent->d_name);
		file = fopen(filename, "w");
		if (!file)
			continue;
		fprintf(file, "auto\n");
		fclose(file);
	}

	closedir(dir);
}

void suggest_runtime_suspend(void)
{
	DIR *dir;
	struct dirent *dirent;
	FILE *file;
	char filename[PATH_MAX];
	char line[1024];
	int need_hint = 0;

	dir = opendir("/sys/bus/pci/devices");
	if (!dir)
		return;

	while ((dirent = readdir(dir))) {
		if (dirent->d_name[0]=='.')
			continue;

		sprintf(filename, "/sys/bus/pci/devices/%s/power/control", dirent->d_name);
		file = fopen(filename, "r");
		if (!file)
			continue;
		memset(line, 0, 1024);
		if (fgets(line, 1023,file)==NULL) {
			fclose(file);
			continue;
		}
		if (strstr(line, "on"))
			need_hint = 1;

		fclose(file);


	}

	closedir(dir);

	if (need_hint) {
		add_suggestion(_("Suggestion: Enable Device Power Management by pressing the P key\n"
				 ),
				45, 'P', _(" P - Enable Runtime PM"), activate_runtime_suspend);
	}
}



struct device_data;

struct device_data {
	struct device_data *next;
	char pathname[4096];
	char human_name[4096];
	uint64_t active, suspended;
	uint64_t previous_active, previous_suspended;
};


static struct device_data *devices;

static void cachunk_devs(void)
{
	struct device_data *ptr;
	ptr = devices;
	while (ptr) {
		ptr->previous_active = ptr->active;
		ptr->previous_suspended = ptr->suspended;
		ptr = ptr->next;
	}
}

static void update_devstats(char *path, char *shortname)
{
	struct device_data *ptr;
	FILE *file;
	char fullpath[4096], name[4096];
	ptr = devices;
	char *c;

	while (ptr) {
		if (strcmp(ptr->pathname, path)==0) {
			sprintf(fullpath, "%s/power/runtime_active_time", path);
			file = fopen(fullpath, "r");
			if (!file)
				return;
			fgets(name, 4096, file);
			ptr->active = strtoull(name, NULL, 10);
			fclose(file);
			sprintf(fullpath, "%s/power/runtime_suspended_time", path);
			file = fopen(fullpath, "r");
			if (!file)
				return;
			fgets(name, 4096, file);
			ptr->suspended = strtoull(name, NULL, 10);
			fclose(file);

			return;
		}
		ptr = ptr->next;
	}
	/* no luck, new one */
	ptr = malloc(sizeof(struct device_data));
	assert(ptr!=0);
	memset(ptr, 0, sizeof(struct device_data));
	ptr->next = devices;
	devices = ptr;
	strcpy(ptr->pathname, path);

	strcpy(ptr->human_name, shortname);
	
	sprintf(fullpath, "/sbin/lspci -s %s", shortname);
	file = popen(fullpath, "r");
	if (!file)
		return;
	fgets(ptr->human_name, 4095, file);
	pclose(file);
	c = strchr(ptr->human_name, '\n');
	if (c) *c = 0;
	c = strstr(ptr->human_name, "(rev");
	if (c) *c = 0;
}

void count_device_pm(void)
{
	DIR *dir;
	struct dirent *dirent;
	char pathname[PATH_MAX];

	dir = opendir("/sys/bus/pci/devices");
	if (!dir)
		return;
		
	cachunk_devs();
	while ((dirent = readdir(dir))) {
		if (dirent->d_name[0]=='.')
			continue;
		sprintf(pathname, "/sys/bus/pci/devices/%s", dirent->d_name);

		update_devstats(pathname, dirent->d_name);
	}

	closedir(dir);
}


void display_runtime_activity(void)
{
	struct device_data *dev;
	printf("\n");
	printf("%s\n", _("Recent runtime PM statistics"));
	printf("%s\n", _("Active  Device name"));
	dev = devices;
	while (dev) {
		if (dev->active + dev->suspended > 0) 
			printf("%5.1f%%\t%s\n", 100.0*(dev->active - dev->previous_active) / 
				(0.00001 + dev->active - dev->previous_active + dev->suspended - dev->previous_suspended), dev->human_name);
		dev = dev->next;
	}

	dev = devices;
	printf("\n");
	printf("%s\n", _("Devices without runtime PM"));
	dev = devices;
	while (dev) {
		if (dev->active + dev->suspended == 0) 
			printf("%s\n", dev->human_name);
		dev = dev->next;
	}

}

void devicepm_activity_hint(void)
{
	int total_active = 0;
	int pick;
	struct device_data *dev;
	dev = devices;
	while (dev) {
		if (dev->active-1 > dev->previous_active)
			total_active++;
		dev = dev->next;
	}
	if (!total_active)
		return;

	pick = rand() % total_active;
	total_active = 0;
	dev = devices;
	while (dev) {
		if (dev->active-1 > dev->previous_active) {
			if (total_active == pick) {
				char devicepm_hint[8000];

				sprintf(devicepm_hint, _("A device is active %4.1f%% of the time:\n%s"),
				 	100.0*(dev->active - dev->previous_active) / 
    				    	(0.00001 + dev->active - dev->previous_active + dev->suspended - dev->previous_suspended),
					dev->human_name);
				add_suggestion(devicepm_hint,
				1, 'P', _(" P - Enable device power management "), activate_runtime_suspend);
			}
			total_active++;
		}
		dev = dev->next;
	}

}
