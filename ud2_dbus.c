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

#include "ud2_dbus.h"
#include "glib.h"
#include <gio/gio.h>

gchar* get_version() {
	g_type_init();
	GDBusProxy* UD2_Manager_Proxy = NULL;
	GError* error=NULL;
	
	UD2_Manager_Proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
		G_DBUS_PROXY_FLAGS_NONE,
		NULL,
		UD2_DBUS_NAME,
		UD2_MANAGER_PATH,
		DBUS_PROP_IFACE,
		NULL,
		&error);

	if (!UD2_Manager_Proxy) {
		g_printerr("%s\n", error->message);
		g_error_free(error);
	}

	GVariant* UD2_Version = g_dbus_proxy_call_sync(	UD2_Manager_Proxy,
							(gchar*) "Get",
							g_variant_new(	"(ss)",
									"org.freedesktop.UDisks2.Manager",
									"Version"
							),
							G_DBUS_CALL_FLAGS_NONE,
							-1,
							NULL,
							&error);

	if (!UD2_Version) {
		g_printerr("%s\n", error->message);
		g_error_free(error);
	}

	GVariant* version_string;
	g_variant_get(UD2_Version, "(v)", &version_string);
	/* version is allocated independently from the GVariants */
	gchar* version=g_variant_dup_string(version_string, NULL);

	g_variant_unref(version_string);
	g_variant_unref(UD2_Version);
	g_object_unref(UD2_Manager_Proxy);
	return version;
}

struct UD2_enumerations* enum_objects() {
	g_type_init();

	struct UD2_enumerations* enumerations = g_new(struct UD2_enumerations, 1);
	GList* UD2_drives = NULL;
	GList* UD2_blocks = NULL;

	GDBusProxy* UD2_Proxy = NULL;
	GError* error=NULL;
	
	UD2_Proxy = g_dbus_proxy_new_for_bus_sync(	G_BUS_TYPE_SYSTEM,
							G_DBUS_PROXY_FLAGS_NONE,
							NULL,
							UD2_DBUS_NAME,
							UD2_PATH,
							DBUS_MANAGER_IFACE,
							NULL,
							&error);

	if (!UD2_Proxy) {
		g_printerr("%s\n", error->message);
		g_error_free(error);
	}

	/* Use the GetManagedObjects method from the ObjectManager interface to
	 * enumerate UD2 objects
	 */
	GVariant* UD2_Objects = g_dbus_proxy_call_sync(	UD2_Proxy,
							(gchar*) "GetManagedObjects",
							NULL,
							G_DBUS_CALL_FLAGS_NONE,
							-1,
							NULL,
							&error);

	if (!UD2_Objects) {
		g_printerr("%s\n", error->message);
		g_error_free(error);
	}

	/* 
	As per http://dbus.freedesktop.org/doc/dbus-specification.html
	the return type for method GetManagedObjects is
	DICT<OBJPATH,DICT<STRING,DICT<STRING,VARIANT>>> or (a{oa{sa{sv}}}) in
	DBus parlance.
	*/
	g_assert_cmpstr("(a{oa{sa{sv}}})", ==, g_variant_get_type_string(UD2_Objects));
	/* The tuple contains only one dictionary */
	g_assert_cmpint(1, ==, g_variant_n_children(UD2_Objects));
	/* Re-parent UD2_Objects to the sole element in the tuple */
	UD2_Objects = g_variant_get_child_value(UD2_Objects, 0);


