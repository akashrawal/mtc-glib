/* gevent.h
 * Event-driven framework implementation based on GMainLoop
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

/**
 * \addtogroup mtc_g_event
 * \{
 * 
 */

/**Creates a new event backend manager based on GMainLoop.
 * \param context A GMainContext for backends to attach to, or 
 *         NULL to use default backend
 */
MtcEventMgr *mtc_g_event_mgr_new(GMainContext *context);

/**
 * \}
 */
