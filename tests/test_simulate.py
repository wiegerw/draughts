#!/usr/bin/env python3

#  (C) Copyright Wieger Wesselink 2022. Distributed under the GPL-3.0
#  Software License, (See accompanying file license.txt or copy at
#  https://www.gnu.org/licenses/gpl-3.0.txt)

import unittest
from draughts1 import *


def normalize_piece_count(pos: Pos, score: int) -> float:
    if score == 0:
        return 0.5
    return 0 if score < 0 else 1


def normalize_scan(pos: Pos, score: int) -> float:
    if abs(score) < 50:
        return 0.5
    if not pos.is_white_to_move():
        score = -score
    return 0 if score < 0 else 1


# Base class for simulation (rollout).
# The value is normalized to the interval [0,1], and it is from the perspective of the white player.
class Simulate(object):
    def __call__(self, pos: Pos) -> float:
        raise NotImplementedError


# Piece count evaluation
class SimulatePieceCountEval(Simulate):
    def __call__(self, pos: Pos) -> float:
        score = piece_count_eval(play_forced_moves(pos))
        return normalize_piece_count(pos, score)


# Scan evaluation
class SimulateScanEval(Simulate):
    def __call__(self, pos: Pos) -> float:
        score = eval_position(pos)
        return normalize_scan(pos, score)


# Minimax with shuffle
class SimulateMinimaxWithShuffle(Simulate):
    def __init__(self, max_depth: int):
        self.max_depth = max_depth

    def __call__(self, pos: Pos) -> float:
        score, move = minimax_search_with_shuffle(pos, self.max_depth)
        return normalize_piece_count(pos, score)


# Minimax with Scan evaluation
class SimulateMinimaxScan(Simulate):
    def __init__(self, max_depth: int):
        self.max_depth = max_depth

    def __call__(self, pos: Pos) -> float:
        score, move = minimax_search_scan(pos, self.max_depth)
        return normalize_scan(pos, score)


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

    def check_eval(self, text, expected):
        simulate_piece_count_eval = SimulatePieceCountEval()
        simulate_scan_eval = SimulateScanEval()
        simulate_minimax_with_shuffle_5 = SimulateMinimaxWithShuffle(5)
        simulate_minimax_scan_5 = SimulateMinimaxScan(5)

        pos = parse_position(text)
        display_position(pos)
        print('scan eval', eval_position(pos))
        print('simulate_piece_count_eval', simulate_piece_count_eval(pos))
        print('simulate_scan_eval', simulate_scan_eval(pos))
        print('simulate_minimax_with_shuffle_5', simulate_minimax_with_shuffle_5(pos))
        print('simulate_minimax_scan_5', simulate_minimax_scan_5(pos))
        self.assertEqual(expected, simulate_piece_count_eval(pos))
        self.assertEqual(expected, simulate_scan_eval(pos))
        self.assertEqual(expected, simulate_minimax_with_shuffle_5(pos))
        # self.assertEqual(expected, simulate_minimax_scan_5(pos))
        print('')


    def test_eval(self):
        text = '''
           .   .   .   .   .
         .   .   x   x   .
           .   .   .   .   .
         .   .   .   .   .
           .   .   .   .   .
         .   .   .   .   .
           .   .   .   .   .
         .   .   .   .   .
           .   o   o   .   .
         .   .   .   .   .   W
        '''
        expected = 0.5
        self.check_eval(text, expected)
    
        text = '''
           .   .   .   .   .
         .   .   x   x   .
           .   .   .   .   .
         .   .   .   .   .
           .   .   .   .   .
         .   .   .   .   .
           .   .   .   .   .
         .   .   .   .   .
           .   o   o   .   .
         .   .   .   .   .   B
        '''
        expected = 0.5
        self.check_eval(text, expected)
    
        text = '''
           .   .   .   .   .
         .   .   .   .   .
           .   x   x   x   .
         .   .   x   x   .
           .   .   .   .   .
         .   .   .   .   .
           .   o   o   .   .
         .   o   o   o   .
           .   .   .   .   .
         .   .   .   .   .   W
        '''
        expected = 0.5
        self.check_eval(text, expected)
    
        text = '''
           .   .   .   .   .
         .   .   .   .   .
           .   x   x   x   .
         .   .   x   x   .
           .   .   .   .   .
         .   .   .   .   .
           .   o   o   .   .
         .   o   o   o   .
           .   .   .   .   .
         .   .   .   .   .   B
        '''
        expected = 0.5
        self.check_eval(text, expected)
    
        text = '''
           .   .   .   .   .
         .   .   .   .   .
           .   .   .   x   .
         .   .   .   x   .
           .   o   .   .   .
         .   o   o   .   .
           .   o   .   .   .
         .   .   o   .   .
           .   .   .   .   .
         .   .   .   .   .   W
        '''
        expected = 1
        self.check_eval(text, expected)
    
        text = '''
           .   .   .   .   .
         .   .   .   .   .
           .   .   .   x   .
         .   .   .   x   .
           .   o   .   .   .
         .   o   o   .   .
           .   o   .   .   .
         .   .   o   .   .
           .   .   .   .   .
         .   .   .   .   .   B
        '''
        expected = 1
        self.check_eval(text, expected)
    
        text = '''
           .   .   .   .   .
         .   .   .   .   .
           .   x   x   x   .
         .   .   x   x   .
           .   .   .   .   .
         .   .   .   .   .
           .   o   .   .   .
         .   .   o   .   .
           .   .   .   .   .
         .   .   .   .   .   W
        '''
        expected = 0
        self.check_eval(text, expected)
    
        text = '''
           .   .   .   .   .
         .   .   .   .   .
           .   x   x   x   .
         .   .   x   x   .
           .   .   .   .   .
         .   .   .   .   .
           .   o   .   .   .
         .   .   o   .   .
           .   .   .   .   .
         .   .   .   .   .   W
        '''
        expected = 0
        self.check_eval(text, expected)


if __name__ == '__main__':
    import unittest
    unittest.main()
