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
#include <linux/types.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <sys/ioctl.h>

/* work around a bug in debian -- it exposes kernel internal types to userspace */
#define u64 __u64 
#define u32 __u32  
#define u16 __u16  
#define u8 __u8
#include <linux/ethtool.h>
#undef u64
#undef u32
#undef u16
#undef u8



#include "powertop.h"


static char wireless_nic[32];
static char rfkill_path[PATH_MAX];

static int check_unused_wiresless_up(void)
{
	FILE *file;
	char val;
	char line[1024];
	if (strlen(rfkill_path)<2)
		return 0;
	if (access(rfkill_path, W_OK))
		return 0;

	file = fopen(rfkill_path, "r");
	if (!file)
		return 0;
	val = fgetc(file);
	fclose(file);
	if (val == '1') /* already rfkill'd */
		return 0;
	
	sprintf(line,"iwconfig %s", wireless_nic);
	file = popen(line, "r");
	if (!file)
		return 0;
	while (!feof(file)) {
		memset(line, 0, 1024);
		if (fgets(line, 1023, file) == 0)
			break;
		if (strstr(line, "Mode:Managed") && strstr(line,"Access Point: Not-Associated")) {
			pclose(file);
			return 1;
		}		
	}
	pclose(file);	
	return 0;
}


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
	int sock;
	struct ifreq ifr;
	struct ethtool_value ethtool;
	struct ethtool_drvinfo driver;
	int ifaceup = 0;
	int ret;

	wireless_nic[0] = 0;
	rfkill_path[0] = 0;

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

	
	if (strlen(wireless_nic)==0)
		return;


	memset(&ifr, 0, sizeof(struct ifreq));
	memset(&ethtool, 0, sizeof(struct ethtool_value));

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock<0) 
		return;

	strcpy(ifr.ifr_name, wireless_nic);

	/* Check if the interface is up */
	ret = ioctl(sock, SIOCGIFFLAGS, &ifr);
	if (ret<0)
		return;

	ifaceup = 0;
	if (ifr.ifr_flags & (IFF_UP | IFF_RUNNING))
		ifaceup = 1;

	memset(&driver, 0, sizeof(driver));
	driver.cmd = ETHTOOL_GDRVINFO;
        ifr.ifr_data = (void*) &driver;
        ret = ioctl(sock, SIOCETHTOOL, &ifr);

	sprintf(rfkill_path,"/sys/bus/pci/devices/%s/rf_kill", driver.bus_info);
	close(sock);
}

void activate_wireless_suggestion(void)
{
	char line[1024];
	sprintf(line, "/sbin/iwpriv %s set_power 5 2> /dev/null", wireless_nic);
	system(line);
}

void activate_rfkill_suggestion(void)
{	
	FILE *file;
	file = fopen(rfkill_path, "w");
	if (!file)
		return;
	fprintf(file,"1\n");
	fclose(file);
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
	if (check_unused_wiresless_up()) {
		sprintf(sug, _("Suggestion: Disable the unused WIFI radio by executing the following command:\n "
			       " echo 1 > %s \n"), rfkill_path);
		add_suggestion(sug, 60, 'I', _(" I - disable WIFI Radio "), activate_rfkill_suggestion);

	}
}
