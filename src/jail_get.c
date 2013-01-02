/*-
 * Copyright (c) 2012 Marin Atanasov Nikolov <dnaeon@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in this position and unchanged.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

static char PyFB_jail_get__doc__[] =
"jail_get(jid):\n"
"The jail_get() system call retrieves jail parameters."
"The jail to read can be specified by using either the jail's jid or name."
"The returned parameters about the jails are the the jail's jid, name, path, "
"hostname and IPv4 address."
"In order to list the parameters of all jails use 0 (zero) as the jid.";

static PyObject *
PyFB_jail_get(PyObject *self, PyObject *args)
{
	PyObject *r;
        struct iovec fbsd_jparams[10];
	struct in_addr ip4;
	char path[MAXPATHLEN], hostname[MAXHOSTNAMELEN], name[MAXHOSTNAMELEN];
	int jid;

        if (!PyArg_ParseTuple(args, "i:jail_get", &jid))
		return NULL;

	assert (jid >= 0);

	*(const void **) &jparams[0].iov_base = "lastjid";
	jparams[0].iov_len = sizeof("lastjid");
	jparams[1].iov_base = &jid;
	jparams[1].iov_len = sizeof(jid);

	*(const void **) &jparams[2].iov_base = "path";
	jparams[2].iov_len = sizeof("path");
	jparams[3].iov_base = &path;
	jparams[3].iov_len = MAXPATHLEN;

	*(const void **) &jparams[4].iov_base = "host.hostname";
	jparams[4].iov_len = sizeof("host.hostname");
	jparams[5].iov_base = &hostname;
	jparams[5].iov_len = MAXHOSTNAMELEN;

	*(const void **) &jparams[6].iov_base = "name";
	jparams[6].iov_len = sizeof("name");
	jparams[7].iov_base = &name;
	jparams[7].iov_len = MAXHOSTNAMELEN;

	*(const void **) &jparams[8].iov_base = "ip4.addr";
	jparams[8].iov_len = sizeof("ip4.addr");
	jparams[9].iov_base = &ip4;
	jparams[9].iov_len = sizeof(struct in_addr);

	r = PyDict_New();
	
        while ((jid = jail_get(jparams, 10, 0)) > 0) {
		SETDICT_INT(r, "jid", jid);
		SETDICT_STR(r, "name", name);
		SETDICT_STR(r, "ip4", inet_ntoa(ip4));
		SETDICT_STR(r, "path", path);
		SETDICT_STR(r, "hostname", hostname);
	}

	return (r);
}

