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

#include <sys/sysctl.h>
#include <net/if.h>
#include <net/if_mib.h>
#include <netinet/ip_var.h>
#include <netinet/tcp.h>
#include <netinet/tcp_var.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>

EXPCONST(int IFF_UP)
EXPCONST(int IFF_BROADCAST)
EXPCONST(int IFF_DEBUG)
EXPCONST(int IFF_LOOPBACK)
EXPCONST(int IFF_POINTOPOINT)
EXPCONST(int IFF_SMART)
EXPCONST(int IFF_RUNNING)
EXPCONST(int IFF_NOARP)
EXPCONST(int IFF_PROMISC)
EXPCONST(int IFF_ALLMULTI)
EXPCONST(int IFF_OACTIVE)
EXPCONST(int IFF_SIMPLEX)
EXPCONST(int IFF_LINK0)
EXPCONST(int IFF_LINK1)
EXPCONST(int IFF_LINK2)
EXPCONST(int IFF_ALTPHYS)
EXPCONST(int IFF_MULTICAST)

static char PyFB_ipstats__doc__[] =
"ipstats():\n"
"dumps IP statistics structure";

static PyObject *
PyFB_ipstats(PyObject *self)
{
	struct ipstat ipstat;
	size_t len = sizeof ipstat; 
	PyObject *r, *t;

	if (sysctlbyname("net.inet.ip.stats", &ipstat, &len, NULL, 0) < 0)
		return OSERROR();

	r = PyDict_New();

#define p(f) \
	t = PyLong_FromUnsignedLong((unsigned long)ipstat.ips_##f); \
	PyDict_SetItemString(r, #f, t); \
	Py_DECREF(t);
	p(total);           p(badsum);
	p(toosmall);        p(tooshort);
	p(toolong);         p(badhlen);
	p(badlen);          p(badoptions);
	p(badvers);         p(fragments);
	p(fragdropped);     p(fragtimeout);
	p(reassembled);     p(delivered);
	p(noproto);         p(forward);
	p(forward);         p(fastforward);
	p(cantforward);     p(notmember);
	p(redirectsent);    p(localout);
	p(rawout);          p(odropped);
	p(noroute);         p(fragmented);
	p(ofragments);      p(cantfrag);
	p(nogif);           p(badaddr);
#undef p

	return r;
}


static char PyFB_tcpstats__doc__[] =
"tcpstats():\n"
"dumps TCP statistics structure";

static PyObject *
PyFB_tcpstats(PyObject *self)
{
	struct tcpstat tcpstat;
	size_t len = sizeof tcpstat; 
	PyObject *r, *t;

	if (sysctlbyname("net.inet.tcp.stats", &tcpstat, &len, NULL, 0) < 0)
		return OSERROR();

	r = PyDict_New();

#define p(f) \
	t = PyLong_FromUnsignedLong((unsigned long)tcpstat.tcps_##f); \
	PyDict_SetItemString(r, #f, t); \
	Py_DECREF(t);

	p(sndtotal);        p(sndpack);         p(sndbyte);
	p(sndrexmitpack);   p(sndrexmitbyte);   p(mturesent);
	p(sndacks);         p(delack);          p(sndurg);
	p(sndprobe);        p(sndwinup);        p(sndctrl);
	p(rcvtotal);        p(rcvackpack);      p(rcvackbyte);
	p(rcvdupack);       p(rcvacktoomuch);   p(rcvpack);
	p(rcvbyte);         p(rcvduppack);      p(rcvdupbyte);
	p(pawsdrop);        p(rcvpartduppack);  p(rcvpartdupbyte);
	p(rcvoopack);       p(rcvoobyte);       p(rcvpackafterwin);
	p(rcvbyteafterwin); p(rcvwinprobe);     p(rcvwinupd);
	p(rcvafterclose);   p(rcvbadsum);       p(rcvbadoff);
	p(rcvshort);        p(connattempt);     p(accepts);
	p(badsyn);          p(listendrop);      p(connects);
	p(closed);          p(drops);           p(cachedrtt);
	p(cachedrttvar);    p(cachedssthresh);  p(conndrops);
	p(rttupdated);      p(segstimed);       p(rexmttimeo);
	p(timeoutdrop);     p(persisttimeo);    p(persistdrop);
	p(keeptimeo);       p(keepprobe);       p(keepdrops);
	p(predack);         p(preddat);         p(sc_added);
	p(sc_retransmitted); p(sc_dupsyn);      p(sc_dropped);
	p(sc_completed);    p(sc_bucketoverflow);
	p(sc_cacheoverflow); p(sc_reset);       p(sc_stale);
	p(sc_aborted);      p(sc_badack);       p(sc_unreach);
	p(sc_zonefail);     p(sc_sendcookie);   p(sc_recvcookie);
#undef p

	return r;
}


static char PyFB_udpstats__doc__[] =
"udpstats():\n"
"dumps UDP statistics structure";

static PyObject *
PyFB_udpstats(PyObject *self)
{
	struct udpstat udpstat;
	size_t len = sizeof udpstat; 
	PyObject *r, *t;

	if (sysctlbyname("net.inet.udp.stats", &udpstat, &len, NULL, 0) < 0)
		return OSERROR();

	r = PyDict_New();

#define p(f) \
	t = PyLong_FromUnsignedLong((unsigned long)udpstat.udps_##f); \
	PyDict_SetItemString(r, #f, t); \
	Py_DECREF(t);

	p(ipackets);       p(hdrops);         p(badlen);
	p(badsum);         p(nosum);          p(noport);
	p(noportbcast);    p(fullsock);       

	t = PyLong_FromUnsignedLong((unsigned long)udpstat.udpps_pcbhashmiss);
	PyDict_SetItemString(r, "pcbhashmiss", t);
	Py_DECREF(t);

	t = PyLong_FromUnsignedLong((unsigned long)(
				udpstat.udps_ipackets -
				udpstat.udps_hdrops -
				udpstat.udps_badlen -
				udpstat.udps_badsum -
				udpstat.udps_noport -
				udpstat.udps_noportbcast -
				udpstat.udps_fullsock
				));
	PyDict_SetItemString(r, "delivered", t);
	Py_DECREF(t);
	p(opackets);
#undef p

	return r;
}


static char PyFB_ifstats__doc__[] =
"ifstats():\n"
"dump network device statistics structure";

static PyObject *
PyFB_ifstats(PyObject *self)
{
	int mib_ifdata[6] = { CTL_NET, PF_LINK, NETLINK_GENERIC,
			      IFMIB_IFDATA, 0, IFDATA_GENERAL};
	struct ifmibdata ifmd;
	size_t len; 
	int value, i;
	PyObject *r, *d, *t;

	len = sizeof value;
	if (sysctlbyname("net.link.generic.system.ifcount", &value,
			 &len, NULL, 0) < 0)
		return OSERROR();

	r = PyDict_New();

	for (i = 1; i <= value; i++) {
		len = sizeof ifmd;
		mib_ifdata[4] = i;
		if (sysctl(mib_ifdata, 6, &ifmd, &len, NULL, 0) < 0) {
			Py_DECREF(r);
			PyErr_SetFromErrno(PyExc_OSError);
			return Py_None;
		}

		d = PyDict_New();

#define p_long(f)   p(PyLong_FromUnsignedLong, unsigned long, f)
#define p_str(f)    p(PyString_FromString, char *, f)
#define p_int(f)    p(PyInt_FromLong, long, f)

#define p(adoptor, castor, f) \
		t = adoptor((castor)ifmd.ifmd_##f); \
		PyDict_SetItemString(d, #f, t); \
		Py_DECREF(t);
		p_str(name);        p_int(pcount);
		p_int(flags);       p_int(snd_len);
		p_int(snd_maxlen);  p_int(snd_drops);
#undef p

#define p(adoptor, castor, f) \
		t = adoptor((castor)ifmd.ifmd_data.ifi_##f); \
		PyDict_SetItemString(d, #f, t); \
		Py_DECREF(t);
		p_int(type);        p_int(physical);
		p_int(addrlen);     p_int(hdrlen);
		p_int(recvquota);   p_int(xmitquota);
		p_long(mtu);        p_long(metric);
		p_long(baudrate);   p_long(ipackets);
		p_long(ierrors);    p_long(opackets);
		p_long(oerrors);    p_long(collisions);
		p_long(ibytes);     p_long(obytes);
		p_long(imcasts);    p_long(omcasts);
		p_long(iqdrops);    p_long(noproto);
		p_long(hwassist);
#undef p

		PyDict_SetItemString(r, ifmd.ifmd_name, d);
		Py_DECREF(d);
	}

	return r;
}
