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

static char PyFB_gethostname__doc__[] =
"gethostname():\n"
"returns the standard host name for the current processor, as\n"
"previously set by sethostname().";

static PyObject *
PyFB_gethostname(PyObject *self)
{
	char buf[BUFSIZ];

	if (gethostname(buf, BUFSIZ) == -1)
		return OSERROR();

	return PyString_FromString(buf);
}

static char PyFB_sethostname__doc__[] =
"sethostname(name):\n"
"sets the name of the host machine to be name, which has length\n"
"namelen.  This call is restricted to the super-user and is normally\n"
"used only when the system is bootstrapped.";

static PyObject *
PyFB_sethostname(PyObject *self, PyObject *args)
{
	char *buf;
	int buflen;

	if (!PyArg_ParseTuple(args, "s#:sethostname", &buf, &buflen))
		return NULL;

	if (sethostname(buf, buflen) == -1)
		return OSERROR();

	Py_RETURN_NONE;
}
