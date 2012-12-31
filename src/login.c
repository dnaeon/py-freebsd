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

static char PyFB_getlogin__doc__[] =
"getlogin():\n"
"returns the login name of the user associated with the current\n"
"session, as previously set by setlogin().  The name is normally\n"
"associated with a login shell at the time a session is created, and\n"
"is inherited by all processes descended from the login shell.  (This\n"
"is true even if some of those processes assume another user ID, for\n"
"example when su(1) is used).";

static PyObject *
PyFB_getlogin(PyObject *self)
{
	char logname[MAXLOGNAME+1];

	if (getlogin_r(logname, MAXLOGNAME+1) == -1)
		return OSERROR();

	return PyString_FromString(logname);
}

static char PyFB_setlogin__doc__[] =
"setlogin(name):\n"
"The setlogin() sets the login name of the user associated with the\n"
"current session to name.  This function is restricted to the\n"
"super-user, and is normally used only when a new session is being\n"
"created on behalf of the named user (for example, at login time,\n"
"or when a remote shell is invoked).\n"
"\n"
"NOTE: There is only one login name per session.";

static PyObject *
PyFB_setlogin(PyObject *self, PyObject *args)
{
	char *logname;

	if (!PyArg_ParseTuple(args, "s:setlogin", &logname))
		return NULL;

	if (setlogin(logname) == -1)
		return OSERROR();
	Py_RETURN_NONE;
}
