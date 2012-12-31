/*-
 * Copyright (c) 2005 Hye-Shik Chang
 * Copyright (c) 2000 Doug White
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#include <sys/event.h>

#define MAX_KEVENTS 512

/* Event filters */
EXPCONST(int EVFILT_READ)
EXPCONST(int EVFILT_WRITE)
EXPCONST(int EVFILT_AIO)
EXPCONST(int EVFILT_VNODE)
EXPCONST(int EVFILT_PROC)
EXPCONST(int EVFILT_SIGNAL)

/* Event flags */
EXPCONST(int EV_ADD)
EXPCONST(int EV_DELETE)
EXPCONST(int EV_ENABLE)
EXPCONST(int EV_DISABLE)
EXPCONST(int EV_ONESHOT)
EXPCONST(int EV_CLEAR)
EXPCONST(int EV_SYSFLAGS)
EXPCONST(int EV_FLAG1)
EXPCONST(int EV_EOF)
EXPCONST(int EV_ERROR)

/* Kernel note flags (for VNODE & PROC filter types) */
EXPCONST(int NOTE_DELETE)
EXPCONST(int NOTE_WRITE)
EXPCONST(int NOTE_EXTEND)
EXPCONST(int NOTE_ATTRIB)
EXPCONST(int NOTE_LINK)
EXPCONST(int NOTE_RENAME)

EXPCONST(int NOTE_EXIT)
EXPCONST(int NOTE_FORK)
EXPCONST(int NOTE_EXEC)
EXPCONST(int NOTE_PCTRLMASK)
EXPCONST(int NOTE_PDATAMASK)

EXPCONST(int NOTE_TRACK)
EXPCONST(int NOTE_TRACKERR)
EXPCONST(int NOTE_CHILD)

EXPCONST_IFAVAIL(int NOTE_LINKUP)
EXPCONST_IFAVAIL(int NOTE_LINKDOWN)
EXPCONST_IFAVAIL(int NOTE_LINKINV)

/* Types */
DECLTYPE(KEventType, keventobject)
DECLTYPE(KQueueType, kqueueobject)

static char *keventkwlist[] = {
	"ident", "filter", "flags", "fflags", "data",
	"udata", NULL,
};

/* ---------------------------------------------------------------------- */
/*				keventobject				  */
/* ---------------------------------------------------------------------- */

typedef struct {
	PyObject_HEAD
	struct kevent e;
} keventobject;

static PyTypeObject KEventType;

#define KEvent_Check(v)  ((v)->ob_type == &KEventType)

/* kevent methods */

#define create_blank_kevent() \
		(keventobject *)kevent_new(&KEventType, NULL, NULL)
static PyObject *
kevent_new(PyTypeObject *type, PyObject *args, PyObject *kw)
{
	keventobject *ev;

	ev = (keventobject *)type->tp_alloc(type, 0);
	if (ev == NULL)
		return NULL;

	/* defaults */
	ev->e.ident = 0;
	ev->e.filter = EVFILT_READ;
	ev->e.flags = EV_ADD | EV_ENABLE;
	ev->e.fflags = 0;
	ev->e.data = 0;
	ev->e.udata = NULL;

	if (args != NULL &&
	    !PyArg_ParseTupleAndKeywords(args, kw, "i|hhiiO:kevent",
			keventkwlist, &(ev->e.ident), &(ev->e.filter),
			&(ev->e.flags), &(ev->e.fflags), &(ev->e.data),
			&(ev->e.udata))) {
		Py_DECREF(ev);
		return NULL;
	}

	Py_XINCREF((PyObject *)ev->e.udata);
	return (PyObject *)ev;
}

static void
kevent_dealloc(keventobject *self)
{
	Py_XDECREF((PyObject *)self->e.udata);
	self->ob_type->tp_free((PyObject *)self);
}

static int
kevent_traverse(keventobject *self, visitproc visit, void *arg)
{
	if (self->e.udata != NULL)
		Py_VISIT((PyObject *)self->e.udata);
	return 0;
}

