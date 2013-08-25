/*
Copyright (C) 2012 Matt Boyer.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. Neither the name of the project nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
*/

#include <stdlib.h>
#include <unistd.h>
#include "udtools.h"
#include "ud2_dbus.h"
#include "udtools_config.h"

void usage() {
	g_print("Usage:\n"\
		"  udenum -v : Prints version\n\n"\
		"  udenum -d : Enumerates UDisks drives\n"\
		"  Options:\n"\
		"    -e : only shows ejectable drives\n"\
		"    +e : only shows non-ejectable drives\n\n"\
		"  udenum -f : Enumerates UDisks filesystems\n"\
		"  Options:\n"\
		"    -m : only shows mounted filesystems\n"\
		"    +m : only shows unmounted filesystems\n\n"\
		"  udenum -p : Enumerates UDisks partitions\n"\
		"  Options:\n"\
		"    -k : only shows partitions with filesystems\n"\
		"    +k : only shows partitions without filesystems\n\n"\
		/*"  udenum -t : Enumerates UDisks partition tables\n"\*/
		"  Output options:\n"\
		"    -o : displays DBus object paths\n"\
		"    -i : displays block devices by id\n"\
		"    -u : displays block devices by uuid\n"\
		"    -l : displays block devices by label\n");
}

void print_version() {
	g_print("udenum v%d.%02d.%d\n",
		UDTOOLS_VERSION_MAJOR,
		UDTOOLS_VERSION_MINOR,
		UDTOOLS_VERSION_CHANGESET);
	gchar* udisks_version = get_version();
	g_print("UDisks v%s\n", udisks_version);
	g_free(udisks_version);
}

void enumerate_drives(enum UD_FORMAT mode, enum OPTION_FILTER ejectable_filter) {

	g_assert(mode==DBUS_OBJECT_PATH || mode==DRIVE_NAME);
	struct UD2_enumerations* enums = enum_objects();
	GList* drives = enums->drives;
	if (!drives) {
		g_printerr("No drives found in UDisks enumeration\n");
	}

	for(drives=g_list_first(drives); drives; drives=g_list_next(drives)) {
		switch(ejectable_filter) {
			case REQUIRED:
				if (!GPOINTER_TO_INT(g_hash_table_lookup(drives->data, "ejectable")))
					continue;
				break;
			case DISALLOWED:
				if (GPOINTER_TO_INT(g_hash_table_lookup(drives->data, "ejectable")))
					continue;
				break;
			case INDIFFERENT:
				break;
		}

		gchar* dbus_drive_path = g_hash_table_lookup(drives->data, "object_path");
		glong prefix_length = g_utf8_strlen(UD2_DRIVE_PREFIX, -1);

		g_assert(g_str_has_prefix(dbus_drive_path, UD2_DRIVE_PREFIX));
		if (mode==DBUS_OBJECT_PATH) {
			g_print("%s\n", dbus_drive_path);
		} else if (mode==DRIVE_NAME) {
			/* Remove the prefix from the DBus object path */
			g_print("%s\n", dbus_drive_path+prefix_length);
		}
	}
	g_list_free(drives);
	g_free(enums);
}

void enumerate_block_devices(enum BLOCK_TYPE block_type, enum UD_FORMAT mode, enum OPTION_FILTER filter) {

	/* Use the PreferredDevice property which should be a link under /dev */
	struct UD2_enumerations* enums = enum_objects();

	GList* block_device = enums->block_devices;
	if (!block_device) {
		g_printerr("No block devices found in UDisks enumeration\n");
		goto end_enum;
	}

	gchar* filter_property = NULL;
	switch(block_type) {
		case PARTITION:
			filter_property = g_strdup("has_filesystem");
			break;
		case FILESYSTEM:
			filter_property = g_strdup("mounted");
			break;
	}

	for(block_device=g_list_first(block_device); block_device; block_device=g_list_next(block_device)) {
		if (1 == GPOINTER_TO_INT(g_hash_table_lookup(block_device->data, "has_partition_table")))
			continue;

		switch(filter) {
			case REQUIRED:
				if (!GPOINTER_TO_INT(g_hash_table_lookup(block_device->data, filter_property)))
					continue;
				break;
			case DISALLOWED:
				if (GPOINTER_TO_INT(g_hash_table_lookup(block_device->data, filter_property)))
					continue;
				break;
			case INDIFFERENT:
				break;
		}

		/* Display the block device in the mode requested */
		if (mode==DBUS_OBJECT_PATH) {
			g_print("%s\n", g_hash_table_lookup(block_device->data, "object_path"));
		} else if (mode==DEV_PREFERRED) {
			g_print("%s\n", g_hash_table_lookup(block_device->data, "dev_path"));
		} else {
			/* Iterate on all symlinks */
			GList* symlink_list = g_hash_table_lookup(block_device->data, "symlinks");
			gboolean found = FALSE;
			for(symlink_list = g_list_first(symlink_list); symlink_list; symlink_list = g_list_next(symlink_list)) {
				switch(mode) {
					case DEV_ID:
						if (!g_str_has_prefix(symlink_list->data, "/dev/disk/by-id"))
							continue;
						found = TRUE;
						break;
					case DEV_UUID:
						if (!g_str_has_prefix(symlink_list->data, "/dev/disk/by-uuid"))
							continue;
						found = TRUE;
						break;
					case DEV_LABEL:
						if (!g_str_has_prefix(symlink_list->data, "/dev/disk/by-label"))
							continue;
						found = TRUE;
						break;
				}
				if (found) {
					g_print("%s\n", symlink_list->data);
					break;
				}
			}
			if (!found)
				/* Couldn't find a symlink of the preferred
				 * type, fall back to the preferred path
				 */
				g_print("%s\n", g_hash_table_lookup(block_device->data, "dev_path"));
		}
	}
	g_free(filter_property);
	g_list_free(block_device);

end_enum:
	g_free(enums);
}

