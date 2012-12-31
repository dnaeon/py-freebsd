/*-
 * Copyright (c) 2002-2005 Hye-Shik Chang
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

#include <sys/sysctl.h>

static char PyFB_getloadavg__doc__[] =
"getloadavg():\n"
"returns the number of processes in the system run queue averaged\n"
"over various periods of time.  The system imposes 3 samples,\n"
"representing averages over the last 1, 5, and 15 minutes, respectively.";

static PyObject *
PyFB_getloadavg(PyObject *self)
{
#define NSAMPLES 3
	double loadavg[NSAMPLES];
	int nsamples, i;
	PyObject *r;

	if ((nsamples = getloadavg(loadavg, NSAMPLES)) == -1)
		OSERROR();

	r = PyTuple_New(nsamples);
	for (i = 0; i < nsamples; i++)
		PyTuple_SET_ITEM(r, i, PyFloat_FromDouble(loadavg[i]));

	return r;
#undef NSAMPLES
}


extern int getosreldate(void);

static char PyFB_getosreldate__doc__[] =
"getosreldate():\n"
"returns an integer showing the version of the currently running\n"
"FreeBSD kernel.  Definitions of the values can be found in The\n"
"Porter's Handbook which is usually installed at\n"
"/usr/share/doc/en_US.ISO8859-1/books/porters-handbook/.";

static PyObject *
PyFB_getosreldate(PyObject *self)
{
	int osreldate;
	if ((osreldate = getosreldate()) == -1)
		return OSERROR();
	return PyInt_FromLong(osreldate);
}


/* Internal helper function for getting type of sysctl node */
static unsigned int
sysctltype(int *oid, size_t len)
{
	int qoid[CTL_MAXNAME+2];
	int i, r;
	char buf[BUFSIZ];
	size_t bufsize;

	qoid[0] = 0;
	qoid[1] = 4;
	for (i = 0; i < len; i++)
		qoid[i+2] = oid[i];

	bufsize = sizeof(buf);
	r = sysctl(qoid, len+2, buf, &bufsize, NULL, 0);
	if (r != 0)
		return 0;
	return *(unsigned int *)buf;
}

/* Internal helper function to get sysctl name by oid */
static PyObject *
_sysctlmibtoname(int *oid, size_t size)
{
	char name[BUFSIZ];
	int qoid[CTL_MAXNAME+2];
	size_t namelen;
	int i, r;

	qoid[0] = 0;
	qoid[1] = 1;
	for (i = 0; i < size; i++)
		qoid[i+2] = oid[i];

	namelen = sizeof(name);
	r = sysctl(qoid, size + 2, name, &namelen, NULL, 0);
	if (r == -1)
		return OSERROR();

	return PyString_FromStringAndSize(name, namelen - 1);
}

const size_t sysctl_type_sizes[CTLTYPE] = {
	0,
	0,			/* CTLTYPE_NODE */
	sizeof(int),		/* CTLTYPE_INT */
	0,			/* CTLTYPE_STRING */
	sizeof(quad_t),		/* CTLTYPE_QUAD */
	0,			/* CTLTYPE_OPAQUE */
	sizeof(unsigned int),	/* CTLTYPE_UINT */
	sizeof(long),		/* CTLTYPE_LONG */
	sizeof(unsigned long),	/* CTLTYPE_ULONG */
};

static int
parse_oid_sequence(PyObject *name, int *oid, size_t *size)
{
	int i;
	*size = PySequence_Size(name);
	for (i = 0; i < *size && i < CTL_MAXNAME; i++) {
		PyObject *el;
		el = PySequence_GetItem(name, i);
		if (el == NULL)
			return -1;
		if (PyInt_Check(el))
			oid[i] = (int)PyInt_AsLong(el);
		else if (PyLong_Check(el))
			oid[i] = (int)PyLong_AsLong(el);
		else {
			PyErr_SetString(PyExc_TypeError,
				"every oid elements must be integers");
			Py_DECREF(el);
			return -1;
		}
		Py_DECREF(el);
	}
	if (i == CTL_MAXNAME) {
		PyErr_SetString(PyExc_ValueError,
			"there're too many elements in oid");
		return -1;
	}

	return 0;
}

