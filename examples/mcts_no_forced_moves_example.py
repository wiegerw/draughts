#!/usr/bin/env python3

#  (C) Copyright Wieger Wesselink 2022. Distributed under the GPL-3.0
#  Software License, (See accompanying file license.txt or copy at
#  https://www.gnu.org/licenses/gpl-3.0.txt)

# Alternative MCTS implementation that does not use a parent attribute in the nodes.
# Instead, a path from the root to a leaf is explicitly constructed inside the MCTS algorithm.

from draughts1 import *

import math
from mcts_common import init_scan, GlobalSettings, find_move
from mcts import MCTSTree, mcts


def test_position(pos, max_iterations, play_forced_moves):
    GlobalSettings.play_forced_moves = play_forced_moves
    tree = MCTSTree(pos)
    c = 1.0 / math.sqrt(2)
    u = mcts(tree, c, max_iterations)
    m = find_move(pos, u.state)
    print(f'best move (iterations = {max_iterations}, play_forced_moves = {play_forced_moves}): {print_move(m, pos)}')


def main():
    # This position can sometimes be solved if play_forced_moves is set to True
    text = '''
           .   .   .   .   . 
         .   .   .   x   .   
           x   .   x   x   . 
         .   x   .   x   .   
           x   .   x   .   o 
         .   .   .   .   .   
           o   o   .   o   o 
         o   .   .   .   o   
           .   .   .   .   o 
         .   .   .   .   .   W
        '''
    pos = parse_position(text)
    display_position(pos)
    print(f'expected move: 25-20\n')

    for max_iterations in [10000, 100000]:
        for play_forced_moves in [False, True]:
            test_position(pos, max_iterations, play_forced_moves)

    print('')

    # With extra pieces on the board even with play_forced_moves the solution cannot be found
    text = '''
           .   .   x   x   . 
         .   .   .   x   .   
           x   .   x   x   . 
         .   x   .   x   .   
           x   .   x   .   o 
         .   .   .   .   .   
           o   o   .   o   o 
         o   .   .   .   o   
           .   .   .   .   o 
         .   o   o   .   .   W
        '''
    pos = parse_position(text)
    display_position(pos)
    print(f'expected move: 25-20\n')

    for max_iterations in [10000, 100000]:
        for play_forced_moves in [False, True]:
            test_position(pos, max_iterations, play_forced_moves)


if __name__ == '__main__':
    init_scan()
    main()
