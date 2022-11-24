#!/usr/bin/env python3

#  (C) Copyright Wieger Wesselink 2022. Distributed under the GPL-3.0
#  Software License, (See accompanying file license.txt or copy at
#  https://www.gnu.org/licenses/gpl-3.0.txt)

# Alternative MCTS implementation that tries to mimick the edge based version
# in https://github.com/AppliedDataSciencePartners/DeepReinforcementLearning
#
# This version should behave exactly the same as mcts.py.

from draughts1 import *

import math
import random
from mcts_common import init_scan, GlobalSettings, find_move, best_child, print_path, log_uct_scores, \
    Simulate, SimulatePieceCountDiscrete, print_move_between_positions, opponent_score


class MCTSNode(object):
    def __init__(self, state: Pos):
        self.state = state
        self.moves = generate_moves(state)
        self.children = []
        self.Q = 0  # the total score of the simulations through this node
        self.N = 0  # the number of simulations through this node

    def is_fully_expanded(self) -> bool:
        return self.children and all(v.N > 0 for v in self.children)

    def add_children(self):
        if not self.children:
            for move in self.moves:
                self.children.append(MCTSNode(self.state.succ(move)))


class MCTSTree(object):
    def __init__(self, state: Pos):
        self.nodes = [MCTSNode(state)]

    def root(self):
        return self.nodes[0]


def expand(tree: MCTSTree, u: MCTSNode) -> MCTSNode:
    u.add_children()
    V = [v for v in u.children if v.N == 0]
    return random.choice(V)


def uct(u, v, c: float = 0):
    if v.N == 0:
        return 999  # TODO: infinity
    return opponent_score(v) / v.N + c * math.sqrt(2 * math.log(u.N) / v.N)


def best_child(u, c: float):
    assert u.children

    def key(i: int) -> float:
        v = u.children[i]
        return uct(u, v, c)

    i = max(range(len(u.children)), key=key)
    return u.children[i]


def log_uct_scores(tree, c: float = 0) -> None:
    print('uct scores')
    u = tree.root()
    for v in u.children:
        print(f'{print_move_between_positions(u.state, v.state):>5s}  uct({c}) = {uct(u, v, c):8.4f}  N = {v.N:5d}  Q = {v.Q:5.3f}')
    print('')


def mcts(tree: MCTSTree, c: float, max_iterations, simulate: Simulate = SimulatePieceCountDiscrete()) -> MCTSNode:
    tree.root().add_children()

    for i in range(max_iterations):
        if GlobalSettings.verbose and i % 1000 == 0 and i > 0:
            print(f'i = {i}')

        u = tree.root()
        path = [u]

        # selection of a leaf node
        while u.moves:
            if u.is_fully_expanded():
                u = best_child(u, c)
                path.append(u)
            else:
                break

        # expansion of the tree
        if u.moves:
            u = expand(tree, u)
            path.append(u)
            while len(u.moves) == 1:
                assert not u.children
                u.add_children()
                u = u.children[0]
                path.append(u)

            if GlobalSettings.debug:
                print(f'path {print_path(path)}')
        else:
            if GlobalSettings.debug:
                print(f'path {print_path(path)} (no expansion possible)')

        # simulation
        Delta = simulate(u.state)

        # backpropagation
        for v in path:
            v.Q += Delta
            v.N += 1

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
