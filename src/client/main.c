/*
 * bluez_list_devices.c - Find the Bluetooth devices
 * 	- The example uses GDBUS to get the list of bluetooth devices using DBUS
 * 	  interfaces provided by bluez. This lists only the devices which are already or
 *	  currently enumerated by the Adapter.
 * 	- The device may be appeared in followind conditions,
 *		- Previously paired with Adapter
 *		- Previously paired and connected with Adapter
 *		- Appeared after StartDiscovery session (but not yet used/removed)
 * gcc `pkg-config --cflags glib-2.0 gio-2.0` -Wall -Wextra -o ./bin/bluez_list_devices ./bluez_list_devices.c `pkg-config --libs glib-2.0 gio-2.0`
 */

#include <glib.h>
#include <gio/gio.h>
#include <glib-unix.h>
#include "a2dp.h"

int main(void)
{
	
	return 0;
}