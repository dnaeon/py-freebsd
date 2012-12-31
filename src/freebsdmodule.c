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

#include <iso646.h>
#include <sys/param.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <fcntl.h>

#include "Python.h"
#include "structmember.h"
#include "version.h"

union multitype {
	int m_int;
	unsigned int m_uint;
	long m_long;
	unsigned long m_ulong;
	quad_t m_quad;
};

struct FlagRepr {
        unsigned int flag;
	const char *repr;
};
#define FLAGREPR(f) { f, #f },

#ifdef __GNUC__
#define __inline__ static inline
#else
#define __inline__ static
#endif

/* 2.3 compatibility */
#ifndef Py_RETURN_NONE
#define Py_RETURN_NONE do { Py_INCREF(Py_None); return Py_None; } while (0);
#endif

/* 2.3 compatibility */
#define Py_VISIT(op)                                    \
	do {                                            \
		if (op) {                               \
			int vret = visit((op), arg);    \
			if (vret)                       \
				return vret;		\
		}                                       \
	} while (0)

static PyObject *
repr_flag(const struct FlagRepr *flags, unsigned int v)
{
	PyObject *l;

	l = PyList_New(0);
	if (l == NULL)
		return NULL;

	for (; flags->flag; flags++)
		if (v & flags->flag) {
			PyObject *f;
			f = PyString_FromString(flags->repr);
			if (f == NULL) {
				Py_DECREF(l);
				return NULL;
			}
			PyList_Append(l, f);
			Py_DECREF(f);
		}

	if (PyList_GET_SIZE(l) == 0) {
		Py_DECREF(l);
		return PyString_FromString("0");
	}
	else {
		PyObject *sep, *r;

		sep = PyString_FromString("|");
		if (sep == NULL)
			return NULL;
		r = _PyString_Join(sep, l);
		Py_DECREF(l);
		Py_DECREF(sep);
		return r;
	}
}

__inline__ void
PyDict_SetItemString_StealRef(PyObject *d, char *name, PyObject *o)
{
	PyDict_SetItemString(d, name, o);
	Py_DECREF(o);
}

/* int -> int */
#define SETDICT_INT(dict, name, value)					\
	PyDict_SetItemString_StealRef(dict, name,			\
			PyInt_FromLong((long) value))

/* unsigned int -> long */
#define SETDICT_UINT(dict, name, value)					\
	PyDict_SetItemString_StealRef(dict, name,			\
			PyLong_FromUnsignedLong((unsigned long) value))

#define SETDICT_LONG SETDICT_INT /* long -> int */
#define SETDICT_ULONG SETDICT_UINT /* unsigned long -> long */

/* double -> float */
#define SETDICT_DOUBLE(dict, name, value)				\
	PyDict_SetItemString_StealRef(dict, name,			\
			PyFloat_FromDouble((double) value))

/* char* (zero-terminated) -> string */
#define SETDICT_STR(dict, name, value)					\
	PyDict_SetItemString_StealRef(dict, name,			\
			PyString_FromString(value))

/* char* (opaque) -> string */
#define SETDICT_OPAQUE(dict, name, value, size)				\
	PyDict_SetItemString_StealRef(dict, name,			\
			PyString_FromStringAndSize(value, size))

/* char -> string */
#define SETDICT_CHAR(dict, name, value)					\
	PyDict_SetItemString_StealRef(dict, name,			\
			PyString_FromFormat("%c", value))


/* Dummy macros to ignore constants exports which are processed by external
 * generator*/
#define EXPCONST(decl)
#define EXPCONST_IFAVAIL(decl)
#define EXPCONST_MIN_OSREL(osrev, decl)
#define EXPCONST_MAX_OSREL(osrev, decl)
#define DECLTYPE(typeobject, objstruct)
#define LIB_DEPENDS(lib)

#define OSERROR() PyErr_SetFromErrno(PyExc_OSError)

#include ".sources.def"

/* Module Methods */
static struct PyMethodDef freebsd_methods[] = {
#include ".methods.def"
	{0, 0},
};

static PyObject *
create_newmodule(char *name)
{
	PyObject *mod, *moddict;

	mod = PyModule_New(name);
	if (mod == NULL)
		return NULL;

	/* Register into module dict to allow such codes like:
	 *   from freebsd.const import *
	 */
	moddict = PyImport_GetModuleDict();
	if (moddict == NULL) {
		Py_DECREF(mod);
		return NULL;
	}

	PyDict_SetItemString(moddict, name, mod);
	return mod;
}

#define INITTYPE(type, obj)						\
	if (PyType_Ready(&(type)) < 0)					\
		return;							\
	PyModule_AddObject(m, (type).tp_name, (PyObject *)&(type));

void
initfreebsd(void)
{
	PyObject *m, *constmod;

	m = Py_InitModule4("freebsd", freebsd_methods, 0, 0,
			   PYTHON_API_VERSION);

	#include ".types.def"

	/* Create a virtual submodule that provides constants */
	constmod = create_newmodule("freebsd.const");
	if (constmod == NULL)
		return;
	PyModule_AddObject(m, "const", constmod);

	{
		PyObject *d = PyModule_GetDict(constmod);
		#include ".const.def"
	}

	PyModule_AddStringConstant(m, "__version__", __version__);

	if (PyErr_Occurred())
		Py_FatalError("can't initialize the freebsd module");
}