#define OFF(x) offsetof(keventobject, x)
static struct PyMemberDef kevent_memberlist[] = {
	{"ident",	T_UINT,		OFF(e.ident),	0,
	 "Value used to identify this event.  The exact interpretation "
	 "is determined by the attached filter, but often is a file "
	 "descriptor."},
	{"filter",	T_SHORT,	OFF(e.filter),	0,
	 "Identifies the kernel filter used to process this event.  The "
	 "pre-defined system filters are described below."},
	{"flags",	T_USHORT,	OFF(e.flags),	0,
	 "Actions to perform on the event."},
	{"fflags",	T_UINT,		OFF(e.fflags),	0,
	 "Filter-specific flags."},
	{"data",	T_INT,		OFF(e.data),	0,
	 "Filter-specific data value."},
	{"udata",	T_OBJECT,	OFF(e.udata),	0,
	 "Opaque user-defined value passed through the kernel unchanged."},
	{NULL}	/* sentinel */
};
#undef OFF

#define F(x) {x, #x},
const static struct KEventFilterRepr {
	short filter;
	const char *name;
} kevent_filter_repr[] = {
	F(EVFILT_READ)		F(EVFILT_WRITE)
	F(EVFILT_AIO)		F(EVFILT_VNODE)
	F(EVFILT_PROC)		F(EVFILT_SIGNAL)
	{ 0, "UNKNOWN" }
};
#undef F

const static struct FlagRepr kevent_flags_repr[] = {
	FLAGREPR(EV_ADD)
	FLAGREPR(EV_DELETE)
	FLAGREPR(EV_ENABLE)
	FLAGREPR(EV_DISABLE)
	FLAGREPR(EV_ONESHOT)
	FLAGREPR(EV_CLEAR)
	FLAGREPR(EV_SYSFLAGS)
	FLAGREPR(EV_FLAG1)
	FLAGREPR(EV_EOF)
	FLAGREPR(EV_ERROR)
	{ 0, }
};

static PyObject *
kevent_repr(keventobject *s)
{
	const struct KEventFilterRepr *rc;
	PyObject *flags, *udata, *r;
	char unknownfilter[15];
	const char *filtername;

	for (rc = kevent_filter_repr; rc->filter; rc++)
		if (rc->filter == s->e.filter) {
			filtername = rc->name;
			break;
		}
	if (rc->filter == 0) {
		sprintf(unknownfilter, "%d", s->e.filter);
		filtername = unknownfilter;
	}

	flags = repr_flag(kevent_flags_repr, s->e.flags);
	if (flags == NULL)
		return NULL;

	if (s->e.udata != NULL)
		udata = PyObject_Repr((PyObject *)s->e.udata);
	else
		udata = PyString_FromString("None");
	if (udata == NULL) {
		Py_DECREF(udata);
		return NULL;
	}

#ifdef __amd64__
#define IDENTTYPE "%ld"
#else
#define IDENTTYPE "%d"
#endif
	r = PyString_FromFormat(
		"<kevent ident=" IDENTTYPE " filter=%s flags=%s fflags=%x "
		"data=%x udata=%s>",
		s->e.ident, filtername, PyString_AS_STRING(flags), s->e.fflags,
		(int)s->e.data, PyString_AS_STRING(udata));
	Py_DECREF(flags);
	Py_DECREF(udata);
	return r;
}

static char kevent_doc[] =
"kevent(ident[, filter[, flags[, fflags[, data[, udata]]]]]):\n"
"this object keeps each kevents to be used in return value and\n"
"argument for kqueue.event().";

static PyTypeObject KEventType = {
	PyObject_HEAD_INIT(NULL)
	tp_name:	"kevent",
	tp_basicsize:	sizeof(keventobject),
	tp_dealloc:	(destructor)kevent_dealloc,
	tp_getattro:	PyObject_GenericGetAttr,
	tp_repr:	(reprfunc)kevent_repr,
	tp_flags:	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,
	tp_traverse:	(traverseproc)kevent_traverse,
	tp_members:	kevent_memberlist,
	tp_new:		kevent_new,
	tp_doc:		kevent_doc,
};


/* ---------------------------------------------------------------------- */
/*				kqueueobject				  */
/* ---------------------------------------------------------------------- */

typedef struct {
	PyObject_HEAD
	int fd;
	PyObject *udrefkeep;
} kqueueobject;

static PyTypeObject KQueueType;

#define KQueue_Check(v)	((v)->ob_type == &KQueueType)

/* kqueue methods */

