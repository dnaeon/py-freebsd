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

#include <sys/jail.h>

static char PyFB_jail__doc__[] =
"jail(path, hostname, ip_number):\n"
"The jail() system call sets up a jail and locks the current process\n"
"in it.  The ``path'' should be set to the directory which is to be\n"
"the root of the prison.  The ``hostname'' can be set to the hostname\n"
"of the prison.  This can be changed from the inside of the prison.\n"
"The ``ip_number'' can be set to the IP number assigned to the prison.";

static PyObject *
PyFB_jail(PyObject *self, PyObject *args)
{
	struct jail jp;
	char *ipaddr;

	if (!PyArg_ParseTuple(args, "sss:jail", &(jp.path),
			      &(jp.hostname), &ipaddr))
		return NULL;

	jp.version   = 0;
	jp.ip_number = inet_addr(ipaddr);

	if (jp.ip_number == INADDR_NONE) {
		PyErr_SetString(PyExc_ValueError, "malformed internet address");
		return NULL;
	}

	if (jail(&jp) == -1)
		return OSERROR();

	Py_RETURN_NONE;
}
