/* gsimple.c
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

#include "common.h"

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stddef.h>
#include <sys/un.h>

static void mtc_g_create_local_address
	(const char *filename, struct sockaddr **addr, socklen_t *len)
{
	struct sockaddr_un *addr_mem;
	*len = offsetof(struct sockaddr_un, sun_path) 
		+ strlen(filename) + 1;
	
	addr_mem = (struct sockaddr_un *) mtc_alloc(*len);
	addr_mem->sun_family = AF_LOCAL;
	strcpy(addr_mem->sun_path, filename);
	
	*addr = (struct sockaddr *) addr_mem;
}

static int mtc_g_find_namespace(struct sockaddr *addr)
{
	//Get namespace
	if (addr->sa_family == AF_LOCAL)
		return PF_LOCAL;
	if (addr->sa_family == AF_INET)
		return PF_INET;
	if (addr->sa_family == AF_INET6)
		return PF_INET6;
	
	mtc_error("Unknown address format %d", addr->sa_family);
	return -1;
}

static gboolean mtc_g_listener_prepare(GSource *source, gint *timeout_)
{
	MtcGListener *self = (MtcGListener *) source;
	
	*timeout_ = -1;
	self->fd.revents = 0;
	return FALSE;
}

static gboolean mtc_g_listener_check(GSource *source)
{
	MtcGListener *self = (MtcGListener *) source;
	
	if (self->fd.revents & G_IO_IN)
		return TRUE;
	
	return FALSE;
}

static gboolean mtc_g_listener_dispatch
	(GSource *source, GSourceFunc func, gpointer data)
{
	MtcGListener *self = (MtcGListener *) source;
	MtcLink *link;
	MtcGPeer *peer;
		
	int fd = accept(self->fd.fd, NULL, NULL);
	if (fd < 0)
		mtc_error("accept(): %s", strerror(errno));
	
	mtc_fd_set_blocking(fd, 0);
	link = mtc_fd_link_new(fd, fd);
	mtc_fd_link_set_close_fd(link, 1);
	peer = mtc_g_peer_new(self->source, link);
	mtc_link_unref(link);
	
	if (func)
		return ((MtcGListenerConnectCB) func) 
			(self->source, peer, data);
	else
		return TRUE;
}

static void mtc_g_listener_finalize(GSource *source)
{
	MtcGListener *self = (MtcGListener *) source;
	
	close(self->fd.fd);
}

GSourceFuncs mtc_g_listener_fns = {
	mtc_g_listener_prepare,
	mtc_g_listener_check,
	mtc_g_listener_dispatch,
	mtc_g_listener_finalize};

GSource *mtc_g_listener_new
	(MtcGSource *source, struct sockaddr *addr, socklen_t len)
{
	int fd;
	MtcGListener *self;
	
	//Create socket
	fd = socket(mtc_g_find_namespace(addr), SOCK_STREAM, 0);
	if (fd < 0)
		mtc_error("socket(): %s", strerror(errno));
	if (bind(fd, addr, len) < 0)
		mtc_error("bind(): %s", strerror(errno));
	if (listen(fd, 100 /*Why*/) < 0)
		mtc_error("listen(): %s", strerror(errno));
	mtc_fd_set_blocking(fd, 0);
	
	//Create a new source
	self = (MtcGListener *) g_source_new
		(&mtc_g_listener_fns, sizeof(MtcGListener));
	
	//Initialize
	self->fd.fd = fd;
	self->fd.events = G_IO_IN;
	self->source = source;
	
	g_source_add_poll((GSource *) self, &(self->fd));
	g_source_set_can_recurse((GSource *) self, FALSE);
	
	return (GSource *) self;
}

GSource *mtc_g_listener_new_local
	(MtcGSource *source, const char *filename)
{
	GSource *res;
	struct sockaddr *addr;
	socklen_t addr_len;
	
	mtc_g_create_local_address(filename, &addr, &addr_len);
	res = mtc_g_listener_new(source, addr, addr_len);
	mtc_free(addr);
	
	return res;
}

MtcGPeer *mtc_g_peer_connect
	(MtcGSource *source, struct sockaddr *addr, socklen_t len,
	int nonblocking)
{
	int fd;
	MtcLink *link;
	MtcGPeer *peer;
	
	//Create socket
	fd = socket(mtc_g_find_namespace(addr), SOCK_STREAM, 0);
	if (fd < 0)
		mtc_error("socket(): %s", strerror(errno));
	
	//Non-blocking mode
	if (nonblocking)
		mtc_fd_set_blocking(fd, 0);
	
	//Connect to server
	if (connect(fd, addr, len) < 0)
		if (errno != EINPROGRESS)
			mtc_error("connect(): %s", strerror(errno));
	
	//Create link
	link = mtc_fd_link_new(fd, fd);
	peer = mtc_g_peer_new(source, link);
	mtc_link_unref(link);
	
	return peer;
}

MtcGPeer *mtc_g_peer_connect_local
	(MtcGSource *source, const char *filename, 
	int nonblocking)
{
	MtcGPeer *res;
	struct sockaddr *addr;
	socklen_t addr_len;
	
	mtc_g_create_local_address(filename, &addr, &addr_len);
	res = mtc_g_peer_connect(source, addr, addr_len, nonblocking);
	mtc_free(addr);
	
	return res;
}
