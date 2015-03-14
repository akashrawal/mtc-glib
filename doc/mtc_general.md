\addtogroup mtc_general
\{
\defgroup mtc_building Compiling and installing MTC-GLib itself
\ingroup mtc_general
\{
MTC-GLib is a cross-unix library.

MTC-GLib uses GNU Autotools build system. After extracting the release tarball,
open a terminal in the extracted directory and type:
~~~~~{.sh}
./configure
make
make install
~~~~~
You can type `./configure --help` to get complete list of compile-time options.

Cross-compiling of MTC-GLib is not supported at this instant. Any suggestions
to make it possible would be welcome.
\}

\defgroup mtc_usage Using MTC-GLib in applications
\ingroup mtc_general
\{
To use MTC-GLib in your applications you have to do two things.

First, include <mtc0/mtc.h> header file in your sources

Then link against _mtc0-glib_ package by using pkg-config utility.

e.g. this is how to compile a single file program:
~~~~~{.sh}
cc prog.c \`pkg-config --libs mtc0\` -o prog
~~~~~

\}

\}
