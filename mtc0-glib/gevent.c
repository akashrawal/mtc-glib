/* gevent.c
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

#include "common.h"

#include <string.h>

//Translator between GPollFD and MtcEventTestPollFD events
const static struct {int mtc_flag, g_flag;} translator[] = {
				{MTC_POLLIN,   G_IO_IN},
				{MTC_POLLOUT,  G_IO_OUT},
				{MTC_POLLPRI,  G_IO_PRI},
				{MTC_POLLERR,  G_IO_ERR},
				{MTC_POLLHUP,  G_IO_HUP},
				{MTC_POLLNVAL, G_IO_NVAL}};
const static int n_flags = 6;

static int mtc_poll_to_glib(int events)
{
	int i, res = 0;
	
	for (i = 0; i < n_flags; i++)
		if (events & translator[i].mtc_flag)
			res |= translator[i].g_flag;
	
	return res;
}

static int mtc_poll_from_glib(int events)
{
	int i, res = 0;
	
	for (i = 0; i < n_flags; i++)
		if (events & translator[i].g_flag)
			res |= translator[i].mtc_flag;
	
	return res;
}

//Event implementation
typedef struct _MtcGEventImpl MtcGEventImpl;

typedef struct 
{
	MtcEventTestPollFD *test;
	GPollFD fd;
} MtcGPollFD;

typedef struct
{
	GSource parent;
	
	MtcGEventImpl *impl;
	int n_fd_tests;
	MtcGPollFD fd_tests[];
} MtcGSource;

struct _MtcGEventImpl
{
	MtcEventSource *source;
	MtcGSource *g_source;
	GMainContext *context;
	MtcEventTest *tests;
	int is_polling;
	int update_pending;
};

typedef struct 
{
	MtcEventMgr parent;
	
	GMainContext *context;
} MtcGEventMgr;

extern GSourceFuncs mtc_g_source_funcs;

static void mtc_g_event_create_g_source
	(MtcGEventImpl *impl, int n_fd_tests)
{
	int i;
	GSource *glib_source;
	
	glib_source = g_source_new(&mtc_g_source_funcs, sizeof(MtcGSource) 
			+ (n_fd_tests * sizeof(MtcGPollFD)));
	impl->g_source = (MtcGSource *) glib_source;
	
	impl->g_source->impl = impl;
	impl->g_source->n_fd_tests = n_fd_tests;
	
	for (i = 0; i < n_fd_tests; i++)
	{
		g_source_add_poll
			(glib_source, &(impl->g_source->fd_tests[i].fd));
	}
	
	g_source_attach(glib_source, impl->context);
}

static void mtc_g_event_update(MtcGEventImpl *impl)
{
	int n_fd_tests = 0;
	int fd_test_id;
	MtcEventTest *test_iter;
	
	//Metrics
	for (test_iter = impl->tests; test_iter; test_iter = test_iter->next)
	{
		if (strcmp(test_iter->name, MTC_EVENT_TEST_POLLFD) != 0)
		{
			mtc_error("Unsupported test %s for event source %p",
				test_iter->name, impl->source);
		}
		n_fd_tests++;
	}
	
	//Create/recreate impl->g_source as needed
	if (! impl->g_source)
	{
		mtc_g_event_create_g_source(impl, n_fd_tests);
	}
	else
	{
		if (n_fd_tests != impl->g_source->n_fd_tests)
		{
			g_source_destroy((GSource *) impl->g_source);
			g_source_unref((GSource *) impl->g_source);
			mtc_g_event_create_g_source(impl, n_fd_tests);
		}
	}
	
	//Fill data
	fd_test_id = 0;
	for (test_iter = impl->tests; test_iter; test_iter = test_iter->next)
	{
		if (strcmp(test_iter->name, MTC_EVENT_TEST_POLLFD) == 0)
		{
			#define cur_test (impl->g_source->fd_tests[fd_test_id])
			MtcEventTestPollFD *fd_test 
				= (MtcEventTestPollFD *) test_iter;
			
			cur_test.test = fd_test;
			cur_test.fd.fd = fd_test->fd;
			cur_test.fd.events = mtc_poll_to_glib(fd_test->events);
			cur_test.fd.revents = 0;
			
			#undef cur_test
		}
		fd_test_id++;
	}
	
	//Done
	impl->update_pending = 0;
}

static gboolean mtc_g_source_prepare(GSource *glib_source, gint *timeout)
{
	MtcGSource *g_source = (MtcGSource *) glib_source;
	MtcGEventImpl *impl = g_source->impl;
	
	if (! impl->is_polling)
	{
		impl->is_polling = 1;
		
		//Send 'poll-begin' message
		mtc_event_source_event(impl->source, MTC_EVENT_POLL_BEGIN);
		
		if (impl->update_pending)
			mtc_g_event_update(impl);
	}
	
	*timeout = -1;
	return FALSE;
}

static gboolean mtc_g_source_check(GSource *glib_source)
{
	MtcGSource *g_source = (MtcGSource *) glib_source;
	MtcGEventImpl *impl = g_source->impl;
	int i, msg = MTC_EVENT_POLL_END;
	
	impl->is_polling = 0;
	
	//Translate all revents
	//Also check if we need to send 'check' message.
	for (i = 0; i < g_source->n_fd_tests; i++)
	{
		if ((g_source->fd_tests[i].test->revents
			= mtc_poll_from_glib(g_source->fd_tests[i].fd.revents)))
			msg |= MTC_EVENT_CHECK;
	}
	
	//Send 'poll-end' message (and maybe 'check' signal)
	mtc_event_source_event(impl->source, msg);
	
	//The job is done. No use of dispatching.
	return FALSE;
}

static gboolean mtc_g_source_dispatch
	(GSource *glib_source, GSourceFunc callback, gpointer user_data)
{
	return G_SOURCE_CONTINUE;
}

//REFER: Finalize can be null, but we might need it later
/*
static void mtc_g_source_finalize(GSource *glib_source)
{
	MtcGSource *g_source = (MtcGSource *) glib_source;
	MtcGEventImpl *impl = g_source->impl;
	
}
*/