static PyObject *
kqueue_new(PyTypeObject *type, PyObject *args, PyObject *kw)
{
	kqueueobject *kq;

	kq = (kqueueobject *)type->tp_alloc(type, 0);
	if (kq == NULL)
		return NULL;

	if (PyTuple_Size(args) > 0 ||
	    (kw != NULL && PyDict_Size(kw) > 0)) {
		PyErr_BadArgument();
		return NULL;
	}

	kq->fd = kqueue();
	if (kq->fd == -1) {
		Py_DECREF(kq);
		return OSERROR();
	}

	kq->udrefkeep = PyDict_New();
	if (kq->udrefkeep == NULL)
		return NULL;
	return (PyObject *)kq;
}

static void
kqueue_dealloc(kqueueobject *self)
{
	if (self->fd != -1) {
		close(self->fd);
		self->fd = -1;
	}
	Py_XDECREF(self->udrefkeep);
	self->ob_type->tp_free((PyObject *)self);
}

static int
kqueue_traverse(kqueueobject *self, visitproc visit, void *arg)
{
	Py_VISIT(self->udrefkeep);
	return 0;
}

static char kqueue_event_doc[] =
"event(changelist, nevents, timeout)\n"
"is used to register events with the queue, and return any pending\n"
"events to the user.  The `changelist` argument is a list of kevent\n"
"objects or None.  All changes contained in the `changelist` are\n"
"applied before any pending events are read from the queue.\n"
"\n"
"When `nevents` is zero, event() will return immediately even if\n"
"there is a timeout specified unlike select.select().  If timeout\n"
"is zero or positive, it specifies a maximum interval to wait for\n"
"an event.  If timeout is negative, event() waits indefinitely.  To\n"
"effect a poll, the timeout argument should be zero.  The same object\n"
"may be used for the changelist and return value.";

/* Call kevent(2) and do appropriate digestion of lists. */
#define UDATAREFKEY(ev)	(PyString_FromStringAndSize((char *)&(ev), \
			 sizeof(uintptr_t)+sizeof(short)))

static PyObject *
kqueue_event(kqueueobject *self, PyObject *args) 
{
	PyObject *kelist, *output;
	struct kevent *changelist;
	struct kevent *triggered;
	struct timespec totimespec, *tspec;
	int i, haveNumEvents, gotNumEvents;
	int wantNumEvents = 1, timeout = -1;

	if (!PyArg_ParseTuple(args, "O|ii:event", &kelist, &wantNumEvents,
				&timeout))
		return NULL;

	if (PyList_Check(kelist))
		haveNumEvents = PyList_GET_SIZE(kelist);
	else if (kelist == Py_None)
		haveNumEvents = 0;
	else {
		PyErr_SetString(PyExc_TypeError,
			"argument 1 must be list or None");
		return NULL;
	}

	/* If there's no events to process, don't bother. */
	if (haveNumEvents > 0) {
		changelist = PyMem_New(struct kevent, haveNumEvents);
		if (changelist == NULL)
			return PyErr_NoMemory();

		for (i = 0; i < haveNumEvents; i++) {
			PyObject *ei = PyList_GET_ITEM(kelist, i);
			keventobject *ev = (keventobject *)ei;

			if (!KEvent_Check(ei)) {
				PyErr_SetString(PyExc_TypeError,
					"arg 1 must be a list of `kevent` "
					"objects");
				PyMem_Del(changelist);
				return NULL;
			}

			/* copy this kevent into the array */
			memcpy(&(changelist[i]),
				&(ev->e), sizeof(struct kevent));

			if (ev->e.udata != NULL && (ev->e.flags & EV_ADD)) {
				PyErr_SetString(PyExc_ValueError,
					"use `addevent` method to "
					"add an event with udata");
				PyMem_Del(changelist);
				return NULL;
			}

			if (ev->e.flags & EV_DELETE) {
				PyObject *key;
				int r;
				key = UDATAREFKEY(ev->e);
				if (key == NULL) {
					PyMem_Del(changelist);
					return NULL;
				}
				r = PyDict_DelItem(self->udrefkeep,
						   key);
				if (r == -1)
					PyErr_Clear();
				Py_DECREF(key);
			}
		}
	}
	else
		changelist = NULL;

	/* Allocate some space to hold the triggered events */
	triggered = PyMem_New(struct kevent, wantNumEvents);
	if (triggered == NULL) {
		PyMem_Del(changelist);
		return PyErr_NoMemory();
	}

	/* Build timespec for timeout */
	if (timeout >= 0) {
		totimespec.tv_sec = timeout / 1000;
		totimespec.tv_nsec = (timeout % 1000) * 1000000;
		tspec = &totimespec;
	}
	else
		tspec = NULL;

	/* Make the call */
	Py_BEGIN_ALLOW_THREADS
	gotNumEvents = kevent(self->fd, changelist, haveNumEvents,
			      triggered, wantNumEvents, tspec);
	Py_END_ALLOW_THREADS

	/* Don't need the input event list anymore, so get rid of it */
	PyMem_Del(changelist);

	if (gotNumEvents == -1) {
		PyMem_Del(triggered);
		return OSERROR();
	}
	else if (gotNumEvents == 0) {
		/* return empty list */
		PyMem_Del(triggered);
		return PyList_New(0);
	}

	/* Succeeded, got something back; return it in a list */
	output = PyList_New(gotNumEvents);
	if (output == NULL) {
		PyMem_Del(triggered);
		return PyErr_NoMemory();
	}

	for (i = 0; i < gotNumEvents; i++) {
		keventobject *ke = create_blank_kevent();

		if (ke == NULL) {
			PyMem_Del(triggered);
			Py_DECREF(output);
			return NULL;
		}
		else {
			/* copy event data into our struct */
			memmove((void*)&(ke->e), &(triggered[i]),
				sizeof(struct kevent));
			Py_XINCREF((PyObject *)ke->e.udata);
			PyList_SET_ITEM(output, i, (PyObject *)ke);
		}
	}

	PyMem_Del(triggered);
	/* pass back the results */
	return output;
}

