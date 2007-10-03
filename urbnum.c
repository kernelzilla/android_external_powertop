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

struct device_data;

struct device_data {
	struct device_data *next;
	char pathname[4096];
	char human_name[4096];
	uint64_t urbs;
	uint64_t previous_urbs;
};


static struct device_data *devices;

static void cachunk_urbs(void)
{
	struct device_data *ptr;
	ptr = devices;
	while (ptr) {
		ptr->previous_urbs = ptr->urbs;
		ptr = ptr->next;
	}
}

static void update_urbnum(char *path, uint64_t count, char *shortname)
{
	struct device_data *ptr;
	FILE *file;
	char fullpath[4096];
	char name[4096], vendor[4096];
	ptr = devices;
	while (ptr) {
		if (strcmp(ptr->pathname, path)==0) {
			ptr->urbs = count;
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
	ptr->urbs = ptr->previous_urbs = count;
	sprintf(fullpath, "%s/product", path);
	file = fopen(fullpath, "r");
	memset(name, 0, 4096);
	if (file) { 
		fgets(name, 4096, file);
		fclose(file);
	}
	sprintf(fullpath, "%s/manufacturer", path);
	file = fopen(fullpath, "r");
	memset(vendor, 0, 4096);
	if (file) { 
		fgets(vendor, 4096, file);
		fclose(file);
	}
	
	if (strlen(name)>0 && name[strlen(name)-1]=='\n')
		name[strlen(name)-1]=0;
	if (strlen(vendor)>0 && vendor[strlen(vendor)-1]=='\n')
		vendor[strlen(vendor)-1]=0;
	/* some devices have bogus names */
	if (strlen(name)<4)
		strcpy(ptr->human_name, path);
	else
		sprintf(ptr->human_name, _(" USB device %s : %s (%s)"), shortname, name, vendor);
	
}

void count_usb_urbs(void)
{
	DIR *dir;
	struct dirent *dirent;
	FILE *file;
	char filename[PATH_MAX];
	char pathname[PATH_MAX];
	char buffer[4096];
	struct device_data *dev;

	dir = opendir("/sys/bus/usb/devices");
	if (!dir)
		return;
		
	cachunk_urbs();
	while ((dirent = readdir(dir))) {
		if (dirent->d_name[0]=='.')
			continue;
		sprintf(pathname, "/sys/bus/usb/devices/%s", dirent->d_name);
		sprintf(filename, "%s/urbnum", pathname);
		file = fopen(filename, "r");
		if (!file)
			continue;
		memset(buffer, 0, 4096);
		fgets(buffer, 4095, file);
		update_urbnum(pathname, strtoull(buffer, NULL, 10), dirent->d_name);
		fclose(file);
	}

	closedir(dir);
	
	dev = devices;
	while (dev) {
		if (dev->urbs != dev->previous_urbs) {
			push_line(dev->human_name, dev->urbs - dev->previous_urbs);
		}
		dev = dev->next;
	}
}

