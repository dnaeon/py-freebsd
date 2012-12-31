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

#include <sys/ktrace.h>

EXPCONST(int KTROP_SET)
EXPCONST(int KTROP_CLEAR)
EXPCONST(int KTROP_CLEARFILE)

EXPCONST(int KTRFLAG_DESCEND)

EXPCONST(int KTRFAC_MASK)
EXPCONST(int KTRFAC_SYSCALL)
EXPCONST(int KTRFAC_SYSRET)
EXPCONST(int KTRFAC_NAMEI)
EXPCONST(int KTRFAC_GENIO)
EXPCONST(int KTRFAC_PSIG)
EXPCONST(int KTRFAC_CSW)
EXPCONST(int KTRFAC_USER)
EXPCONST(int KTRFAC_ROOT)
EXPCONST(int KTRFAC_INHERIT)
EXPCONST_IFAVAIL(int KTRFAC_DROP)


static char PyFB_ktrace__doc__[] =
"ktrace(tracefile, ops, trpoints, pid):\n"
"The ktrace() enables or disables tracing of one or more processes.\n"
"Users may only trace their own processes.  Only the super-user can\n"
"trace setuid or setgid programs.";

static PyObject *
PyFB_ktrace(PyObject *self, PyObject *args)
{
	char *tracefile;
	int ops, trpoints, pid;

	if (!PyArg_ParseTuple(args, "siii:ktrace", &tracefile, &ops,
			      &trpoints, &pid))
		return NULL;

	if (ktrace(tracefile, ops, trpoints, pid) == -1)
		return OSERROR();

	Py_RETURN_NONE;
}
