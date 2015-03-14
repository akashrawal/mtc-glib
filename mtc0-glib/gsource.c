/* gsource.c or gsource.h
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

#include "common.h"

#include <string.h>
#include <errno.h>

//Destructor prototype
static void mtc_g_source_destroy(MtcGSource *self);

//Child source for MtcGPeer
typedef struct
{
	GSource source;
	MtcGPeer peer;
} MtcGPeerSource;

#define MTC_G_PEER_GET_SOURCE(peer) \
	((GSource *) (((char *) (peer)) - offsetof(MtcGPeerSource, peer)))

#define MTC_G_PEER_FROM_SOURCE(source) \
	(&(((MtcGPeerSource *) (source))->peer))

const static struct {int mtc_flag, g_flag;} translator[] = {
				{MTC_POLLIN,   G_IO_IN},
				{MTC_POLLOUT,  G_IO_OUT},
				{MTC_POLLPRI,  G_IO_PRI},
				{MTC_POLLERR,  G_IO_ERR},
				{MTC_POLLHUP,  G_IO_HUP},
				{MTC_POLLNVAL, G_IO_NVAL}};
const static int n_flags = 6;

static gboolean mtc_g_peer_prepare(GSource *source, gint *timeout_)
{
	MtcGPeer *peer = MTC_G_PEER_FROM_SOURCE(source);
	MtcLinkTest *test_i;
	int fd_i;
	
	peer->tests = mtc_link_get_tests(peer->parent.link);
	
	//Prepare for all tests
	fd_i = 0;
	for (test_i = peer->tests; test_i; test_i = test_i->next)
	{
		if (strcmp(test_i->name, MTC_LINK_TEST_POLLFD) == 0)
		{
			MtcLinkTestPollFD *pollfd = (MtcLinkTestPollFD *) test_i;
			int i;
			
			peer->fds[fd_i].events = 0;
			for (i = 0; i < n_flags; i++)
				if (pollfd->events & translator[i].mtc_flag)
					peer->fds[fd_i].events |= translator[i].g_flag;
			
			peer->fds[fd_i].revents = 0;
			
			fd_i++;
		}
	}
	
	*timeout_ = -1;
	return FALSE;
}

static gboolean mtc_g_peer_check(GSource *source)
{
	MtcGPeer *peer = MTC_G_PEER_FROM_SOURCE(source);
	MtcLinkTest *test_i;
	int fd_i;
	
	//Translate test results back to MTC form
	fd_i = 0;
	for (test_i = peer->tests; test_i; test_i = test_i->next)
	{
		if (strcmp(test_i->name, MTC_LINK_TEST_POLLFD) == 0)
		{
			MtcLinkTestPollFD *pollfd = (MtcLinkTestPollFD *) test_i;
			int i;
			
			pollfd->revents = 0;
			for (i = 0; i < n_flags; i++)
				if (peer->fds[fd_i].revents & translator[i].g_flag)
					pollfd->revents |= translator[i].mtc_flag;
			
			fd_i++;
		}
	}
	
	//Evaluate test results
	peer->test_action = mtc_link_eval_test_result(peer->parent.link);
	
	return (peer->test_action ? TRUE : FALSE);
}

static gboolean mtc_g_peer_dispatch
	(GSource *source, GSourceFunc callback, gpointer user_data)
{
	MtcGPeer *peer = MTC_G_PEER_FROM_SOURCE(source);
	
	//Error condition
	if (peer->test_action & MTC_LINK_TEST_BREAK)
	{
		mtc_link_break(peer->parent.link);
		
		mtc_g_peer_remove(peer);
	}
	else
	{
		//Sending
		if (peer->test_action & MTC_LINK_TEST_SEND)
		{
			MtcLinkIOStatus status;
			
			status = mtc_link_send(peer->parent.link);
			
			if (status == MTC_LINK_IO_FAIL)
			{
				mtc_g_peer_remove(peer);
				peer->test_action = 0;
			}
		}
		
		//Receiving
		if (peer->test_action & MTC_LINK_TEST_RECV)
		{
			MtcLinkInData in_data;
			MtcLinkIOStatus status;

			while ((status = mtc_link_receive
					(peer->parent.link, &in_data)) 
				== MTC_LINK_IO_OK)
			{
				mtc_da_route(&(peer->source->da), (MtcPeer *) peer, &in_data);
			}
			
			if (status == MTC_LINK_IO_FAIL)
			{
				mtc_g_peer_remove(peer);
				peer->test_action = 0;
			}
		}
	}
	
	peer->test_action = 0;
	
	//Don't remove this source
	return TRUE;
}

static GSourceFuncs mtc_g_peer_source_funcs = 
{
	mtc_g_peer_prepare,
	mtc_g_peer_check,
	mtc_g_peer_dispatch,
	NULL
};

//No need to do anything here, child sources do things for us
static gboolean mtc_g_source_prepare(GSource *source, gint *timeout_)
{
	//Return values (constant)
	*timeout_ = -1;
	return FALSE;
}
static gboolean mtc_g_source_check(GSource *source)
{
	return FALSE;
}
static gboolean mtc_g_source_dispatch
	(GSource *source, GSourceFunc callback, gpointer user_data)
{
	return TRUE;
}
static void mtc_g_source_finalize(GSource *source)
{
	mtc_g_source_destroy((MtcGSource *) source);
}

static GSourceFuncs mtc_g_source_funcs = 
{
	mtc_g_source_prepare,
	mtc_g_source_check,
	mtc_g_source_dispatch,
	mtc_g_source_finalize
};

//Creates a new peer integrated with the event source.
MtcGPeer *mtc_g_peer_new(MtcGSource *self, MtcLink *link)
{
	GSource *peer_source;
	MtcGPeer *peer;
	MtcLinkTest *test_i, *tests;
	int n_fds, fd_i;
	
	//Metrics
	n_fds = 0;
	tests = mtc_link_get_tests(link);
	for (test_i = tests; test_i; test_i = test_i->next)
	{
		if (strcmp(test_i->name, MTC_LINK_TEST_POLLFD) == 0)
			n_fds++;
		else
		{
			mtc_warn("Unsupported test %s for link %p", 
				test_i->name, peer->parent.link);
		}
	}
	
	//Allocate a new peer/source
	peer_source = g_source_new(&mtc_g_peer_source_funcs, 
		sizeof(MtcGPeerSource) + ((n_fds - 2) * sizeof(GPollFD)));
	peer = MTC_G_PEER_FROM_SOURCE(peer_source);
	
	//Initialize it
	mtc_peer_init((MtcPeer *) peer, link, &(self->da));
	
	peer->source = self;
	peer->peer_data = NULL;
	peer->tests = tests;
	peer->test_action = 0;
	peer->n_fds = n_fds;
	fd_i = 0;
	for (test_i = tests; test_i; test_i = test_i->next)
	{
		if (strcmp(test_i->name, MTC_LINK_TEST_POLLFD) == 0)
		{
			MtcLinkTestPollFD *pollfd = (MtcLinkTestPollFD *) test_i;
			
			peer->fds[fd_i].fd = pollfd->fd;
			peer->fds[fd_i].events = 0;
			peer->fds[fd_i].revents = 0;
			g_source_add_poll(peer_source, peer->fds + fd_i);
			
			fd_i++;
		}
	}
	
	//Add as child source
	g_source_add_child_source((GSource *) self, peer_source);
	
	//Add it to linked list
	peer->next = &(self->peers);
	peer->prev = self->peers.prev;
	peer->next->prev = peer;
	peer->prev->next = peer;
	
	return peer;
}

//Removes a peer from the event source it is associated with and
void mtc_g_peer_remove(MtcGPeer *peer)
{
	GSource *peer_source = MTC_G_PEER_GET_SOURCE(peer);
	
	//Remove from list
	peer->prev->next = peer->next;
	peer->next->prev = peer->prev;
	
	//Free all resources
	if (! peer->source->finalizing)
		g_source_remove_child_source((GSource *) peer->source, peer_source);
	//g_source_destroy(peer_source);
	mtc_peer_destroy((MtcPeer *) peer);
	g_source_unref(peer_source);
}

//Creates a new event source for a switch.
MtcGSource *mtc_g_source_new(int n_static_objects)
{
	MtcGSource *self;
	
	//Create a new source
	self = (MtcGSource *) g_source_new(&mtc_g_source_funcs, sizeof(MtcGSource));
	
	//Initialize
	self->peers.next = self->peers.prev = &(self->peers);
	mtc_da_init(&(self->da), n_static_objects);
	self->finalizing = 0;
	
	g_source_set_can_recurse((GSource *) self, FALSE);
	
	return self;
}

//Destructor
static void mtc_g_source_destroy(MtcGSource *self)
{
	self->finalizing = 1;
	
	//Destroy all peers
	while (self->peers.next != &(self->peers))
		mtc_g_peer_remove(self->peers.next);
	
	//Destroy the DA
	mtc_da_destroy(&(self->da));
}

//Gets the destination allocator used by the event source.
MtcDA *mtc_g_source_get_da(MtcGSource *self)
{
	return &(self->da);
}

///\}
