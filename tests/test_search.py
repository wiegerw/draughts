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

    def test_scan_search(self):
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
        pos = parse_position(text)
        display_position(pos)
        print('eval', eval_position(pos))
        max_depth = 15
        max_time = 5.0
        score, move = scan_search(pos, max_depth, max_time)
        print(f'score = {score}, move = {print_move(move, pos)}')

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
        pos = parse_position(text)
        display_position(pos)
        print('eval', eval_position(pos))
        max_depth = 15
        max_time = 5.0
        score, move = scan_search(pos, max_depth, max_time)
        print(f'score = {score}, move = {print_move(move, pos)}')

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
        pos = parse_position(text)
        display_position(pos)
        print('eval', eval_position(pos))
        max_depth = 15
        max_time = 5.0
        score, move = scan_search(pos, max_depth, max_time)
        print(f'score = {score}, move = {print_move(move, pos)}')

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
        pos = parse_position(text)
        display_position(pos)
        print('eval', eval_position(pos))
        max_depth = 15
        max_time = 5.0
        score, move = scan_search(pos, max_depth, max_time)
        print(f'score = {score}, move = {print_move(move, pos)}')

    def test_minimax_search(self):
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
        pos = parse_position(text)
        display_position(pos)
        max_depth = 1
        score, move = minimax_search(pos, max_depth)
        print(f'score = {score}, move = {print_move(move, pos)}')
        self.assertEqual(2, score)
        self.assertEqual('37-31', print_move(move, pos))

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
        pos = parse_position(text)
        display_position(pos)
        max_depth = 3
        score, move = minimax_search(pos, max_depth)
        print(f'score = {score}, move = {print_move(move, pos)}')
        self.assertEqual(3, score)
        self.assertTrue(print_move(move, pos) in ['19-13', '28-22'])

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
        pos = parse_position(text)
        display_position(pos)
        max_depth = 3
        score, move = minimax_search(pos, max_depth)
        print(f'score = {score}, move = {print_move(move, pos)}')
        self.assertEqual(3, score)
        self.assertTrue(print_move(move, pos) in ['32-38', '23-29'])


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
        pos = parse_position(text)
        display_position(pos)
        max_depth = 3
        score, move = minimax_search_with_shuffle(pos, max_depth)
        print(f'score = {score}, move = {print_move(move, pos)}')
        self.assertEqual(3, score)
        self.assertTrue(print_move(move, pos) in ['32-38', '23-29'])


if __name__ == '__main__':
    import unittest
    unittest.main()

