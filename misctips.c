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

void set_laptop_mode(void)
{
	FILE *file;
	file = fopen("/proc/sys/vm/laptop_mode", "r");
	if (!file)
		return;
	fprintf(file,"5\n");
	fclose(file);
}

void suggest_laptop_mode(void)
{
	FILE *file;
	int i;
	char buffer[1024];
	file = fopen("/proc/sys/vm/laptop_mode", "r");
	if (!file)
		return;
	memset(buffer, 0, 1024);
	if (!fgets(buffer, 1023, file)) {
		fclose(file);
		return;
	}
	i = strtoul(buffer, NULL, 10);
	if (i<1) {
		add_suggestion( _("Suggestion: Enable laptop-mode by executing the following command:\n"
		 	"   echo 5 > /proc/sys/vm/laptop_mode \n"), 15, 'L', _(" L - enable Laptop mode "), set_laptop_mode);
	}
	fclose(file);
}

void nmi_watchdog_off(void)
{
	FILE *file;
	file = fopen("/proc/sys/kernel/nmi_watchdog", "w");
	if (!file)
		return;
	fprintf(file,"0\n");
	fclose(file);
}
void suggest_nmi_watchdog(void)
{
	FILE *file;
	int i;
	char buffer[1024];
	file = fopen("/proc/sys/kernel/nmi_watchdog", "r");
	if (!file)
		return;
	memset(buffer, 0, 1024);
	if (!fgets(buffer, 1023, file)) {
		fclose(file);
		return;
	}
	i = strtoul(buffer, NULL, 10);
	if (i!=0) {
		add_suggestion( _("Suggestion: disable the NMI watchdog by executing the following command:\n"
		 	"   echo 0 > /proc/sys/kernel/nmi_watchdog \n"
			"The NMI watchdog is a kernel debug mechanism to detect deadlocks"), 25, 'N', _(" N - Turn NMI watchdog off "), nmi_watchdog_off);
	}
	fclose(file);
}

void suggest_hpet(void)
{
	FILE *file;
	char buffer[1024];
	file = fopen("/proc/timer_list", "r");
	if (!file)
		return;
	memset(buffer, 0, 1024);
	while (!feof(file)) {
		if (!fgets(buffer, 1023, file)) {
			fclose(file);
			return;
		}
		if (strstr(buffer, "Clock Event Device: hpet")) {
			fclose(file);
			return;
		}
	}

	add_suggestion( _("Suggestion: enable the HPET (Multimedia Timer) in your BIOS or add \n"
		          "the kernel patch to force-enable HPET. HPET support allows Linux to \n"
			  "have much longer sleep intervals."), 7, 0, NULL, NULL);
}
