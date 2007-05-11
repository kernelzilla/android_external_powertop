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

uint64_t start_usage[8], start_duration[8];
uint64_t last_usage[8], last_duration[8];

void suggest_kernel_config(char *string, int onoff, char *comment);


int ticktime = 5;

int suggestioncount = 0;

#define IRQCOUNT 100

struct irqdata {
	int active;
	int number;
	uint64_t count;
	char description[256];
};

struct irqdata interrupts[IRQCOUNT];

#define FREQ 3579.545

int nostats;

#define LINECOUNT 250+IRQCOUNT
char lines[LINECOUNT][256];
int linec[LINECOUNT];
int linehead;
int linectotal;

void push_line(char *string, int count)
{
	int i;
	for (i=0; i<linehead; i++)
		if (strcmp(string, lines[i])==0) {
			linec[i]+= count;
			return;
		}
	strcpy(lines[linehead], string);
	linec[linehead] = count;
	linehead++;
}

void clear_lines(void)
{
	memset(lines,0, sizeof(lines));
	memset(linec,0, sizeof(linec));
	linehead = 0;
}

void count_lines(void)
{
	uint64_t q = 0;
	int i;
	for (i=0; i<linehead; i++)
		q+=linec[i];
	linectotal = q;	
}

int update_irq(int irq, uint64_t count, char *name)
{
	int i;
	int firstfree=IRQCOUNT;
	for (i=0; i<IRQCOUNT; i++) {
		if (interrupts[i].active && interrupts[i].number == irq) {
			uint64_t oldcount;
			oldcount = interrupts[i].count;
			interrupts[i].count = count;
			return count - oldcount;
		}
		if (!interrupts[i].active && firstfree > i)
			firstfree = i;
	}
	
	interrupts[firstfree].active = 1;
	interrupts[firstfree].count = count;
	interrupts[firstfree].number = irq;
	strcpy(interrupts[firstfree].description, name);
	if (strlen(name)>40)
		interrupts[firstfree].description[40]=0;
	return count;
}

static void do_proc_irq(void)
{
	FILE *file;
	char line[1024];
	char *name;
	uint64_t delta;
	file = fopen("/proc/interrupts", "r");
	if (!file)
		return;
	while (!feof(file)) {
		char *c;
		int nr = -1;
		uint64_t count = 0;
		memset(line,0,sizeof(line));
		fgets(line, 1024, file);
		c = strchr(line, ':');
		if (!c)
			continue;
		/* deal with NMI and the like */
		nr = strtoull(line, NULL, 10);
		if (line[0]!=' ' && (line[0]<'0' || line[0]>'9'))
			continue;
		*c=0;
		c++;
		while (c && strlen(c)) {
			char *newc;
			count += strtoull(c, &newc, 10);
			if (newc==c)
				break;
			c = newc;
		}
		c = strchr(c, ' ');
		while (c && *c==' ') c++;
		c = strchr(c, ' ');
		while (c && *c==' ') c++;
		name = c;
		delta = update_irq(nr, count, name);
		c = strchr(name, '\n');
		if (strlen(name)>50)
			name[50]=0;
		if (c) *c=0;
		sprintf(line, "    <interrupt> : %s",name);
		if (nr>0 && delta>0)
			push_line(line, delta);
	}
	fclose(file);
}

static void read_data(uint64_t *usage, uint64_t *duration)
{
	DIR 	*dir;
	struct dirent *entry;
	FILE	*file = NULL;
	char	line[4096];
	char 	*c; 
	int 	clevel = 0;

	memset(usage, 0, 64);
	memset(duration, 0, 64);

	dir = opendir("/proc/acpi/processor");
	if (!dir)
		return;
	while ((entry=readdir(dir))) {
		if (strlen(entry->d_name)<3)
			continue;
		sprintf(line,"/proc/acpi/processor/%s/power", entry->d_name);
		file = fopen(line, "r");
		if (!file)
			continue;

		clevel = 0;

		while (!feof(file)) {
			memset(line, 0, 4096);
			fgets(line, 4096, file);
			c = strstr(line, "age[");
			if (!c) 
				continue;
			c+=4;
			usage[clevel] += strtoull(c, NULL, 10);
			c = strstr(line, "ation[");
			if (!c) 
				continue;
			c+=6;
			duration[clevel] += strtoull(c, NULL, 10);
	
			clevel++;
		
		}
		fclose(file);
	}
	closedir(dir);
}	

void stop_timerstats(void)
{
	FILE *file;
	file = fopen("/proc/timer_stats","w");
	if (!file) {
		nostats = 1;
		return;
	}
	fprintf(file,"0\n");
	fclose(file);
}
void start_timerstats(void)
{
	FILE *file;
	file = fopen("/proc/timer_stats","w");
	if (!file) {
		nostats = 1;
		return;
	}		
	fprintf(file,"1\n");
	fclose(file);
}

