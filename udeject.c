#include "glib.h"
#include <gio/gio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ud2_dbus.h"
#include "udtools.h"
#include "udtools_config.h"

void usage() {
	g_print("Usage:\n"\
		"  udeject [options] <drive>\n\n"\
		"  Options:\n"\
		"    -o : drive is a DBus object path\n"\
	);
}



int main(int argc, char** argv) {

	if (2>argc) {
		usage();
		return EXIT_FAILURE;
	}

	enum UD_FORMAT drive_format = DRIVE_NAME;

	int option=0;
	while ((option = getopt(argc, argv, "o")) != -1) {
		switch(option) {
			case 'o':
				drive_format = DBUS_OBJECT_PATH;
				break;
			case '?':
				usage();
				return EXIT_FAILURE;
				break;
		}
	}

	if (argc!=optind+1) {
		usage();
		return EXIT_FAILURE;
	}

	gchar* dbus_drive_path=NULL;
	if (DBUS_OBJECT_PATH==drive_format) {
		dbus_drive_path = g_strdup(argv[optind]);
	} else {
		/* Find the object path based on name */
		return EXIT_FAILURE;
	}

	gboolean success = eject(dbus_drive_path);
	g_free(dbus_drive_path);
	if (success)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}
