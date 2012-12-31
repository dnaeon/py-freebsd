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

#include <sys/mount.h>

EXPCONST(int MNT_RDONLY)
EXPCONST(int MNT_NOEXEC)
EXPCONST(int MNT_NOSUID)
EXPCONST_IFAVAIL(int MNT_NODEV)
EXPCONST(int MNT_SYNCHRONOUS)
EXPCONST(int MNT_ASYNC)
EXPCONST(int MNT_LOCAL)
EXPCONST(int MNT_QUOTA)
EXPCONST(int MNT_ROOTFS)
EXPCONST(int MNT_EXRDONLY)
EXPCONST(int MNT_EXPORTED)
EXPCONST(int MNT_DEFEXPORTED)
EXPCONST(int MNT_EXPORTANON)
EXPCONST(int MNT_EXKERB)

EXPCONST(int MNT_WAIT)
EXPCONST(int MNT_NOWAIT)
EXPCONST(int MNT_LAZY)


static PyObject *
PyObject_FromStatfs(struct statfs *buf)
{
	PyObject *r, *fsid;

	r = PyDict_New();
	fsid = PyTuple_New(2);

	SETDICT_INT(r, "bsize", buf->f_bsize);
	SETDICT_INT(r, "iosize", buf->f_iosize);
	SETDICT_INT(r, "blocks", buf->f_blocks);
	SETDICT_INT(r, "bfree", buf->f_bfree);
	SETDICT_INT(r, "bavail", buf->f_bavail);
	SETDICT_INT(r, "files", buf->f_files);
	SETDICT_INT(r, "ffree", buf->f_ffree);
	PyTuple_SET_ITEM(fsid, 0, PyInt_FromLong((long)buf->f_fsid.val[0]));
	PyTuple_SET_ITEM(fsid, 1, PyInt_FromLong((long)buf->f_fsid.val[1]));
	PyDict_SetItemString(r, "fsid", fsid);
	Py_DECREF(fsid);
	SETDICT_INT(r, "owner", buf->f_owner);
	SETDICT_INT(r, "type", buf->f_type);
	SETDICT_INT(r, "flags", buf->f_flags);
	SETDICT_INT(r, "syncwrites", buf->f_syncwrites);
	SETDICT_INT(r, "asyncwrites", buf->f_asyncwrites);
	SETDICT_STR(r, "fstypename", buf->f_fstypename);
	SETDICT_STR(r, "mntonname", buf->f_mntonname);
	SETDICT_INT(r, "syncreads", buf->f_syncreads);
	SETDICT_INT(r, "asyncreads", buf->f_asyncreads);
	SETDICT_STR(r, "mntfromname", buf->f_mntfromname);

	return r;
}


static char PyFB_statfs__doc__[] =
"statfs(path):\n"
"returns information about a mounted file system.  The `path` argument\n"
"is the path name of any file within the mounted file system.";

static PyObject *
PyFB_statfs(PyObject *self, PyObject *args)
{
	struct statfs buf;
	char *path;

	if (!PyArg_ParseTuple(args, "s:statfs", &path))
		return NULL;

	if (statfs(path, &buf) == -1)
		return OSERROR();
	return PyObject_FromStatfs(&buf);
}


static char PyFB_fstatfs__doc__[] =
"fstatfs(fd):\n"
"returns the same information about an open file referenced by\n"
"descriptor `fd`.";

static PyObject *
PyFB_fstatfs(PyObject *self, PyObject *args)
{
	struct statfs buf;
	int fd;

	if (!PyArg_ParseTuple(args, "i:fstatfs", &fd))
		return NULL;

	if (fstatfs(fd, &buf) == -1)
		return OSERROR();
	return PyObject_FromStatfs(&buf);
}


static char PyFB_getfsstat__doc__[] =
"getfsstat(flags):\n"
"returns information about all mounted file systems.";


static PyObject *
PyFB_getfsstat(PyObject *self, PyObject *args)
{
	struct statfs *buf;
	int flags, nbufs, i;
	PyObject *r;

	if (!PyArg_ParseTuple(args, "i:getfsstat", &flags))
		return NULL;

	nbufs = getfsstat(NULL, 0, flags);
	if (nbufs == -1)
		return OSERROR();
	else if (nbufs == 0)
		return PyList_New(0);

	buf = PyMem_New(struct statfs, nbufs);

	nbufs = getfsstat(buf, nbufs * sizeof(struct statfs), flags);
	if (nbufs == -1) {
		PyMem_Del(buf);
		return OSERROR();
	}

	r = PyList_New(nbufs);
	for (i = 0; i < nbufs; i++)
		PyList_SET_ITEM(r, i, PyObject_FromStatfs(&buf[i]));
	PyMem_Del(buf);

	return r;
}
