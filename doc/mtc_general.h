/**
 * \addtogroup mtc_general
 * \{
 * MTC is a high performance general purpose IPC library.
 * 
 * \defgroup mtc_building Compiling and installing MTC GLib itself
 * \ingroup mtc_general
 * \{
 * MTC is a cross-unix library.
 * 
 * MTC uses GNU Autotools build system. After extracting the release tarball,
 * open a terminal in the extracted directory and type:
 * ~~~~~{.sh}
 * ./configure
 * make
 * make install
 * ~~~~~
 * You can type `./configure --help` to get complete list of compile-time options.
 * 
 * \}
 * 
 * \defgroup mtc_usage Using MTC GLib in applications
 * \ingroup mtc_general
 * \{
 * To use MTC Standalone in your applications you have to do two things.
 * 
 * First, include <mtc0-glib/mtc-glib.h> header file in your sources
 * 
 * Then link against _mtc0-glib_ package by using pkg-config utility.
 * 
 * e.g. this is how to compile a single file program:
 * ~~~~~{.sh}
 * cc prog.c \`pkg-config --libs mtc0-glib\` -o prog
 * ~~~~~
 * 
 * \}
 * 
 * \}
 */
