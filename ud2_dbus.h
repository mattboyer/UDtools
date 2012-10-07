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

#include "glib.h"
#include <gio/gio.h>

#define UD2_DBUS_NAME "org.freedesktop.UDisks2"
#define UD2_DRIVE_IFACE "org.freedesktop.UDisks2.Drive"
#define UD2_BLOCK_IFACE "org.freedesktop.UDisks2.Block"
#define UD2_FS_IFACE "org.freedesktop.UDisks2.Filesystem"
#define UD2_PTABLE_IFACE "org.freedesktop.UDisks2.PartitionTable"

#define UD2_PATH "/org/freedesktop/UDisks2"
#define UD2_MANAGER_PATH "/org/freedesktop/UDisks2/Manager"
#define UD2_DRIVE_PREFIX "/org/freedesktop/UDisks2/drives/"

#define DBUS_PROP_IFACE "org.freedesktop.DBus.Properties"
#define DBUS_MANAGER_IFACE "org.freedesktop.DBus.ObjectManager"

struct UD2_enumerations {
	GList* drives;
	GList* block_devices;
};

gchar* get_version();
struct UD2_enumerations* enum_objects();
