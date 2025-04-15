#!/usr/bin/env python3

#  (C) Copyright Wieger Wesselink 2022. Distributed under the GPL-3.0
#  Software License, (See accompanying file license.txt or copy at
#  https://www.gnu.org/licenses/gpl-3.0.txt)

import unittest
from draughts1 import *


class Test(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        Scan.set("variant", "normal")
        Scan.set("book", "false")
        Scan.set("book-ply", "4")
        Scan.set("book-margin", "4")
        Scan.set("ponder", "false")
        Scan.set("threads", "1")
        Scan.set("tt-size", "24")
        Scan.set("bb-size", "6")
        Scan.update()
        Scan.init()

    def test_egdb(self):
        text = '''
           .   .   .   .   X
         .   .   .   .   .
           .   .   .   .   .
         .   .   .   .   .
           .   .   .   .   .
         .   .   .   o   .
           .   .   .   .   .
         x   .   .   .   .
           .   .   .   .   O
         .   o   .   .   O   W;
        '''
        pos = parse_position(text)
        display_position(pos)
        value = EGDB.probe_raw(pos)  # N.B. probe_raw can not be called in capture positions
        self.assertEqual(EGDBValue.Win, value)

        text = '''
           .   .   .   .   .
         .   .   .   .   X
           .   .   .   .   .
         .   .   .   .   .
           .   .   .   .   .
         .   .   .   o   .
           .   .   .   .   .
         x   .   .   .   .
           .   o   .   .   O
         .   .   .   .   O   B;
        '''
        pos = parse_position(text)
        display_position(pos)
        value = EGDB.probe_raw(pos)
        self.assertEqual(EGDBValue.Loss, value)

        text = '''
           .   .   .   .   .
         .   .   .   .   .
           .   .   .   X   .
         .   .   .   .   .
           .   .   .   .   .
         .   .   .   o   .
           .   .   .   .   .
         x   .   .   .   .
           .   o   .   .   O
         .   .   .   .   O   B;
        '''
        pos = parse_position(text)
        display_position(pos)
        value = EGDB.probe_raw(pos)
        self.assertEqual(EGDBValue.Draw, value)

        text = '''
           .   .   .   .   .
         o   .   .   .   .
           .   .   .   .   .
         .   .   .   o   .
           .   .   .   .   .
         .   .   .   .   X
           .   .   .   .   x
         .   .   .   .   .
           .   .   .   .   o
         .   .   .   O   .   B;
        '''
        pos = parse_position(text)
        display_position(pos)
        value = EGDB.probe(pos)
        self.assertEqual(EGDBValue.Loss, value)


    def test_egdb_enumerator(self):
        enumerator = EGDBEnumerator(1, 1, 1, 0)
        count = 0
        while enumerator.next():
            count += 1
        self.assertEqual(73876, count)

        enumerator = EGDBEnumerator(1, 2, 0, 0)
        count = 0
        while enumerator.next():
            count += 1
        self.assertEqual(37863, count)

        positions = []
        enumerator = EGDBEnumerator(1, 1, 0, 0)
        while enumerator.next():
            positions.append(enumerator.position())

        pos1 = parse_position('''
               x   .   .   .   .
             o   .   .   .   .
               .   .   .   .   .
             .   .   .   .   .
               .   .   .   .   .
             .   .   .   .   .
               .   .   .   .   .
             .   .   .   .   .
               .   .   .   .   .
             .   .   .   .   .   W;
            ''')

        pos2 = parse_position('''
               .   .   .   .   .
             .   .   .   .   .
               .   .   .   .   .
             .   .   .   .   .
               .   .   .   .   .
             .   .   .   .   .
               .   .   .   .   .
             .   .   .   .   .
               .   .   .   .   x
             .   .   .   .   o   W;
            ''')

        self.assertEqual(pos1, positions[0])
        self.assertEqual(pos2, positions[-1])


if __name__ == '__main__':
    import unittest
    unittest.main()

