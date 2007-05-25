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


#ifndef __INCLUDE_GUARD_POWERTOP_H_
#define __INCLUDE_GUARD_POWERTOP_H_

#include <libintl.h>

struct line {
	char	*string;
	int	count;
};

typedef void (suggestion_func)(void);

extern struct line     *lines;  
extern int             linehead;
extern int             linesize;
extern int             linectotal;

void suggest_process_death(char *process, struct line *lines, int linecount, char *comment, int weight);
void suggest_kernel_config(char *string, int onoff, char *comment, int weight);
void suggest_laptop_mode(void);
void suggest_bluetooth_off(void);
void suggest_nmi_watchdog(void);
void suggest_hpet(void);
void suggest_ac97_powersave(void);
void suggest_wireless_powersave(void);
void suggest_ondemand_governer(void);



extern char cstate_lines[6][200];
extern int topcstate;

extern char status_bar_slots[10][40];
extern char suggestion_key;
extern suggestion_func *suggestion_activate; 


/* min definition borrowed from the Linux kernel */
#define min(x,y) ({ \
        typeof(x) _x = (x);     \
        typeof(y) _y = (y);     \
        (void) (&_x == &_y);            \
        _x < _y ? _x : _y; })


#define _(STRING)    gettext(STRING)


#define PT_COLOR_DEFAULT    1
#define PT_COLOR_HEADER_BAR 2
#define PT_COLOR_ERROR      3
#define PT_COLOR_RED        4
#define PT_COLOR_YELLOW     5
#define PT_COLOR_GREEN      6
#define PT_COLOR_BRIGHT     7
extern int maxwidth;

void show_title_bar(void);
void setup_windows(void);
void initialize_curses(void);
void show_acpi_power_line(double rate, double cap);
void show_cstates(void);
void show_wakeups(double d);
void show_timerstats(int nostats, int ticktime);
void show_suggestion(char *sug);

void pick_suggestion(void);
void add_suggestion(char *text, int weight, char key, char *keystring, suggestion_func *func);
void reset_suggestions(void);


#endif
