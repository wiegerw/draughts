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
        Scan.set("bb-size", "4")
        Scan.update()
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
        self.assertTrue(pos.is_empty_(1))
        self.assertTrue(pos.is_empty_(2))
        self.assertFalse(pos.is_empty_(3))
        self.assertTrue(pos.is_king(3))
        self.assertFalse(pos.is_king(14))
        self.assertTrue(pos.is_white(3))
        self.assertTrue(pos.is_black(12))
        self.assertFalse(pos.is_king(12))
        self.assertTrue(pos.is_white_to_move())
        self.assertEqual(0, pos.white_man_count())
        self.assertEqual(1, pos.white_king_count())
        self.assertEqual(9, pos.black_man_count())
        self.assertEqual(0, pos.black_king_count())

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

    def test_set_position(self):
        text1 = '''
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
        pos1 = parse_position(text1)
        pos1.put_piece(3, False, True)
        pos1.put_piece(9, False, False)
        pos1.put_piece(43, True, False)
        pos1.put_piece(48, True, True)

        text2 = '''
           .   .   X   .   . 
         .   .   .   x   .   
           .   .   x   x   . 
         x   x   x   x   .   
           x   .   x   x   o 
         x   o   o   .   o   
           .   o   o   .   o 
         .   o   o   o   .   
           .   .   o   .   . 
         .   .   O   .   .   B;
        '''
        pos2 = parse_position(text2)

        self.assertEqual(pos1, pos2)

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

    def test_play_forced_moves(self):
        text1 = '''
           x   .   .   .   .
         .   .   .   .   .
           .   x   x   x   .
         x   .   .   .   .
           x   .   .   x   .
         .   .   .   .   .
           x   .   .   .   .
         o   .   o   o   o
           .   .   .   .   o
         .   .   .   .   .   W;
        '''
        pos1 = parse_position(text1)
        text2 = '''
           .   .   .   .   .
         .   .   .   .   .
           .   x   .   .   .
         x   .   .   .   .
           .   .   .   .   .
         .   .   .   .   .
           .   .   .   .   .
         .   .   .   .   o
           .   .   .   .   .
         .   .   .   .   .   B;
        '''
        expected = parse_position(text2)
        pos = play_forced_moves(pos1)
        self.assertEqual(expected, pos)

    def naive_rollout(self, text, expected_piece_count, expected_rollout):
        pos = parse_position(text)
        self.assertEqual(expected_piece_count, piece_count_eval(play_forced_moves(pos)))
        self.assertEqual(expected_rollout, naive_rollout(pos))

    def test_naive_rollout(self):
        text = '''
           .   .   .   .   .
         .   .   .   x   x
           .   .   .   .   .
         .   .   .   .   .
           .   .   .   .   .
         .   .   .   .   .
           .   o   o   o   .
         .   .   .   .   .
           .   .   .   .   .
         .   .   .   .   o   W;
        '''
        expected_piece_count = 2
        expected_rollout = 1
        self.naive_rollout(text, expected_piece_count, expected_rollout)

        text = '''
           .   X   X   X   .
         .   .   .   x   x
           .   .   .   .   .
         .   .   .   .   .
           .   .   .   .   .
         .   .   .   .   .
           .   o   o   o   .
         .   .   O   O   .
           .   .   .   .   .
         .   .   .   .   o   W;
        '''
        expected_piece_count = -1
        expected_rollout = 0
        self.naive_rollout(text, expected_piece_count, expected_rollout)

        text = '''
           .   .   .   .   .
         x   .   .   .   .
           .   .   x   x   .
         x   .   .   .   .
           x   .   .   x   .
         .   .   .   .   .
           x   .   .   .   .
         o   .   o   o   o
           .   .   .   .   o
         .   .   .   .   .   W;
        '''
        expected_piece_count = 0
        expected_rollout = 0.5
        self.naive_rollout(text, expected_piece_count, expected_rollout)

    def test_pickle(self):
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
        import pickle
        data = pickle.dumps(pos, 5)
        pos1 = pickle.loads(data)
        print(pos1)
        self.assertEqual(pos, pos1)


    def test_flip(self):
        text1 = '''
           x   X   .   .   .
         .   .   .   .   .
           .   .   .   .   .
         .   .   .   .   .
           .   .   .   .   .
         .   .   .   .   .
           .   .   .   .   .
         .   .   .   .   .
           .   .   .   .   .
         o   O   .   .   .   W;
        '''
        pos1 = parse_position(text1)
        text2 = '''
           .   .   .   X   x
         .   .   .   .   .
           .   .   .   .   .
         .   .   .   .   .
           .   .   .   .   .
         .   .   .   .   .
           .   .   .   .   .
         .   .   .   .   .
           .   .   .   .   .
         .   .   .   O   o   B;
        '''
        expected = parse_position(text2)
        pos1.flip()
        self.assertEqual(expected, pos1)


if __name__ == '__main__':
    import unittest
    unittest.main()

