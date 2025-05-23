#!/usr/bin/env python3

#  (C) Copyright Wieger Wesselink 2022. Distributed under the GPL-3.0
#  Software License, (See accompanying file license.txt or copy at
#  https://www.gnu.org/licenses/gpl-3.0.txt)

# Default MCTS implementation that uses a parent attribute in the nodes.

from draughts1 import *

import math
import random
from mcts_common import init_scan, GlobalSettings, find_move, print_move_between_positions, SimulatePieceCountDiscrete, \
    best_child, log_uct_scores


class MCTSNode(object):
    def __init__(self, state: Pos):
        self.state = state
        self.moves = generate_moves(state)
        self.children = []
        self.parent = None
        self.Q = 0  # the total score of the simulations through this node
        self.N = 0  # the number of simulations through this node

    def is_fully_expanded(self) -> bool:
        return len(self.children) == len(self.moves)


class MCTSTree(object):
    def __init__(self, state: Pos):
        self.nodes = [MCTSNode(state)]

    def root(self):
        return self.nodes[0]

    # Adds a child v to node u, such that play_move(u->state, u->moves[i]) == v->state.
    # Modifies u (N.B. the order of the moves may be changed).
    def add_child(self, u: MCTSNode, i: int) -> MCTSNode:
        j = len(u.children)
        if i > j:
            u.moves.swap(i, j)  # swap moves i and j
        v = MCTSNode(u.state.succ(u.moves[j]))
        u.children.append(v)
        v.parent = u
        return v


def print_path(v: MCTSNode) -> str:
    import mcts_common

    # construct the path from the root to v
    path = [v]
    while v.parent:
        v = v.parent
        path = [v] + path

    return mcts_common.print_path(path)


def expand(tree: MCTSTree, u: MCTSNode) -> MCTSNode:
    i = random.randrange(len(u.children), len(u.moves))  # u.expanded_move_count <= i < len(u.moves)
    return tree.add_child(u, i)


def mcts(tree: MCTSTree, c: float, max_iterations, simulate=SimulatePieceCountDiscrete()) -> MCTSNode:
    for i in range(max_iterations):
        if GlobalSettings.verbose and i % 1000 == 0 and i > 0:
            print(f'i = {i}')

        u = tree.root()

        # selection of a leaf node
        while u.moves:
            if u.is_fully_expanded():
                u = best_child(u, c)
            else:
                break

        # expansion of the tree
        if u.moves:
            u = expand(tree, u)

            if GlobalSettings.play_forced_moves:
                while len(u.moves) == 1:
                    u = tree.add_child(u, 0)

            if GlobalSettings.debug:
                print(f'path {print_path(u)}')
        else:
            if GlobalSettings.debug:
                print(f'no expansion possible')

        # simulation
        Delta = simulate(u.state)

        # backpropagation
        v = u
        while v is not None:
            v.Q += Delta
            v.N += 1
            v = v.parent

        if GlobalSettings.debug:
            log_uct_scores(tree, c)

    return best_child(tree.root(), 0)


def run():
    text = '''
           .   .   .   .   . 
         .   .   .   x   x   
           .   .   .   .   . 
         .   .   .   .   x   
           .   .   .   .   . 
         .   .   .   .   .   
           .   .   .   .   o 
         .   x   o   o   o   
           .   .   .   .   . 
         .   .   .   .   .   W
        '''
    pos = parse_position(text)
    display_position(pos)

    GlobalSettings.verbose = True
    GlobalSettings.debug = True
    max_iterations = 10000
    tree = MCTSTree(pos)
    c = 1.0 / math.sqrt(2)
    u = mcts(tree, c, max_iterations)
    m = find_move(pos, u.state)
    print(f'best move: {print_move(m, pos)}')


if __name__ == '__main__':
    init_scan()
    run()