static char kqueue_addevent_doc[] =
"addevent(event or ident[, filter[, flags[, fflags[, data[, udata]]]]]):\n"
"is used to register events with the queue.  This function is like\n"
"event() except that it's allowed to register kevent with non-None\n"
"udata object.";

static PyObject *
kqueue_addevent(kqueueobject *self, PyObject *args, PyObject *kw) 
{
	struct kevent change;
	PyObject *key;
	int r;

	if (PyTuple_Size(args) == 1 &&
	    KEvent_Check(PyTuple_GET_ITEM(args, 0))) {
		keventobject *ke = (keventobject *)PyTuple_GET_ITEM(args, 0);
		memcpy(&change, &ke->e, sizeof(change));
	}
	else {
		change.filter = EVFILT_READ;
		change.flags = EV_ADD | EV_ENABLE;
		change.fflags = 0;
		change.data = 0;
		change.udata = NULL;

		if (!PyArg_ParseTupleAndKeywords(args, kw, "i|hhiiO:addevent",
				keventkwlist, &change.ident, &change.filter,
				&change.flags, &change.fflags, &change.data,
				&change.udata))
			return NULL;
		change.flags |= EV_ADD;
	}

	if (change.udata != NULL) {
		key = UDATAREFKEY(change);
		if (key == NULL)
			return NULL;
	}
	else
		key = NULL;

	Py_BEGIN_ALLOW_THREADS
	r = kevent(self->fd, &change, 1, NULL, 0, NULL);
	Py_END_ALLOW_THREADS

	if (r == -1) {
		Py_XDECREF(key);
		return OSERROR();
	}

	if (key != NULL) {
		r = PyDict_SetItem(self->udrefkeep, key,
				   (PyObject *)change.udata);
		Py_DECREF(key);
		if (r == -1)
			return NULL;
	}

	Py_RETURN_NONE;
}

static PyMethodDef kqueue_methods[] = {
	{"event", (PyCFunction)kqueue_event, METH_VARARGS,
	 kqueue_event_doc},
	{"addevent", (PyCFunction)kqueue_addevent, METH_VARARGS|METH_KEYWORDS,
	 kqueue_addevent_doc},
	{NULL, NULL}
};

static char kqueue_doc[] =
"kqueue():\n"
"The kqueue object provides a generic method of notifying the user\n"
"when an event happens or a condition holds, based on the results\n"
"of small pieces of kernel code termed filters.  A kevent is identified\n"
"by the (ident, filter) pair; there may only be one unique kevent\n"
"per kqueue.";

static PyTypeObject KQueueType = {
	PyObject_HEAD_INIT(NULL)
	tp_name:	"kqueue",
	tp_basicsize:	sizeof(kqueueobject),
	tp_dealloc:	(destructor)kqueue_dealloc,
	tp_getattro:	PyObject_GenericGetAttr,
	tp_flags:	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,
	tp_traverse:	(traverseproc)kqueue_traverse,
	tp_methods:	kqueue_methods,
	tp_new:		kqueue_new,
	tp_doc:		kqueue_doc,
};
