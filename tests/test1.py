#  (C) Copyright Wieger Wesselink 2022. Distributed under the GPL-3.0
#  Software License, (See accompanying file license.txt or copy at
#  https://www.gnu.org/licenses/gpl-3.0.txt)

import unittest
from draughts1 import *


class Test(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        bit_init()
        hash_init()
        pos_init()
        eval_init()
        ScanSettings.init()  # var::init()
        EGDBIndex.init()     # bb::index_init()
        comp_init()
        rand_init()
        ScanSettings.load("scan.ini")

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
        display_position(pos)
        for move in moves:
            print(print_move(move, pos))

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

        print('hash', hash_key(pos), hash(pos))
        # N.B. The values are different, because the function hash truncates the return value to a size of Py_ssize_t.


        init_high()  # TODO: improve this interface

        node = make_node(pos)

        si = SearchInput()
        si.move = True
        si.book = False
        si.depth = 10
        si.nodes = 1000000000000
        si.time = 5.0
        si.input = True
        si.output = OutputType.Terminal

        so = SearchOutput()
        search(so, node, si)

        # run_terminal_game()


if __name__ == '__main__':
    import unittest
    unittest.main()

