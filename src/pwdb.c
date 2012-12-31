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

#include <pwd.h>

static PyObject *
PyObject_FromPasswd(struct passwd *pwd)
{
	PyObject *r;

	r = PyDict_New();
	SETDICT_STR(r, "name", pwd->pw_name);
	SETDICT_STR(r, "passwd", pwd->pw_passwd);
	SETDICT_INT(r, "uid", pwd->pw_uid);
	SETDICT_INT(r, "gid", pwd->pw_gid);
	SETDICT_INT(r, "change", pwd->pw_change);
	SETDICT_STR(r, "class", pwd->pw_class);
	SETDICT_STR(r, "gecos", pwd->pw_gecos);
	SETDICT_STR(r, "dir", pwd->pw_dir);
	SETDICT_STR(r, "shell", pwd->pw_shell);
	SETDICT_INT(r, "expire", pwd->pw_expire);
	SETDICT_INT(r, "fields", pwd->pw_fields);

	return r;
}


static char PyFB_getpwuid__doc__[] =
"getpwuid(uid):\n"
"search the password database for the given user `uid`, respectively,\n"
"always returning the first one encountered.";

static PyObject *
PyFB_getpwuid(PyObject *self, PyObject *args)
{
	struct passwd *pwd;
	int uid;

	if (!PyArg_ParseTuple(args, "i:getpwuid", &uid))
		return NULL;

	errno = 0;
	pwd = getpwuid((uid_t)uid);
	if (pwd == NULL) {
		if (errno == 0) {
			PyObject *uidn = PyInt_FromLong(uid);
			PyErr_SetObject(PyExc_KeyError, uidn);
			Py_DECREF(uidn);
			return NULL;
		}
		else
			return OSERROR();
	}
	return PyObject_FromPasswd(pwd);
}

static char PyFB_getpwnam__doc__[] =
"getpwnam(name):\n"
"search the password database for the given login name, always\n"
"returning the first one encountered.";

static PyObject *
PyFB_getpwnam(PyObject *self, PyObject *args)
{
	struct passwd *pwd;
	char *name;

	if (!PyArg_ParseTuple(args, "s:getpwnam", &name))
		return NULL;

	errno = 0;
	pwd = getpwnam(name);
	if (pwd == NULL) {
		if (errno == 0) {
			PyErr_SetString(PyExc_KeyError, name);
			return NULL;
		}
		else
			return OSERROR();
	}

	return PyObject_FromPasswd(pwd);
}

static char PyFB_getpwent__doc__[] =
"getpwent():\n"
"reads the whole password database and is intended for programs\n"
"that wish to process the complete list of users.";

static PyObject *
PyFB_getpwent(PyObject *self)
{
	PyObject *r, *m;
	struct passwd *pwd;

	setpwent();
	r = PyList_New(0);

	while ((pwd = getpwent())) {
		m = PyObject_FromPasswd(pwd);
		PyList_Append(r, m);
		Py_DECREF(m);
	}
	endpwent();

	return r;
}