	GVariantIter* dict_iter = g_variant_iter_new(UD2_Objects);
	GVariant* kv_pair;
	while(kv_pair = g_variant_iter_next_value(dict_iter)) {

		gchar* dbus_object_path;
		GVariant* interface_dict;
		g_variant_get(kv_pair, "{o*}", &dbus_object_path, &interface_dict);

		if (0 == g_strcmp0("/org/freedesktop/UDisks2/Manager", dbus_object_path))
			goto next_object;

		/* Store the object's properties in a hash table */
		GHashTable* UD2_object_properties = g_hash_table_new(g_str_hash, g_str_equal);
		g_hash_table_insert(UD2_object_properties, "object_path", g_strdup(dbus_object_path));

		/* Parse the object's DBus interfaces */
		GVariantIter* interface_iterator = g_variant_iter_new(interface_dict);
		GVariant* interface_kv_pair;
		while(interface_kv_pair = g_variant_iter_next_value(interface_iterator)) {
			gchar* UD2_interface;
			GVariant* properties_dict;
			g_variant_get(interface_kv_pair, "{s*}", &UD2_interface, &properties_dict);


			/* Is this a UD2 drive? */
			if (0 == g_strcmp0(UD2_DRIVE_IFACE, UD2_interface)) {
				GVariant* ejectable = g_variant_lookup_value(
				    properties_dict,
				    "Ejectable",
				    NULL);
				g_hash_table_insert(UD2_object_properties,
				    (gchar*) "ejectable",
				    GINT_TO_POINTER(g_variant_get_boolean(ejectable)));
				g_variant_unref(ejectable);

				GVariant* optical = g_variant_lookup_value(
				    properties_dict,
				    "Optical",
				    NULL);
				g_hash_table_insert(UD2_object_properties,
				    (gchar*) "optical",
				    GINT_TO_POINTER(g_variant_get_boolean(optical)));
				g_variant_unref(optical);

				UD2_drives = g_list_append(UD2_drives, UD2_object_properties);
			}

			/* Is this a block device? */
			if (0 == g_strcmp0(UD2_BLOCK_IFACE, UD2_interface)) {
				GVariant* preferred_path = g_variant_lookup_value(
				    properties_dict,
				    "PreferredDevice",
				    NULL);
				g_hash_table_insert(UD2_object_properties,
				    (gchar*) "dev_path",
				    g_variant_dup_bytestring(preferred_path, NULL));
				g_variant_unref(preferred_path);

				/* In addition to the preferred path, there may
 				 * be any number of human-readable symlinks under
				 * /dev
				 */
				GVariant* symlinks = g_variant_lookup_value(properties_dict, "Symlinks", NULL);
				//g_print("Symlinks has %d elements\n", g_variant_n_children(symlinks));

				GVariantIter* symlink_iterator = g_variant_iter_new(symlinks);
				GVariant* symlink;
				GList* symlink_list = NULL;
				while(symlink = g_variant_iter_next_value(symlink_iterator)) {
					symlink_list = g_list_append(symlink_list, (gpointer) g_variant_dup_bytestring(symlink, NULL));
					g_variant_unref(symlink);
				}
				g_variant_iter_free(symlink_iterator);
				g_variant_unref(symlinks);

				g_hash_table_insert(UD2_object_properties, (gchar*) "symlinks", symlink_list);

				UD2_blocks = g_list_append(UD2_blocks, UD2_object_properties);
			}

			/* TODO How shall we handle partition tables? */
			if (0 == g_strcmp0(UD2_PTABLE_IFACE, UD2_interface)) {
				g_hash_table_insert(UD2_object_properties,
				    (gchar*) "has_partition_table",
				    GINT_TO_POINTER((gboolean) TRUE));
			}

			/* Does this block device have a filesystem? */
			if (0 == g_strcmp0(UD2_FS_IFACE, UD2_interface)) {
				g_hash_table_insert(UD2_object_properties,
				    (gchar*) "has_filesystem",
				    GINT_TO_POINTER((gboolean) TRUE));

				/* Is the FS mounted? We don't really care where */
				GVariant* mountpoints = g_variant_lookup_value(
				    properties_dict,
				    "MountPoints",
				    NULL);
				if (0 < g_variant_n_children(mountpoints)) {
					g_hash_table_insert(UD2_object_properties,
					    (gchar*) "mounted",
					    GINT_TO_POINTER((gboolean) TRUE));
				} else {
					g_hash_table_insert(UD2_object_properties,
					    (gchar*) "mounted",
					    GINT_TO_POINTER((gboolean) FALSE));
				}
				g_variant_unref(mountpoints);
			}

			g_free(UD2_interface);
			g_variant_unref(interface_kv_pair);
		}
		g_variant_iter_free(interface_iterator);

next_object:
		g_variant_unref(interface_dict);
		g_free(dbus_object_path);
		g_variant_unref(kv_pair);
	}
	g_variant_iter_free(dict_iter);

