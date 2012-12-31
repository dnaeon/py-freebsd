import unittest
from test import test_support
import sys, os
from freebsd import *
from freebsd.const import *

class Test_kqueue(unittest.TestCase):

    def test_kevent_udata_reference(self):
        dummy = [1, 2, 3, 4]
        ev = kevent(sys.stdout.fileno(), udata=dummy)
        del dummy
        self.__shuffle_freeheaps()
        self.assertEqual(ev.udata, [1, 2, 3, 4])

    def test_kqueue_udata_reference(self):
        kq = kqueue()
        rd, wr = os.pipe()
        try:
            class cntlist(list):
                ninst = 0
                def __init__(self, *args):
                    list.__init__(self, *args)
                    cntlist.ninst += 1
                def __del__(self):
                    cntlist.ninst -= 1

            ev = kevent(rd, udata=cntlist([1, 2, 3, 4]))
            #r = kq.event([ev], 0)
            self.assertRaises(ValueError, kq.event, [ev], 0)
            kq.addevent(ev)
            del ev
            self.__shuffle_freeheaps()

            os.write(wr, 'unittest')
            r = kq.event(None, 1)
            self.assertEqual(os.read(rd, 10), 'unittest')
            self.assertEqual(r[0].udata, [1, 2, 3, 4])
            del r

            self.assertEqual(cntlist.ninst, 1)
            kq.event([kevent(rd, EVFILT_READ, EV_DELETE)], 0)
            self.assertEqual(cntlist.ninst, 0)
        finally:
            os.close(rd)
            os.close(wr)

    def __shuffle_freeheaps(self):
        # drive some memory allocations to shuffle free heaps.
        [[x]*50 for x in range(1000)]


def test_main():
    suite = unittest.TestSuite()
    suite.addTest(unittest.makeSuite(Test_kqueue))
    test_support.run_suite(suite)

if __name__ == "__main__":
    test_main()
