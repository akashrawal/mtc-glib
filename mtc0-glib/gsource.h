/* gsource.h
 * MtcGSource: Integrating links and access points with GMainContext
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
 * \addtogroup mtc_g_source
 * \{
 * 
 * This is an event source that can be integrated with GLib main loop.
 * 
 * Use mtc_g_source_new() to create a new event source.
 * 
 * Then you can use mtc_g_peer_new() to add peers.
 * 
 * Initially the source is not associated with any context. Use 
 * g_source_attach() to attach it to a context and g_source_destroy() to
 * remove it from the context.
 * 
 * When done, destroy it using g_source_unref().
 * 
 * However g_source_set_callback() does not work.
 */

///MTC event source for links and access points
typedef struct _MtcGSource MtcGSource;

typedef struct _MtcGPeer MtcGPeer;

///Peer, integrated with MtcGSource
struct _MtcGPeer
{
	///Parent structure
	MtcPeer parent;
	///Back pointer to the event source
	MtcGSource *source;
	///You can use this to store your data.
	gpointer peer_data;
	
#ifndef DOXYGEN_SKIP_THIS
	MtcGPeer *next;
	MtcGPeer *prev;
	MtcLinkTest *tests;
	int test_action;
	int n_fds;
	GPollFD fds[2];
#endif
};

struct _MtcGSource
{
	GSource parent;
	
	MtcDA da;
	
	MtcGPeer peers;
	int finalizing;
};

/**Creates a new peer integrated with the event source.
 * \param self The event source.
 * \param link The link to attach.
 * \return A new peer
 */
MtcGPeer *mtc_g_peer_new(MtcGSource *self, MtcLink *link);

/**Removes a peer from the event source it is associated with and
 * frees all associated memory. You should not access the peer after
 * calling this function.
 * \param peer The peer to remove
 */
void mtc_g_peer_remove(MtcGPeer *peer);

/**Creates a new event source.
 * 
 * The switch is used for routing incoming messages.
 * \param n_static_objects Number of static objects which will be
 *        added to the destination allocator
 * \return A new event source, destroyable with g_source_unref().
 */
MtcGSource *mtc_g_source_new(int n_static_objects);

/**Gets the destination allocator used by the event source.
 * \param self The event source
 * \return The destination allocator used by the event source.
 */
MtcDA *mtc_g_source_get_da(MtcGSource *self);

///\}
