#include "glib.h"
#include <gio/gio.h>
#include <stdlib.h>
#include <unistd.h>

#include "udpart.h"
#include "ud2_dbus.h"
#include "udtools_config.h"

void usage() {
	g_print("Usage:\n"\
		"  udpart <command> <block_device>\n\n"\
		"With <command> one of:\n"\
		"  list : Prints partition table\n\n"\
	);
}



int main(int argc, char** argv) {

	if (1>=argc) {
		usage();
		return EXIT_FAILURE;
	}

	gboolean valid_command = FALSE;
	gchar** commands = g_strsplit(UDPART_COMMANDS, " ", 0);
	int com_idx;
	for(com_idx=0; com_idx<g_strv_length(commands); ++com_idx) {
		valid_command = (0==g_strcmp0(commands[com_idx], argv[1]));
		if (valid_command)
			break;
	}
	if (!valid_command) {
		usage();
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
