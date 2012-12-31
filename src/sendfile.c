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

#include <sys/uio.h>

/* TODO: headers and trailers are not supported yet. */
static char PyFB_sendfile__doc__[] =
"sendfile(fd, s, offset, nbytes[, headers, tailers]):\n"
"sends a regular file specified by descriptor `fd` out a stream\n"
"socket specified by descriptor `s`.";

static PyObject *
PyFB_sendfile(PyObject *self, PyObject *args)
{
	int fd, s, res;
	off_t offset, sbytes;
	size_t nbytes;
	PyObject *headers = NULL, *tailers = NULL;

	if (!PyArg_ParseTuple(args, "iill|OO:sendfile", &fd, &s, &offset,
			      &nbytes, &headers, &tailers))
		return NULL;

	res = sendfile(fd, s, offset, nbytes, NULL, &sbytes, 0);
	if (res == -1 && errno != EAGAIN)
		return OSERROR();

	return PyInt_FromLong(sbytes);
}