GSourceFuncs mtc_g_source_funcs = 
{
	mtc_g_source_prepare,
	mtc_g_source_check, 
	mtc_g_source_dispatch,
	//REFER: Finalize
	//mtc_g_source_finalize
	NULL
};

static void mtc_g_event_prepare
	(MtcEventSource *source, MtcEventTest *tests)
{
	MtcGEventImpl *impl = (MtcGEventImpl *) 
		mtc_event_source_get_impl(source);

	impl->tests = tests;

	if (impl->is_polling)
		//REFER: Main loop should have stopped here
		mtc_g_event_update(impl);
	else
		impl->update_pending = 1;
}

void mtc_g_event_init_source
	(MtcEventSource *source, MtcEventMgr *mgr)
{
	MtcGEventMgr *gmgr = (MtcGEventMgr *) mgr;
	MtcGEventImpl *impl = mtc_event_source_get_impl(source);
	
	impl->source = source;
	impl->context = gmgr->context;
	g_main_context_ref(impl->context);
	impl->g_source = NULL;
	impl->tests = NULL;
	impl->is_polling = 0;
	impl->update_pending = 1;
	mtc_g_event_update(impl);
}

void mtc_g_event_destroy_source(MtcEventSource *source)
{
	MtcGEventImpl *impl = mtc_event_source_get_impl(source);
	
	g_main_context_unref(impl->context);
	
	if (impl->g_source)
	{
		g_source_destroy((GSource *) impl->g_source);
		g_source_unref((GSource *) impl->g_source);
	}
}

void mtc_g_event_mgr_destroy(MtcEventMgr *mgr)
{
	MtcGEventMgr *gmgr = (MtcGEventMgr *) mgr;
	
	g_main_context_unref(gmgr->context);
	
	mtc_event_mgr_destroy(mgr);
	
	mtc_free(gmgr);
}

static MtcEventImpl mtc_g_event_impl = 
{
	sizeof(MtcGEventImpl),
	mtc_g_event_prepare,
	mtc_g_event_init_source,
	mtc_g_event_destroy_source,
	mtc_g_event_mgr_destroy
};

MtcEventMgr *mtc_g_event_mgr_new(GMainContext *context)
{
	MtcGEventMgr *gmgr = (MtcGEventMgr *) mtc_alloc(sizeof(MtcGEventMgr));
	
	mtc_event_mgr_init((MtcEventMgr *) gmgr, &mtc_g_event_impl);
	
	if (context)
	{
		gmgr->context = context;
		g_main_context_ref(context);
	}
	else
	{
		gmgr->context = g_main_context_default();
	}
	
	return (MtcEventMgr *) gmgr;
}