void green(void)
{
	printf("\33[42m\33[1m");
}
void yellow(void)
{
	printf("\33[43m\33[1m");
}
void red(void)
{
	printf("\33[41m\33[1m");
}
void normal(void)
{
	printf("\33[0m");
	fflush(stdout);
}

void sort_lines(void)
{
	int swaps = 1;
	while (swaps) {
		int i;
		uint64_t c;
		char line[1024];
		swaps = 0;
		for (i=0; i<LINECOUNT-1; i++)
			if (linec[i] < linec[i+1]) {
				swaps++;
				strcpy(line,lines[i]);
				c=linec[i];
				linec[i]=linec[i+1];
				linec[i+1]=c;
				strcpy(lines[i], lines[i+1]);
				strcpy(lines[i+1], line);
			}
	}
}

void print_battery(void)
{
	DIR *dir;
	struct dirent *dirent;
	FILE *file;
	unsigned long rate = 0;
	unsigned long cap = 0;

	char filename[256];

	dir = opendir("/proc/acpi/battery");
	if (!dir)
		return;
	
	while ( (dirent = readdir(dir))) {
		sprintf(filename,"/proc/acpi/battery/%s/state", dirent->d_name);
		file = fopen(filename, "r");
		if (!file)
			continue;
		while (!feof(file)) {
			char line[1024];
			char *c;
			int dontcount = 0;
			fgets(line, 1024, file);
			if (strstr(line, "present:") && strstr(line,"no"))
				break;

			if (strstr(line, "charging state:") && !strstr(line,"discharging"))
				dontcount=1;
			if (!strstr(line,"present rate") && !strstr(line, "remaining capacity"))
				continue;
			c = strchr(line, ':');
			if (!c)
				continue;
			c++;
			if (strstr(line,"capaci"))
				cap += strtoull(c, NULL, 10);
			else if (!dontcount)
				rate += strtoull(c, NULL, 10);
			
		}
		fclose(file);
	}
	closedir(dir);
	if (rate>0)
		printf("Power usage (ACPI estimate) : %5.1f W (%3.1f hours left)\n", rate/1000.0, cap*1.0/rate);
}


