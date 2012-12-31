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

#include <ufs/ufs/quota.h>

EXPCONST(int USRQUOTA)
EXPCONST(int GRPQUOTA)


static char PyFB_quotaon__doc__[] =
"quotaon(path, which, quotafile):\n"
"turns filesystem's quota support on";

static PyObject *
PyFB_quotaon(PyObject *self, PyObject *args)
{
	int which;
	char *path, *quotafile;

	if (!PyArg_ParseTuple(args, "sis:quotaon", &path, &which, &quotafile))
		return NULL;

	if (quotactl(path, QCMD(Q_QUOTAON, which), 0, (void*)quotafile) == -1)
		return OSERROR();
	Py_RETURN_NONE;
}


static char PyFB_quotaoff__doc__[] =
"quotaoff(path, which):\n"
"turns filesystem's quota support off";

static PyObject *
PyFB_quotaoff(PyObject *self, PyObject *args)
{
	int which;
	char *path;

	if (!PyArg_ParseTuple(args, "si:quotaoff", &path, &which))
		return NULL;

	if (quotactl(path, QCMD(Q_QUOTAOFF, which), 0, 0) == -1)
		return OSERROR();
	Py_RETURN_NONE;
}


static char PyFB_getquota__doc__[] =
"getquota(path, which, uid):\n"
"get filesystem quotas; `which` must be const.GRPQUOTA or\n"
"const.USRQUOTA";

static PyObject *
PyFB_getquota(PyObject *self, PyObject *args)
{
	struct dqblk qp;
	int uid, which;
	char *path;
	PyObject *r;

	if (!PyArg_ParseTuple(args,"sii:getquota", &path, &which, &uid))
		return NULL;

	if (quotactl(path, QCMD(Q_GETQUOTA, which), uid, (void*)&qp) == -1)
		return OSERROR();

	r = PyDict_New();

	SETDICT_LONG(r, "bhardlimit", qp.dqb_bhardlimit);
	SETDICT_LONG(r, "bsoftlimit", qp.dqb_bsoftlimit);
	SETDICT_LONG(r, "curblocks", qp.dqb_curblocks);
	SETDICT_LONG(r, "ihardlimit", qp.dqb_ihardlimit);
	SETDICT_LONG(r, "isoftlimit", qp.dqb_isoftlimit);
	SETDICT_LONG(r, "curinodes", qp.dqb_curinodes);
	SETDICT_INT(r, "btime", qp.dqb_btime);
	SETDICT_INT(r, "itime", qp.dqb_itime);

	return r;
}


static char PyFB_setquota__doc__[] =
"setquota(path, which, uid[, {name}={value}]):\n"
"set filesystem quotas; `which` must be GRPQUOTA or USRQUOTA.\n"
"Available names are bhardlimit, bsoftlimit, ihardlimit, isoftlimit,\n"
"btime, itime.";

static PyObject *
PyFB_setquota(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *kwlist[] = {
		"path", "which", "uid", "bhardlimit", "bsoftlimit",
		"ihardlimit", "isoftlimit", "btime", "itime", NULL
	};
	struct dqblk qp;
	long long bhardlimit = -1, bsoftlimit = -1, ihardlimit = -1;
	long long isoftlimit = -1, btime = -1, itime = -1;
	int uid, which;
	char *path;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs,
			"sii|LLLLii:setquota", kwlist, &path, &which, &uid,
			&bhardlimit, &bsoftlimit, &ihardlimit, &isoftlimit,
			&btime, &itime))
		return NULL;

	if (bhardlimit < 0 or bsoftlimit < 0 or ihardlimit < 0 or
	    isoftlimit < 0 or btime < 0 or itime < 0) {
		if (quotactl(path, QCMD(Q_GETQUOTA, which), uid,
				(void*)&qp) == -1)
			return OSERROR();
		if (bhardlimit >= 0) qp.dqb_bhardlimit = bhardlimit;
		if (bsoftlimit >= 0) qp.dqb_bsoftlimit = bsoftlimit;
		if (ihardlimit >= 0) qp.dqb_ihardlimit = ihardlimit;
		if (isoftlimit >= 0) qp.dqb_isoftlimit = isoftlimit;
		if (btime >= 0) qp.dqb_btime = btime;
		if (itime >= 0) qp.dqb_itime = itime;
	}
	else {
		qp.dqb_bhardlimit = bhardlimit;
		qp.dqb_bsoftlimit = bsoftlimit;
		qp.dqb_ihardlimit = ihardlimit;
		qp.dqb_isoftlimit = isoftlimit;
		qp.dqb_btime = btime;
		qp.dqb_itime= itime;
	}

	if (quotactl(path, QCMD(Q_SETQUOTA, which), uid, (void*)&qp) == -1)
		return OSERROR();

	Py_RETURN_NONE;
}


static char PyFB_quotasync__doc__[] =
"quotasync(path, which):\n"
"sync filesystem quota";

static PyObject *
PyFB_quotasync(PyObject *self, PyObject *args)
{
	int which;
	char *path;

	if (!PyArg_ParseTuple(args, "si:quotasync", &path, &which))
		return NULL;

	if (quotactl(path, QCMD(Q_SYNC, which), 0, 0) == -1)
		return OSERROR();

	Py_RETURN_NONE;
}
