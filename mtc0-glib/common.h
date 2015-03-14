/* common.h
 * Common includes
 * 
 * Copyright 2013 Akash Rawal
 * This file is part of MTC-GLib.
 * 
 * MTC-GLib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * MTC-GLib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with MTC-GLib.  If not, see <http://www.gnu.org/licenses/>.
 */

//Common header files
#include <mtc0/mtc.h>
#include <glib.h>
#include <sys/socket.h>

#ifndef _MTC_PUBLIC
#include <string.h>
#include <error.h>
#include <assert.h>
#endif

#define _MTC_HEADER
//Target related header files
#include "gsource.h"
#include "gsimple.h"

#undef _MTC_HEADER
