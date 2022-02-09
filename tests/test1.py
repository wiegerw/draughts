# Copyright 2021 Wieger Wesselink.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or http://www.boost.org/LICENSE_1_0.txt)

import unittest
from draughts1 import *


class Test(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        bit_init()
        pos_init()
        eval_init()

    def test_position(self):
        pos = Pos()
        text = '..................................................W'
        self.assertEqual(text, print_position(pos, False, True))
        self.assertEqual(pos, parse_position(text))

        pos = start_position()
        text = 'xxxxxxxxxxxxxxxxxxxx..........ooooooooooooooooooooW'
        self.assertEqual(text, print_position(pos, False, True))
        self.assertEqual(pos, parse_position(text))
        moves = generate_moves(pos)
        self.assertEqual(9, len(moves))

        for move in moves:
            print(print_move(move, pos))

        display_position(pos)

        text = '''
           .   .   .   .   . 
         .   .   .   .   .   
           .   .   x   x   . 
         x   x   x   x   .   
           x   .   x   x   o 
         x   o   o   .   o   
           .   o   o   .   o 
         .   o   o   o   .   
           .   .   .   .   . 
         .   .   .   .   .   B;
        '''
        pos = parse_position(text)
        display_position(pos)
        print('eval', eval_position(pos))


if __name__ == '__main__':
    import unittest
    unittest.main()

