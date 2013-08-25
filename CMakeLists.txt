cmake_minimum_required (VERSION 2.6)
project (UDTOOLS)
set (UDTOOLS_VERSION_MAJOR 0)
set (UDTOOLS_VERSION_MINOR 20)
execute_process (	COMMAND hg heads --template {rev}
			WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
			OUTPUT_VARIABLE HG_REVISION)
set (UDTOOLS_VERSION_CHANGESET ${HG_REVISION})

configure_file ("${PROJECT_SOURCE_DIR}/udtools_config.h.in"
    "${PROJECT_BINARY_DIR}/udtools_config.h")
include_directories (${PROJECT_BINARY_DIR})

find_package (PkgConfig)

pkg_check_modules (GLIB2 REQUIRED glib-2.0)
if (GLIB2_FOUND)
	include_directories (${GLIB2_INCLUDE_DIRS})
	link_directories (${GLIB2_LIBRARY_DIRS})
endif (GLIB2_FOUND)

set (	udenum_SRCS
	udenum.c
	ud2_dbus.c
)

set (	udeject_SRCS
	udeject.c
	ud2_dbus.c
)

set (	udpart_SRCS
	udpart.c
	ud2_dbus.c
)
add_executable (udenum ${udenum_SRCS})
add_executable (udeject ${udeject_SRCS})
add_executable (udpart ${udpart_SRCS})

target_link_libraries (udenum glib-2.0 gobject-2.0 gio-2.0)
target_link_libraries (udeject glib-2.0 gobject-2.0 gio-2.0)
target_link_libraries (udpart glib-2.0 gobject-2.0 gio-2.0)

install (TARGETS udenum udeject udpart RUNTIME DESTINATION bin)
install (FILES udcomplete.sh DESTINATION lib)

# vim:set tabstop=8 softtabstop=8 shiftwidth=8 noexpandtab list:
