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

#include <sys/resource.h>

/* constants */
EXPCONST(int RLIMIT_CPU)
EXPCONST(int RLIMIT_FSIZE)
EXPCONST(int RLIMIT_DATA)
EXPCONST(int RLIMIT_STACK)
EXPCONST(int RLIMIT_CORE)
EXPCONST(int RLIMIT_RSS)
EXPCONST(int RLIMIT_MEMLOCK)
EXPCONST(int RLIMIT_NPROC)
EXPCONST(int RLIMIT_NOFILE)
EXPCONST(int RLIMIT_SBSIZE)
EXPCONST(int RLIMIT_VMEM)
EXPCONST_IFAVAIL(int RLIMIT_AS)

EXPCONST(int PRIO_PROCESS)
EXPCONST(int PRIO_PGRP)
EXPCONST(int PRIO_USER)

EXPCONST(int RUSAGE_SELF)
EXPCONST(int RUSAGE_CHILDREN)


/* implementation */
static char PyFB_getrlimit__doc__[] =
"getrlimit(resource):\n"
"obtains limits on the consumption of system resources by the current\n"
"process and each process it creates.\n";

static PyObject *
PyFB_getrlimit(PyObject *self, PyObject *args)
{
	struct rlimit rlp;
	PyObject *r;
	int resource;

	if (! PyArg_ParseTuple(args, "i:getrlimit", &resource))
		return NULL;

	if (getrlimit(resource, &rlp) == -1)
		return OSERROR();

	r = PyTuple_New(2);

	if (rlp.rlim_cur == RLIM_INFINITY) {
		Py_INCREF(Py_None);
		PyTuple_SET_ITEM(r, 0, Py_None);
	}
	else
		PyTuple_SET_ITEM(r, 0, PyLong_FromLongLong(rlp.rlim_cur));

	if (rlp.rlim_max == RLIM_INFINITY) {
		Py_INCREF(Py_None);
		PyTuple_SET_ITEM(r, 1, Py_None);
	}
	else
		PyTuple_SET_ITEM(r, 1, PyLong_FromLongLong(rlp.rlim_max));

	return r;
}


static char PyFB_setrlimit__doc__[] =
"setrlimit(resource, rlim_cur, rlim_max):\n"
"sets limits on the consumption of system resources by the current\n"
"process and each process it creates.\n";

static PyObject *
PyFB_setrlimit(PyObject *self, PyObject *args)
{
	struct rlimit rlp;
	int resource;

	if (!PyArg_ParseTuple(args, "iLL:setrlimit", &resource,
			      &(rlp.rlim_cur), &(rlp.rlim_max)))
		return NULL;

	if (setrlimit(resource, &rlp) == -1)
		return OSERROR();

	Py_RETURN_NONE;
}


static char PyFB_getpriority__doc__[] =
"getpriority(which, who):\n"
"returns the highest priority (lowest numerical value) enjoyed by\n"
"any of the specified processes.";

static PyObject *
PyFB_getpriority(PyObject *self, PyObject *args)
{
	int which, who, r;

	if (!PyArg_ParseTuple(args,"ii:getpriority", &which, &who))
		return NULL;

	r = getpriority(which, who);
	if (errno == -1) /* getpriority can return -1 legitimately */
		return OSERROR();
	else
		return PyInt_FromLong((long) r);
}


static char PyFB_setpriority__doc__[] =
"setpriority(which, who, prio):\n"
"sets the priorities of all of the specified processes to the specified\n"
"value.  Only the super-user may lower priorities.";

static PyObject *
PyFB_setpriority(PyObject *self, PyObject *args)
{
	int which, who, prio, r;

	if (!PyArg_ParseTuple(args,"iii:setpriority", &which, &who, &prio))
		return NULL;

	if ((r = setpriority(which, who, prio)) != 0)
		return OSERROR();
	Py_RETURN_NONE;
}


static char PyFB_getrusage__doc__[] =
"getrusage(who):\n"
"returns information describing the resources utilized by the current\n"
"process, or all its terminated child processes.  The `who` argument\n"
"is either const.RUSAGE_SELF or const.RUSAGE_CHILDREN.";

static PyObject *
PyFB_getrusage(PyObject *self, PyObject *args)
{
	struct rusage rup;
	PyObject *r;
	int who;

	if (!PyArg_ParseTuple(args, "i:getrusage", &who))
		return NULL;

	if (getrusage(who, &rup) == -1)
		return OSERROR();

	r = PyDict_New();

	SETDICT_DOUBLE(r, "utime",
		((double)rup.ru_utime.tv_sec) +
		((double)rup.ru_utime.tv_usec / 1000000.0));
	SETDICT_DOUBLE(r, "stime",
		((double)rup.ru_stime.tv_sec) +
		((double)rup.ru_stime.tv_usec / 1000000.0));
	SETDICT_INT(r, "maxrss", rup.ru_maxrss);
	SETDICT_INT(r, "ixrss", rup.ru_ixrss);
	SETDICT_INT(r, "idrss", rup.ru_idrss);
	SETDICT_INT(r, "isrss", rup.ru_isrss);
	SETDICT_INT(r, "minflt", rup.ru_minflt);
	SETDICT_INT(r, "majflt", rup.ru_majflt);
	SETDICT_INT(r, "nswap", rup.ru_nswap);
	SETDICT_INT(r, "inblock", rup.ru_inblock);
	SETDICT_INT(r, "oublock", rup.ru_oublock);
	SETDICT_INT(r, "msgsnd", rup.ru_msgsnd);
	SETDICT_INT(r, "msgrcv", rup.ru_msgrcv);
	SETDICT_INT(r, "nsignals", rup.ru_nsignals);
	SETDICT_INT(r, "nvcsw", rup.ru_nvcsw);
	SETDICT_INT(r, "nivcsw", rup.ru_nivcsw);

	return r;
}