int main(int argc, char **argv)
{
	char line[1024];
	FILE *file = NULL;
	uint64_t cur_usage[8], cur_duration[8];
	read_data(&start_usage[0], &start_duration[0]);

	memcpy(last_usage, start_usage, sizeof(last_usage));
	memcpy(last_duration, start_duration, sizeof(last_duration));
	
	do_proc_irq();

	memset(cur_usage, 0, sizeof(cur_usage));
	memset(cur_duration, 0, sizeof(cur_duration));
	printf("PowerTOP 1.0    (C) 2007 Intel Corporation \n\n");
	if (getuid()!=0)
		printf("PowerTOP needs to be run as root to collect enough information\n");
	printf("Collecting data for %i seconds \n", ticktime);
	stop_timerstats();
	while (1) {
		double maxsleep = 0.0;
		int64_t totalticks;
		int64_t totalevents;
		int i = 0;
		double c0 = 0;
		char *c;
		do_proc_irq();
		start_timerstats();
		sleep(ticktime);
		stop_timerstats();
		read_data(&cur_usage[0], &cur_duration[0]);
		
		totalticks = 0;
		totalevents = 0;
		for (i=0; i<8; i++) 
			if (cur_usage[i]) {
				totalticks += cur_duration[i] - last_duration[i];
				totalevents +=  cur_usage[i] - last_usage[i];
			}
			
		printf("\33[H\33[J\33[47m\33[30m    PowerTOP version 1.0       (C) 2007 Intel Corporation                       \n");
		normal();
		printf("\n");
		if (totalevents == 0) {
			printf("< Detailed C-state information is only available on Mobile CPUs (laptops) >\n");
		} else {
			c0 = sysconf(_SC_NPROCESSORS_ONLN)*ticktime * 1000 * FREQ - totalticks;
			if (c0<0) c0 = 0; /* rounding errors in measurement might make c0 go slightly negative.. this is confusing */
			printf("Cn\t    Avg residency (%is)\tLong term residency avg\n", ticktime);
			printf("C0 (cpu running)        (%4.1f%%)\n", c0*100.0/(sysconf(_SC_NPROCESSORS_ONLN)*ticktime * 1000*FREQ) );
			for (i=0; i<8; i++) 
				if (cur_usage[i])  {
					double sleep;
					sleep = (cur_duration[i] - last_duration[i]) / (cur_usage[i] - last_usage[i]+0.1) / FREQ;
					printf("C%i\t\t%5.1fms (%4.1f%%)\t\t\t%5.1fms\n", i+1, sleep,
								(cur_duration[i] - last_duration[i])*100/(sysconf(_SC_NPROCESSORS_ONLN)*ticktime * 1000*FREQ),
								(cur_duration[i] - start_duration[i]) / (cur_usage[i] - start_usage[i]+0.1) / FREQ);
					if (maxsleep < sleep)
						maxsleep = sleep;
				}
		}			
		/* now the timer_stats info */
		memset(line,0,sizeof(line));
		clear_lines();
		do_proc_irq();
		i = 0;
		totalticks = 0;
		if (!nostats)
			file = popen("cat /proc/timer_stats | sort -n | tail -190", "r");
		while (file && !feof(file) && i<190) {
			char *count, *pid, *process, *func;
			int cnt;
			fgets(line, 1024, file);
			if (strstr(line, "total events"))
				break;
			c = count =  & line[0];
			c=strchr(c,',');
			if (!c)
				continue;
			*c = 0;
			c++;
			while (*c!=0 && *c==' ') c++;
			pid = c;
			c = strchr(c,' ');
			if (!c)
				continue;
			*c=0; c++;
			while (*c!=0 && *c==' ') c++;
			process = c;
			c = strchr(c, ' ');
			if (!c)
				continue;
			*c = 0;c++;
			while (*c!=0 && *c==' ') c++;
			func = c;
			if (strcmp(process,"insmod")==0)
				process = "<kernel module>";
			if (strcmp(process,"swapper")==0)
				process = "<kernel core>";
			c = strchr(c, '\n');
			if (strncmp(func,"tick_nohz_",10)==0)
				continue;
			if (strcmp(process,"powertop")==0)
				continue;
			if (c) *c=0;
			cnt = strtoull(count, NULL, 10);
			sprintf(line,"%15s : %s", process, func);
			push_line(line, cnt);
			i++;
		}
		if (file )
			pclose(file);
		printf("\n");
		if (!nostats && totalevents) {
			int d;
			d = totalevents * 1.0 / ticktime / sysconf(_SC_NPROCESSORS_ONLN);
			printf("Wakeups-from-idle per second : ");
			if (d<3)
				green();
			else if (d<10)
				yellow();
			else
				red();
			printf(" %4.1f ", totalevents *1.0 / ticktime);
			normal();
			printf("\n");
		}
		print_battery();
		count_lines();
		if (!nostats) {
			int counter =0 ;
			sort_lines();
			printf("\nTop causes for wakeups:\n");
			for (i=0;i<LINECOUNT;i++)
				if (linec[i]>0 && counter++<10)
					printf(" %5.1f%%    %s \n", linec[i]*100.0/linectotal,
							lines[i]);
			fflush(stdout);
		} else {
			printf("No detailed statistics available; please enable the CONFIG_TIMER_STATS kernel option\n");
		}
		if (maxsleep < 5.0)
			ticktime = 5;
		else if (maxsleep < 30.0)
			ticktime = 10;
		else if (maxsleep < 100.0)
			ticktime = 20;
		else if (maxsleep < 400.0)
			ticktime = 30;
		else 
			ticktime = 45;

		printf("\n");			
		suggestioncount = 0;
		suggest_kernel_config("CONFIG_USB_SUSPEND", 1, 
			"Suggestion: Enable the CONFIG_USB_SUSPEND kernel configuration option.\nThis option will automatically disable UHCI USB when not in use, and may\nsave approximately 1 Watt of power.");
		suggest_kernel_config("CONFIG_NO_HZ", 1, 
			"Suggestion: Enable the CONFIG_NO_HZ kernel configuration option.\nThis option is required to get any kind of longer sleep times in the CPU.");
		suggest_kernel_config("CONFIG_HPET", 1, 
			"Suggestion: Enable the CONFIG_HPET kernel configuration option.\n"
			"Without HPET support the kernel needs to wake up every 20 miliseconds for \n"
			"some housekeeping tasks.");
		suggest_kernel_config("CONFIG_CPU_FREQ_GOV_ONDEMAND", 1, 
			"Suggestion: Enable the CONFIG_CPU_FREQ_GOV_ONDEMAND kernel configuration option.\n"
			"The 'ondemand' CPU speed governer will minimize the CPU power usage while\n"
			"giving you performance when it is needed.");
		suggest_kernel_config("CONFIG_IRQBALANCE", 0, 
			"Suggestion: Disable the CONFIG_IRQBALANCE kernel configuration option.\n"
			"The in-kernel irq balancer is obsolete and wakes the CPU up far more than needed.");
		suggest_kernel_config("CONFIG_ACPI_DEBUG", 0, 
			"Suggestion: Disable the CONFIG_ACPI_DEBUG kernel configuration option.\n"
			"ACPI debug can make the bios wake you up a lot more than needed.");

		sleep(3); /* quiet down the effects of any IO to xterms */
	
		read_data(&cur_usage[0], &cur_duration[0]);
		memcpy(last_usage, cur_usage, sizeof(last_usage));
		memcpy(last_duration, cur_duration, sizeof(last_duration));
	}
	return 0;
}