/* Internal helper function to parse "name" argument that represents OID. */
static int
parse_oid_argument(PyObject *name, int *oid, size_t *size)
{
	if (PyString_Check(name)) {
		int r;
		if (PyString_GET_SIZE(name) == 0) {
			*size = 0;
			return 0;
		}
		*size = CTL_MAXNAME;
		r = sysctlnametomib(PyString_AS_STRING(name), oid, size);
		if (r == -1) {
			OSERROR();
			return -1;
		}
	}
	else if (PySequence_Check(name))
		return parse_oid_sequence(name, oid, size);
	else {
		PyErr_SetString(PyExc_TypeError,
			"oid must be given as string or list of integers");
		return -1;
	}
	return 0;
}

/* Internal helper function to list children nodes of a "node" */
static PyObject *
sysctl_listnode(int *oid, size_t oidsize, int byname)
{
	PyObject *children;
	int i, r;
	int name1[CTL_MAXNAME+2], name2[CTL_MAXNAME+2];
	size_t len1, len2;

	name1[0] = 0;
	name1[1] = 2;
	if (oidsize > 0) {
		len1 = oidsize + 2;
		for (i = 0; i < oidsize; i++)
			name1[i+2] = oid[i];
	}
	else {
		name1[3] = 1;
		len1 = 3;
	}

	children = PyList_New(0);
	if (children == NULL)
		return NULL;

	for (;;) {
		PyObject *repr;

		len2 = sizeof(name2);
		r = sysctl(name1, len1, name2, &len2, NULL, 0);
		if (r == -1) {
			if (errno == ENOENT)
				break;
			OSERROR();
			goto error;
		}

		len2 /= sizeof(int);

		for (i = 0; i < oidsize; i++)
			if (oid[i] != name2[i])
				goto endoflist;

		if (byname) {
			repr = _sysctlmibtoname(name2, len2);
			if (repr == NULL)
				goto error;
		}
		else {
			repr = PyTuple_New(len2);
			if (repr == NULL)
				goto error;

			for (i = 0; i < len2; i++)
				PyTuple_SET_ITEM(repr, i,
						 PyInt_FromLong(name2[i]));
		}
		PyList_Append(children, repr);
		Py_DECREF(repr);

		for (i = 0; i < len2; i++)
			name1[i+2] = name2[i];
		len1 = 2 + len2;
	}

endoflist:
	if (PyErr_Occurred()) /* may MemoryError from PyInt_FromLong */
		goto error;

	return children;

error:
	Py_DECREF(children);
	return NULL;
}

static char PyFB_sysctl__doc__[] =
"sysctl(name[, new[, oldlen]]):\n"
"retrieves system information and allows processes with appropriate\n"
"privileges to set system information.  The information available\n"
"from sysctl() consists of integers, strings, and tables.  Information\n"
"may be retrieved and set from the command interface using the\n"
"sysctl(8) utility.\n"
"\n"
"The state is described using a ``Management Information Base'' (MIB)\n"
"style name, listed in `name`, which can be a list of integers or a\n"
"ASCII string.";

