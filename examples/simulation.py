#!/usr/bin/env python3

#  (C) Copyright Wieger Wesselink 2022. Distributed under the GPL-3.0
#  Software License, (See accompanying file license.txt or copy at
#  https://www.gnu.org/licenses/gpl-3.0.txt)

# This program runs MCTS with different simulation strategies on a position in which
# white has three pieces less. The results of MCTS are very poor.

from typing import List
from draughts1 import *
from mcts_common import mcts_to_dot, Simulate, SimulatePieceCountDiscrete, SimulatePieceCountContinuous, SimulateScanDiscrete, SimulateScanContinuous
from mcts import *


def save_pdf(filename, u, depth=2, initial_moves='', skipzero=True):
    graph = mcts_to_dot(u, depth, initial_moves, skipzero)
    graph.filename = filename
    graph.format = 'pdf'
    graph.render()


def check_position(text: str, max_iterations: List[int], c: float = 1.0 / math.sqrt(2)):
    pos = parse_position(text)
    simulations = [SimulatePieceCountDiscrete(), SimulateScanDiscrete(), SimulatePieceCountContinuous(), SimulateScanContinuous()]

    display_position(pos)

    for n in max_iterations:
        for simulate in simulations:
            print(f'simulate: {simulate}')
            print(f'iterations: {n}')
            tree = MCTSTree(pos)
            u = mcts(tree, c, n, simulate)
            m = find_move(pos, u.state)
            print(f'best move: {print_move(m, pos)}')
            log_uct_scores(tree)


def run():
    # Note that white is 3 pieces behind.
    # In this position often the move 34-29? is played, which costs a piece.
    text = '''
           .   x   .   .   x 
         x   .   .   x   .   
           .   .   .   .   x 
         .   x   .   .   x   
           x   .   .   .   x 
         x   x   .   .   x   
           .   .   .   o   x 
         o   .   o   o   o   
           .   o   .   o   . 
         .   o   o   .   o   W
        '''
    max_iterations = [1000, 10000]
    check_position(text, max_iterations)

    # Now the number of pieces is equal. The mistake 34-29? is no longer made.
    text = '''
           .   x   .   .   x 
         x   .   .   x   .   
           .   .   .   .   x 
         .   x   .   .   x   
           x   .   .   .   x 
         x   x   .   .   x   
           .   .   .   o   x 
         o   .   o   o   o   
           o   o   .   o   . 
         o   o   o   o   o   W
        '''
    max_iterations = [1000, 10000]
    check_position(text, max_iterations)

    # This is the position after the mistake 34-29?
    # Black can win a piece using the simple combination 30-34, but this is sometimes missed.
    text = '''
           .   x   .   .   x
         x   .   .   x   .
           .   .   .   .   x
         .   x   .   .   x
           x   .   .   .   x
         x   x   .   o   x
           .   .   .   .   x
         o   .   o   o   o
           .   o   .   o   .
         .   o   o   .   o   B
        '''
    max_iterations = [1000, 10000]
    check_position(text, max_iterations)


if __name__ == '__main__':
    init_scan()
    run()