	/* Populate the UD2_enumerations struct */
	enumerations->drives = UD2_drives;
	enumerations->block_devices = UD2_blocks;

	g_variant_unref(UD2_Objects);
	g_object_unref(UD2_Proxy);

	return enumerations;
}

gboolean eject(gchar* drive_dbus_path) {
	g_type_init();

	GDBusProxy* UD2_Drive_Proxy = NULL;
	GError* error=NULL;

	/* Is the drive ejectable? */
	UD2_Drive_Proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
		G_DBUS_PROXY_FLAGS_NONE,
		NULL,
		UD2_DBUS_NAME,
		drive_dbus_path,
		DBUS_PROP_IFACE,
		NULL,
		&error);

	if (!UD2_Drive_Proxy) {
		g_printerr("%s\n", error->message);
		g_error_free(error);
	}

	GVariant* UD2_Ejectable = g_dbus_proxy_call_sync(	UD2_Drive_Proxy,
							(gchar*) "Get",
							g_variant_new(	"(ss)",
									"org.freedesktop.UDisks2.Drive",
									"Ejectable"
							),
							G_DBUS_CALL_FLAGS_NONE,
							-1,
							NULL,
							&error);

	if (!UD2_Ejectable) {
		g_printerr("%s\n", error->message);
		g_error_free(error);
	}

	GVariant* ejectable;
	g_variant_get(UD2_Ejectable, "(v)", &ejectable);
	g_assert_cmpstr(g_variant_get_type_string(ejectable), ==, "b");
	gboolean is_ejectable = g_variant_get_boolean(ejectable);

	g_variant_unref(ejectable);
	g_variant_unref(UD2_Ejectable);
	g_object_unref(UD2_Drive_Proxy);

	if (FALSE==is_ejectable) {
		g_printerr("%s is not an ejectable drive\n", drive_dbus_path);
		return FALSE;
	}

	/* Proceed with the actual ejection */
	g_print("Ejecting %s\n", drive_dbus_path);
	GDBusProxy* FOO_PROXY= NULL;

	/* Attempt to get a DBus proxy for the drive's UDisks2.Drive iface */
	
	FOO_PROXY= g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
		G_DBUS_PROXY_FLAGS_NONE,
		NULL,
		UD2_DBUS_NAME,
		drive_dbus_path,
		UD2_DRIVE_IFACE,
		NULL,
		&error);

	if (!FOO_PROXY) {
		g_printerr("Couldn't get DBus proxy for %s:\n%s\n", drive_dbus_path, error->message);
		g_error_free(error);
		return FALSE;
	}

	/* We need to build a dictionary of options, albeit an empty one */
	GVariant* dict_entry = g_variant_new_dict_entry( g_variant_new("s", "auth.no_user_interaction"), g_variant_new("b", TRUE));
	GVariant* dict = g_variant_new_tuple(&dict_entry, 1);
	g_print("%s\n", g_variant_print(dict, TRUE));
	g_print("%s\n", g_variant_get_type_string(dict));

	GVariant* UD2_Eject_Success = g_dbus_proxy_call_sync(	FOO_PROXY,
							(gchar*) "Eject",
							dict,
							G_DBUS_CALL_FLAGS_NONE,
							-1,
							NULL,
							&error);

	if (!UD2_Eject_Success) {
		g_printerr("foo %s\n", error->message);
		g_error_free(error);
	}

	/*
	GVariant* version_string;
	g_variant_get(UD2_Eject_Success, "(v)", &version_string);
	*/
	/* version is allocated independently from the GVariants */
	/*
	gchar* version=g_variant_dup_string(version_string, NULL);

	g_variant_unref(version_string);
	*/
	g_variant_unref(UD2_Eject_Success);
	g_object_unref(FOO_PROXY);
	return TRUE;
}