static PyObject *
PyFB_sysctl(PyObject *self, PyObject *args, PyObject *kwds)
{
	static char *kwlist[] = {"name", "new", "oldlen", NULL};

	PyObject *oid, *ret, *newobj = NULL;
	int oldlenhint = -1;
	unsigned int kind;
	void *oldp, *newp;
	size_t oldlen, newlen, qoidsize;
	union multitype val;
	int qoid[CTL_MAXNAME];

	if (!PyArg_ParseTupleAndKeywords(args, kwds,
			"O|Oi:sysctl", kwlist, &oid, &newobj, &oldlenhint))
		return NULL;

	if (parse_oid_argument(oid, qoid, &qoidsize) == -1)
		return NULL;

	kind = sysctltype(qoid, qoidsize);
	if (kind == 0)
		return OSERROR();
	else if ((kind & CTLTYPE) == CTLTYPE_NODE) {
		if (newobj != NULL && newobj != Py_None) {
			PyErr_SetString(PyExc_TypeError,
				"argument 2 must be None for this node");
			return NULL;
		}
		return sysctl_listnode(qoid, qoidsize,
				PyString_Check(oid) ? 1 : 0);
	}

	/* Convert "new" object to argument */
	if (newobj == NULL) {
		newp = NULL;
		newlen = 0;
	}
	else switch (kind & CTLTYPE) {
	case CTLTYPE_STRING:
	case CTLTYPE_OPAQUE:
		if (!PyString_Check(newobj)) {
			PyErr_SetString(PyExc_TypeError,
				"argument 2 must be string for this node");
			return NULL;
		}
		newp = PyString_AS_STRING(newobj);
		newlen = PyString_Size(newobj);
		if ((kind & CTLTYPE) == CTLTYPE_STRING)
			newlen++; /* except terminator */
		break;
	case CTLTYPE_INT:
	case CTLTYPE_UINT:
	case CTLTYPE_LONG:
	case CTLTYPE_ULONG:
	case CTLTYPE_QUAD:
		if (!PyInt_Check(newobj) && !PyLong_Check(newobj)) {
			PyErr_SetString(PyExc_TypeError,
				"argument 2 must be integer for this node");
			return NULL;
		}

		switch (kind & CTLTYPE) {
		case CTLTYPE_INT:
			val.m_int = (int)PyInt_AsLong(newobj);
			break;
		case CTLTYPE_UINT:
			val.m_uint = (unsigned int)
					PyLong_AsUnsignedLong(newobj);
			break;
		case CTLTYPE_LONG:
			val.m_long = PyInt_AsLong(newobj);
			break;
		case CTLTYPE_ULONG:
			val.m_ulong = PyLong_AsUnsignedLong(newobj);
			break;
		case CTLTYPE_QUAD:
			val.m_quad = (quad_t)PyLong_AsLongLong(newobj);
			break;
		}
		newp = &val;
		newlen = sysctl_type_sizes[kind & CTLTYPE];
		break;
	default:
		PyErr_SetString(PyExc_SystemError,
				"is a unknown type of sysctl node.");
		return NULL;
	}

	/* Prepare oldp and oldlen if needed */
	if (oldlenhint == 0) {		/* don't fetch old value at all */
		oldp = NULL;
		oldlen = 0;
	}
	else if ((kind & CTLFLAG_RD) == 0) { /* read action is prohibited */
		if (newp == NULL) {
			oldp = NULL;
			oldlen = 0;
		}
		else {
			errno = EPERM;
			return OSERROR();
		}
	}
	else {
		if (sysctl_type_sizes[kind & CTLTYPE] > 0) { /* numeric */
			if (oldlenhint != -1) {
				PyErr_SetString(PyExc_TypeError,
					"argument 3 must not be given for "
					"this node");
				return OSERROR();
			}
			oldlen = sysctl_type_sizes[kind & CTLTYPE];
		}
		else if (oldlenhint != -1)
			oldlen = oldlenhint;
		else if (newlen > 0)
			oldlen = newlen;
		else
			oldlen = 32;

		oldp = PyMem_Malloc(oldlen);
		if (oldp == NULL)
			return NULL;
	}

	/* Now, it's the time to run */
	for (;;) {
		int r;

		r = sysctl(qoid, qoidsize, oldp, &oldlen, newp, newlen);
		if (r == -1) {
			if (errno == ENOMEM) {
				void *tmp;
				/* just a fun rule to choose next size. :-) */
				oldlen = oldlen * 3 / 2 + 7;
				tmp = PyMem_Realloc(oldp, oldlen);
				if (tmp == NULL) {
					PyErr_NoMemory();
					goto error;
				}
				else
					oldp = tmp;
			}
			else {
				OSERROR();
				goto error;
			}
		}
		else
			break;
	}

	if (oldp == NULL)
		Py_RETURN_NONE;

	/* Convert "old" value gotten to python object */
	switch (kind & CTLTYPE) {
	case CTLTYPE_INT:
		assert(oldlen == sizeof(int));
		ret = PyInt_FromLong((long)*(int *)oldp);
		break;
	case CTLTYPE_STRING:
		ret = PyString_FromStringAndSize(oldp, oldlen - 1);
		break;
	case CTLTYPE_QUAD:
		assert(oldlen == sizeof(quad_t));
		ret = PyLong_FromLongLong((long long)*(quad_t *)oldp);
		break;
	case CTLTYPE_OPAQUE:
		ret = PyString_FromStringAndSize(oldp, oldlen);
		break;
	case CTLTYPE_UINT:
		assert(oldlen == sizeof(unsigned int));
		ret = PyLong_FromUnsignedLong(
			(unsigned long)*(unsigned int *)oldp);
		break;
	case CTLTYPE_LONG:
		assert(oldlen == sizeof(long));
		ret = PyInt_FromLong(*(long *)oldp);
		break;
	case CTLTYPE_ULONG:
		assert(oldlen == sizeof(unsigned long));
		ret = PyLong_FromUnsignedLong(*(unsigned int *)oldp);
		break;
	default: /* unreachable */
		abort();
		return NULL;
	}

	PyMem_Del(oldp);
	return ret;

error:
	if (oldp != NULL)
		PyMem_Del(oldp);
	return NULL;
}

