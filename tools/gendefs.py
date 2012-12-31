#!/usr/bin/env python

import os, glob
import time, re

__verbose__ = False
method_type = {
    1: 'METH_NOARGS',
    2: 'METH_VARARGS',
    3: 'METH_VARARGS|METH_KEYWORDS',
}
pat_const = re.compile(
            '^EXPCONST([A-Z_]*)\s*\(([a-z_0-9 ]+( \*)?)\s*([^\) ]*)\)\s*$')
pat_func = re.compile('^PyFB_([A-Za-z0-9_]+)\s*\(([^\)]*)\)')
pat_type = re.compile('^DECLTYPE\s*\(([A-Za-z_0-9 ]+)\s*,\s*([A-Za-z_0-9 ]+)\)\s*$')
pat_libdep = re.compile('LIB_DEPENDS\s*\(\s*([A-Za-z0-9_]+)\s*\)')
pat_cppif = re.compile('#\s*if')
pat_cppif_vers = re.compile(
            '#\s*if\s+(__FreeBSD_version\s*[<>=]{1,2}\s*[0-9]+)')
pat_cppendif = re.compile('#\s*endif')

try:
    sorted
except NameError:
    def sorted(o):
        o = o[:]
	o.sort()
	return o

def printout_const(f, match):
    defcond, consttype, ignore, name = match
    consttype = consttype.strip()

    if not defcond:
        print >> f, 'SETDICT_%s(d, "%s", %s);' % (
            consttype.upper().replace('UNSIGNED ', 'U'), name,
            name)
    elif defcond == '_IFAVAIL':
        print >> f, '#ifdef %s' % name
        print >> f, 'SETDICT_%s(d, "%s", %s);' % (
            consttype.upper().replace('UNSIGNED ', 'U'), name,
            name)
        print >> f, '#endif'
    else:
        raise NotImplementedError, "%s is not supported condition yet" % (
                defcond)

    if __verbose__:
        print "  Constant: %s %s" % (consttype, name)

def printout_func(f, match):
    name, args = match
    nargs = len(args.split(','))

    print >> f, '{"%s", (PyCFunction)PyFB_%s, %s,' % (
                    name, name, method_type[nargs])
    print >> f, ' PyFB_%s__doc__},' % (name)

    if __verbose__:
        print "  Function: %s %s" % (name, method_type[nargs])

def printout_type(f, match):
    print >> f, 'INITTYPE(%s, %s)' % match
    if __verbose__:
        print "  Type: %s %s" % match

def printout_lib(f, match):
    print >> f, '%s' % match
    if __verbose__:
        print "  Library: %s" % match

def main():
    constdef = open('src/.const.def', 'w')
    methoddef = open('src/.methods.def', 'w')
    sourcedef = open('src/.sources.def', 'w')
    typedef = open('src/.types.def', 'w')
    libdef = open('src/.libraries.def', 'w')

    for f in (constdef, methoddef, sourcedef, typedef):
        print >> f, "/* Generated on %s */" % time.asctime()

    for src in sorted(glob.glob('src/*.c')):
        filename = os.path.basename(src)
        if filename == 'freebsdmodule.c':
            continue

        print "scanning %s" % src

        print >> sourcedef, '#include "%s"' % filename

        ifblocks = []
        for line in open(src):
            if pat_cppif.search(line):
                found = pat_cppif_vers.findall(line)
                ifblocks.append(found and found[0])
            elif pat_cppendif.search(line):
                ifblocks.pop()

            react = None

            found = pat_const.findall(line)
            if found:
                react = printout_const, constdef, found[0]

            found = pat_func.findall(line)
            if found:
                react = printout_func, methoddef, found[0]

            found = pat_type.findall(line)
            if found:
                react = printout_type, typedef, found[0]

            found = pat_libdep.findall(line)
            if found:
                react = printout_lib, libdef, found[0]

            if react is not None:
                cond = ' && '.join(filter(None, ifblocks))
                outf = react[1]
                if cond:
                    print >> outf, '#if', cond
                react[0](react[1], react[2])
                if cond:
                    print >> outf, '#endif'


if __name__ == '__main__':
    __verbose__ = True
    main()
