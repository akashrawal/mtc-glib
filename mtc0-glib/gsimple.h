/* gsimple.h
 * Implementation of simple listeners and connectors
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
 * \addtogroup mtc_g_simple
 * \{
 * 
 * This section documents simple mechanisms for setting up simple
 * servers and clients. More complicated software may not find this
 * useful.
 */

/**A simple event source that listens for connections to 
 * a given socket address and immediately creates a file 
 * descriptor link and corresponding MtcGPeer for incoming connections.
 */
typedef struct
{
	GSource parent;
	GPollFD fd;
	MtcGSource *source;
} MtcGListener;

/**Type of function to register as callback using 
 * g_source_set_callback().
 * \param source The MtcGSource to which MtcGPeer is associated
 * \param peer Newly connected peer
 * \param data User data
 * \return FALSE if the event source should be removed
 */
typedef gboolean (*MtcGListenerConnectCB) 
	(MtcGSource *source, MtcGPeer *peer, gpointer data);

/**Creates a new listener source for given address. 
 * \param source MtcGSource to which peers should be added
 * \param addr Socket address, will be passed to bind()
 * \param len Length of socket address
 * \return A new listener. 
 */
GSource *mtc_g_listener_new
	(MtcGSource *source, struct sockaddr *addr, socklen_t len);

/**Creates a new listener source for given local address. 
 * \param source MtcGSource to which peers should be added
 * \param filename The local socket will be bound to this filename.
 * \return A new listener. 
 */
GSource *mtc_g_listener_new_local
	(MtcGSource *source, const char *filename);

/**Connects to a socket with given address and creates a corresponding
 * peer.
 * \param source MtcGSource to which peers should be added
 * \param addr Socket address, will be passed to connect()
 * \param len Length of socket address
 * \param nonblocking 1 if the connection should not block. Even if
 *        connection has not finished, you can start queueing data
 *        immediately, it will be sent when connection completes.
 * \return A newly connected peer. 
 */
MtcGPeer *mtc_g_peer_connect
	(MtcGSource *source, struct sockaddr *addr, socklen_t len,
	int nonblocking);

/**Connects to a local socket with given filename and creates a 
 * corresponding peer.
 * \param source MtcGSource to which peers should be added
 * \param filename The socket file to connect
 * \param nonblocking 1 if the connection should not block. Even if
 *        connection has not finished, you can start queueing data
 *        immediately, it will be sent when connection completes.
 * \return A newly connected peer. 
 */
MtcGPeer *mtc_g_peer_connect_local
	(MtcGSource *source, const char *filename, 
	int nonblocking);

///\}