static char PyFB_sysctlnametomib__doc__[] =
"sysctlnametomib(mib):\n"
"accepts an ASCII representation of the name, looks up the integer\n"
"name vector, and returns the numeric representation in the `mib`.";

static PyObject *
PyFB_sysctlnametomib(PyObject *self, PyObject *args)
{
	PyObject *ret;
	char *name;
	int qoid[CTL_MAXNAME], i;
	size_t qoidsize;

	if (!PyArg_ParseTuple(args, "s:sysctlnametomib", &name))
		return NULL;

	qoidsize = CTL_MAXNAME;
	if (sysctlnametomib(name, qoid, &qoidsize) == -1)
		return OSERROR();

	ret = PyTuple_New(qoidsize);
	if (ret == NULL)
		return NULL;

	for (i = 0; i < qoidsize; i++)
		PyTuple_SET_ITEM(ret, i, PyInt_FromLong(qoid[i]));

	if (PyErr_Occurred()) {
		Py_DECREF(ret);
		return NULL;
	}
	return ret;
}

static char PyFB_sysctlmibtoname__doc__[] =
"sysctlmibtoname(name):\n"
"accepts an numeric representation of MIB, looks up the ASCII name\n"
"vector, and returns the ASCII representation in the `name`.";

static PyObject *
PyFB_sysctlmibtoname(PyObject *self, PyObject *args)
{
	PyObject *oidobj;
	int oid[CTL_MAXNAME], r;
	size_t oidsize;

	if (!PyArg_ParseTuple(args, "O:sysctlmibtoname", &oidobj))
		return NULL;

	if (!PySequence_Check(oidobj)) {
		PyErr_SetString(PyExc_TypeError,
			"argument 1 must be sequence type");
		return NULL;
	}

	r = parse_oid_sequence(oidobj, oid, &oidsize);
	if (r == -1)
		return NULL;

	return _sysctlmibtoname(oid, oidsize);
}

static char PyFB_sysctldescr__doc__[] =
"sysctldescr(mib):\n"
"returns simple description of the sysctl `mib` which can be specified\n"
"by a string or a list of integers.";

static PyObject *
PyFB_sysctldescr(PyObject *self, PyObject *args)
{
	PyObject *oidobj;
	int oid[CTL_MAXNAME+2], r;
	char descr[BUFSIZ];
	size_t oidsize, descrlen;

	if (!PyArg_ParseTuple(args, "O:sysctldescr", &oidobj))
		return NULL;

	if (parse_oid_argument(oidobj, oid+2, &oidsize) == -1)
		return NULL;

	oid[0] = 0;
	oid[1] = 5;
	oidsize += 2;

	descrlen = BUFSIZ;
	r = sysctl(oid, oidsize, descr, &descrlen, NULL, 0);
	if (r == -1)
		return OSERROR();

	return PyString_FromStringAndSize(descr, descrlen - 1);
}