int main(int argc, char** argv) {
	int drive_flag=0;
	int partition_flag=0;
	int table_flag=0;
	int filesystem_flag=0;
	int version_flag=0;

	enum OPTION_FILTER mounted_filter = INDIFFERENT;
	enum OPTION_FILTER filesystem_filter = INDIFFERENT;
	enum OPTION_FILTER ejectable_filter = INDIFFERENT;

	enum UD_FORMAT output_mode=DEFAULT;

	int option=0;
	opterr = 0;

	/* TODO use http://developer.gnome.org/glib/2.32/glib-Commandline-option-parser.html instead */
	while ((option = getopt(argc, argv, "uilodpfvmke")) != -1) {
		switch(option) {
			case 'd':
				drive_flag+=1;
				if (DEFAULT==output_mode)
					output_mode=DRIVE_NAME;
				break;
			case 'p':
				partition_flag+=1;
				if (DEFAULT==output_mode)
					output_mode=DEV_PREFERRED;
				break;
			case 't':
				table_flag+=1;
				if (DEFAULT==output_mode)
					output_mode=DEV_ID;
				break;
			case 'f':
				filesystem_flag+=1;
				if (DEFAULT==output_mode)
					output_mode=DEV_ID;
				break;
			case 'v':
				version_flag+=1;
				break;

			/* Output modes */
			case 'o':
				output_mode=DBUS_OBJECT_PATH;
				break;
			case 'u':
				output_mode=DEV_UUID;
				break;
			case 'i':
				output_mode=DEV_ID;
				break;
			case 'l':
				output_mode=DEV_LABEL;
				break;

			/* 'ejectable' filter */
			case 'e':
				if (1 != drive_flag) {
					g_printerr("-/+e only allowed with -d\n");
					return EXIT_FAILURE;
				}
				ejectable_filter = REQUIRED;
				break;

			/* 'mounted' filter */
			case 'm':
				if (1 != filesystem_flag) {
					g_printerr("-/+m only allowed with -f\n");
					return EXIT_FAILURE;
				}
				mounted_filter = REQUIRED;
				break;

			/* 'filesystem' filter */
			case 'k':
				if (1 != partition_flag) {
					g_printerr("-/+k only allowed with -p\n");
					return EXIT_FAILURE;
				}
				filesystem_filter = REQUIRED;
				break;
			case '?':
				usage();
				return EXIT_FAILURE;
				break;
		}
	}

	while (argc > optind) {
		if ('+' != argv[optind][0])
			g_printerr("Invalid option: %s\n", argv[optind]);

		if (0 == g_strcmp0("+m", argv[optind])) {
			if (1 != filesystem_flag) {
				g_printerr("-/+m only allowed with -f\n");
				return EXIT_FAILURE;
			}
			mounted_filter = DISALLOWED;
		} else if (0 == g_strcmp0("+k", argv[optind])) {
			if (1 != partition_flag) {
				g_printerr("-/+k only allowed with -p\n");
				return EXIT_FAILURE;
			}
			filesystem_filter = DISALLOWED;
		} else if (0 == g_strcmp0("+e", argv[optind])) {
			if (1 != drive_flag) {
				g_printerr("-/+e only allowed with -d\n");
				return EXIT_FAILURE;
			}
			ejectable_filter = DISALLOWED;
		} else {
			usage();
			return EXIT_FAILURE;
		}
		optind++;
	}

	/* Options d,p,v,t are mutually exclusive. Exactly one MUST be specified */
	if ( 1 != (drive_flag+table_flag+partition_flag+filesystem_flag+version_flag) ) {
		usage();
		return EXIT_FAILURE;
	}

	if (version_flag) print_version();
	if (drive_flag) enumerate_drives(output_mode, ejectable_filter);
	if (partition_flag) enumerate_block_devices(PARTITION, output_mode, filesystem_filter);
	if (filesystem_flag) enumerate_block_devices(FILESYSTEM, output_mode, mounted_filter);

	return EXIT_SUCCESS;
}

/* vim:set tabstop=8 softtabstop=8 shiftwidth=8 noexpandtab list: */
