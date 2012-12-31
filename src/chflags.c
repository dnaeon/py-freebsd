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

EXPCONST(int UF_NODUMP)
EXPCONST(int UF_IMMUTABLE)
EXPCONST(int UF_APPEND)
EXPCONST(int UF_NOUNLINK)
EXPCONST(int UF_OPAQUE)

EXPCONST(int SF_ARCHIVED)
EXPCONST(int SF_IMMUTABLE)
EXPCONST(int SF_APPEND)
EXPCONST(int SF_NOUNLINK)

static char PyFB_chflags__doc__[] =
"chflags(path, flags):\n"
"The file whose name is given by `path` has its flags changed to\n"
"`flags`.";

static PyObject *
PyFB_chflags(PyObject *self, PyObject *args)
{
	char *path;
	unsigned long flags;

	if (!PyArg_ParseTuple(args, "sk:chflags", &path, &flags))
		return NULL;

	if (chflags(path, flags) == -1)
		return OSERROR();
	Py_RETURN_NONE;
}

static char PyFB_lchflags__doc__[] =
"lchflags(path, flags):\n"
"The file whose name is given by `path` has its flags changed\n"
"to `flags`.\n"
"\n"
"The lchflags() is like chflags() except in the case where the\n"
"named file is a symbolic link, in which case lchflags() will\n"
"change the flags of the link itself, rather than the file it\n"
"points to.";

static PyObject *
PyFB_lchflags(PyObject *self, PyObject *args)
{
	char *path;
	int flags;

	if (!PyArg_ParseTuple(args, "si:lchflags", &path, &flags))
		return NULL;

#if __FreeBSD_version >= 500035
	if (lchflags(path, flags) == -1)
		return OSERROR();
	Py_RETURN_NONE;
#else
	PyErr_SetString(PyExc_NotImplementedError,
			"unimplemented function in this OS revision yet.");
	return NULL;
#endif
}

static char PyFB_fchflags__doc__[] =
"fchflags(fd, flags):\n"
"The file referenced by the descriptor `fd` has its flags changed\n"
"to `flags`.";

static PyObject *
PyFB_fchflags(PyObject *self, PyObject *args)
{
	int fd;
	unsigned long flags;

	if (!PyArg_ParseTuple(args, "ik:fchflags", &fd, &flags))
		return NULL;

	if (fchflags(fd, flags) == -1)
		return OSERROR();
	Py_RETURN_NONE;
}
