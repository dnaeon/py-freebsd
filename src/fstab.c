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

#include <fstab.h>

static PyObject *
PyObject_FromFstab(struct fstab *tab)
{
	PyObject *r;

	r = PyDict_New();
	SETDICT_STR(r, "spec", tab->fs_spec);
	SETDICT_STR(r, "file", tab->fs_file);
	SETDICT_STR(r, "vfstype", tab->fs_vfstype);
	SETDICT_STR(r, "mntops", tab->fs_mntops);
	SETDICT_STR(r, "type", tab->fs_type);
	SETDICT_INT(r, "freq", tab->fs_freq);
	SETDICT_INT(r, "passno", tab->fs_passno);

	return r;
}


static char PyFB_getfsent__doc__[] =
"getfsent():\n"
"return an object containing the broken-out fields of a line in the\n"
"file system description file, <fstab.h>.";

static PyObject *
PyFB_getfsent(PyObject *self)
{
	struct fstab *tab;
	PyObject *r, *m;

	if (setfsent() == -1)
		return OSERROR();

	r = PyList_New(0);
	while ((tab = getfsent())) {
		m = PyObject_FromFstab(tab);
		PyList_Append(r, m);
		Py_DECREF(m);
	}
	endfsent();

	return r;
}


static char PyFB_getfsspec__doc__[] =
"getfsspec(spec):\n"
"return an object containing the broken-out fields of a line in the\n"
"file system description file, <fstab.h>.";

static PyObject *
PyFB_getfsspec(PyObject *self, PyObject *args)
{
	struct fstab *tab;
	char *spec;

	if (! PyArg_ParseTuple(args, "s:getfsspec", &spec))
		return NULL;

	if ((tab = getfsspec(spec)) == NULL)
		return OSERROR();

	return PyObject_FromFstab(tab);
}


static char PyFB_getfsfile__doc__[] =
"getfsfile(file):\n"
"return an object containing the broken-out fields of a line in the\n"
"file system description file, <fstab.h>.";

static PyObject *
PyFB_getfsfile(PyObject *self, PyObject *args)
{
	struct fstab *tab;
	char *file;

	if (! PyArg_ParseTuple(args, "s:getfsfile", &file))
		return NULL;

	if ((tab = getfsfile(file)) == NULL)
		return OSERROR();

	return PyObject_FromFstab(tab);
}
