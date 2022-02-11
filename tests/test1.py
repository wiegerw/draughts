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
        Scan.set("bb-size", "4")
        Scan.init()

    def test_position(self):
        pos = Pos()
        text = '..................................................W'
        self.assertEqual(text, print_position(pos, False, True))
        self.assertEqual(pos, parse_position(text))
        self.assertFalse(pos.can_move(Side.White))
        self.assertFalse(pos.can_capture(Side.White))
        self.assertFalse(pos.has_king())
        self.assertFalse(pos.is_threat())

        pos = start_position()
        text = 'xxxxxxxxxxxxxxxxxxxx..........ooooooooooooooooooooW'
        self.assertEqual(text, print_position(pos, False, True))
        self.assertEqual(pos, parse_position(text))
        self.assertTrue(pos.can_move(Side.White))
        self.assertFalse(pos.can_capture(Side.White))
        self.assertFalse(pos.has_king())
        self.assertFalse(pos.is_threat())
        moves = generate_moves(pos)
        self.assertEqual(9, len(moves))
        display_position(pos)
        for move in moves:
            print(print_move(move, pos))

        text = '''
           .   .   O   .   . 
         .   .   .   .   .   
           .   x   .   x   . 
         .   .   x   x   .   
           .   .   x   .   . 
         .   .   x   x   .   
           .   x   .   x   . 
         .   .   .   .   .   
           .   .   .   .   . 
         .   .   .   .   .   W;
        '''
        pos = parse_position(text)
        display_position(pos)
        print('eval', eval_position(pos))
        self.assertTrue(pos.can_move(Side.White))
        self.assertTrue(pos.can_capture(Side.White))
        self.assertTrue(pos.has_king())
        self.assertTrue(pos.has_king_side(Side.White))
        self.assertFalse(pos.is_threat())

        text = '''
           .   .   O   .   . 
         .   .   .   .   .   
           .   x   .   x   . 
         .   .   x   x   .   
           .   .   x   .   . 
         .   .   x   x   .   
           .   x   .   x   . 
         .   .   .   .   .   
           .   .   .   .   . 
         .   .   .   .   .   B;
        '''
        pos = parse_position(text)
        display_position(pos)
        print('eval', eval_position(pos))
        self.assertTrue(pos.can_move(Side.White))
        self.assertTrue(pos.can_move(Side.Black))
        self.assertTrue(pos.can_capture(Side.White))
        self.assertFalse(pos.can_capture(Side.Black))
        self.assertTrue(pos.has_king())
        self.assertTrue(pos.has_king_side(Side.White))
        self.assertFalse(pos.has_king_side(Side.Black))
        self.assertTrue(pos.is_threat())

    def test_parse(self):
        text = '..................................................B'
        pos = parse_position(text)
        self.assertEqual(text, print_position(pos, False, True))


    def test_search(self):
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

        si = SearchInput()
        si.move = True
        si.book = False
        si.depth = 15
        si.nodes = 1000000000000
        si.time = 5.0
        si.input = True
        si.output = OutputType.Terminal

        so = SearchOutput()
        node = make_node(pos)
        search(so, node, si)

        # run_terminal_game()


if __name__ == '__main__':
    import unittest
    unittest.main()

