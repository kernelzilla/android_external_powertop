/*
 * Copyright (C) 2006, Intel Corporation
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

/* static arrays are not nice programming.. but they're easy */
static char configlines[5000][100];
static int configcount;

extern int suggestioncount;

static void read_kernel_config(void)
{
      FILE *file;
      char version[100], *c;
      char filename[100];
      if (configcount)
        return;
      if (access("/proc/config.gz",R_OK)==0) {
        file = popen("zcat /proc/config.gz","r");
        while (file && !feof(file)) {
          char line[100];
          fgets(line, 100, file);
          strcpy(configlines[configcount++], line);
        }
        pclose(file);
        return;
      } 
      file = fopen("/proc/sys/kernel/osrelease","r");
      if (!file)
        return;
      fgets(version, 100, file);
      fclose(file);
      c = strchr(version, '\n');
      if (c) *c=0;
      sprintf(filename,"/boot/config-%s",version);
      file = fopen(filename,"r");
	if (!file) {
		sprintf(filename, "/lib/modules/%s/build/.config", version);
		file = fopen(filename, "r");
	}
      if (!file) 
          return;
        while (!feof(file)) {
          char line[100];
          fgets(line, 100, file);
          strcpy(configlines[configcount++], line);
        }
        fclose(file);
}

/*
 * Suggest the user to turn on/off a kernel config option.
 * "comment" gets displayed if it's not already set to the right value 
 */
void suggest_kernel_config(char *string, int onoff, char *comment)
{
  int i;
  char searchfor[100];
  
  if (suggestioncount>0)
    return;
  read_kernel_config();
  if (onoff)
     sprintf(searchfor, "%s=", string);
  else
     sprintf(searchfor, "# %s is not set", string);
     
  for (i=0; i < configcount; i++) {
      if (strstr(configlines[i],searchfor))
          return;
  }
  printf("%s\n", comment);
  fflush(stdout);
  suggestioncount++;
}
