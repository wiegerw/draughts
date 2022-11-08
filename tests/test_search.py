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

    def check_position(self, text, max_depth, expected_moves, expected_piece_count, max_time=5.0):
        pos = parse_position(text)

        score1, m1 = minimax_search_with_shuffle(pos, max_depth)
        score2, m2 = minimax_search_scan(pos, max_depth)
        score3, m3 = scan_search(pos, max_depth, max_time)
        if not pos.is_white_to_move() and score3 != score_none():
            score3 = -score3
        move1 = print_move(m1, pos)
        move2 = print_move(m2, pos)
        move3 = print_move(m3, pos)
        moves = [move1, move2, move3]
        scores = [score1, score2, score3] if score3 != score_none() else [score1, score2]

        # display the search results
        print('==========================================')
        display_position(pos)
        print(f'moves: {moves}')
        print(f'scores: {scores}')

        # check the moves
        self.assertTrue(move1 in expected_moves)
        self.assertTrue(move2 in expected_moves)
        self.assertTrue(move3 in expected_moves)

        # check the scores
        self.assertEqual(expected_piece_count, score1)
        self.assertTrue(all(s >= 0 for s in scores) or all(s <= 0 for s in scores))


    def test_search(self):
        text = '''
           .   .   .   .   . 
         .   .   .   x   .   
           .   .   .   .   . 
         .   .   .   .   .   
           .   .   .   .   . 
         .   .   .   .   .   
           .   .   x   .   . 
         .   .   .   .   .   
           .   .   .   .   . 
         O   .   .   O   O   W
        '''
        max_depth = 15
        expected_moves = ['50x4']
        expected_piece_count = 9
        self.check_position(text, max_depth, expected_moves, expected_piece_count)

        text = '''
           .   .   .   .   . 
         .   .   .   o   .   
           .   .   .   .   . 
         .   .   .   .   .   
           .   .   .   .   . 
         .   .   .   .   .   
           .   .   o   .   . 
         .   .   .   .   .   
           .   .   .   .   . 
         X   .   .   X   X   B
        '''
        max_depth = 15
        expected_moves = ['50x4']
        expected_piece_count = -9
        self.check_position(text, max_depth, expected_moves, expected_piece_count)

        text = '''
           .   .   .   .   .
         .   .   .   o   .
           .   .   .   .   .
         .   .   .   .   .
           .   .   .   .   .
         .   .   .   .   .
           .   .   o   .   .
         .   .   .   .   .
           .   .   .   .   .
         X   .   .   .   X   B
        '''
        max_depth = 15
        expected_moves = ['50x4']
        expected_piece_count = -6
        self.check_position(text, max_depth, expected_moves, expected_piece_count)

        text = '''
           .   .   .   O   .
         .   .   .   .   .
           .   .   .   .   .
         .   .   .   .   .
           .   .   .   .   .
         .   .   .   .   .
           .   .   o   .   .
         .   .   .   .   .
           .   .   .   .   .
         X   .   .   .   X   B
        '''
        max_depth = 3
        expected_moves = ['50x28', '50x17', '50x11', '50x6']
        expected_piece_count = -3
        self.check_position(text, max_depth, expected_moves, expected_piece_count)

        text = '''
           .   .   .   .   . 
         .   .   .   .   .   
           .   .   .   .   . 
         .   .   .   x   .   
           .   .   .   .   . 
         x   .   x   .   .   
           .   .   .   .   . 
         .   o   .   .   .   
           o   .   .   .   . 
         o   .   .   .   .   W
        '''
        max_depth = 1
        expected_moves = ['37-31']
        expected_piece_count = 2
        self.check_position(text, max_depth, expected_moves, expected_piece_count)

        text = '''
           .   x   .   .   . 
         .   .   x   .   x   
           .   .   .   .   . 
         .   x   .   o   .   
           .   .   .   .   . 
         x   .   o   .   .   
           .   .   .   .   . 
         .   o   .   .   .   
           o   .   .   .   . 
         o   .   .   .   .   W
        '''
        max_depth = 3
        expected_moves = ['19-13', '28-22', '19-14']
        expected_piece_count = 3
        self.check_position(text, max_depth, expected_moves, expected_piece_count)

        text = '''
           .   .   .   .   x 
         .   .   .   .   x   
           .   .   .   x   . 
         .   .   .   .   .   
           .   .   x   .   o 
         .   .   .   .   .   
           .   x   .   o   . 
         .   .   .   .   .   
           o   .   o   .   . 
         .   .   .   o   .   B
        '''
        max_depth = 3
        expected_moves = ['32-38', '23-29', '32-37']
        expected_piece_count = -3
        self.check_position(text, max_depth, expected_moves, expected_piece_count)


if __name__ == '__main__':
    import unittest
    unittest.main()

