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

struct line {
	char	*string;
	int	count;
};

void suggest_process_death(char *process, struct line *lines, int linecount, char *comment);
void suggest_kernel_config(char *string, int onoff, char *comment);
void suggest_laptop_mode(void);
void suggest_bluetooth_off(void);

extern int suggestioncount;

/* min definition borrowed from the Linux kernel */
#define min(x,y) ({ \
        typeof(x) _x = (x);     \
        typeof(y) _y = (y);     \
        (void) (&_x == &_y);            \
        _x < _y ? _x : _y; })


#define _(STRING)    gettext(STRING)

#endif
