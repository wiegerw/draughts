#!/usr/bin/env python3

#  (C) Copyright Wieger Wesselink 2022. Distributed under the GPL-3.0
#  Software License, (See accompanying file license.txt or copy at
#  https://www.gnu.org/licenses/gpl-3.0.txt)

# Alternative MCTS implementation that tries to mimick the edge based version
# in https://github.com/AppliedDataSciencePartners/DeepReinforcementLearning
#
# Three changes are applied with respect to mcts_edge1.py:
# 1) use scores -1, 0 and 1 instead of 0, 0.5 and 1
# 2) divide by 1 + v.N
# 3) do not use the special value 999 in nodes v with v.N = 0

from draughts1 import *

import math
import random
from mcts_common import init_scan, GlobalSettings, find_move, print_path, Simulate, print_move_between_positions


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

    def __str__(self):
        return f'(N = {self.N}, Q = {self.Q})'


class MCTSTree(object):
    def __init__(self, state: Pos):
        self.nodes = [MCTSNode(state)]

    def root(self):
        return self.nodes[0]


def expand(tree: MCTSTree, u: MCTSNode) -> MCTSNode:
    u.add_children()
    V = [v for v in u.children if v.N == 0]
    return random.choice(V)


def opponent_score(u) -> float:
    return -u.Q if u.state.is_white_to_move() else u.Q


def uct(u, v, c: float = 0):
    if v.N == 0:
        return c * math.sqrt(2 * math.log(u.N + 1))
    else:
        return opponent_score(v) / v.N + c * math.sqrt(2 * math.log(u.N + 1) / (1 + v.N))


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


# Normalize the value to -1 (black wins), 0 (draw), 1 (white wins)
def normalize_piece_count_edge(score: int) -> float:
    if score > 0:
        return 1.0
    elif score < 0:
        return -1.0
    else:
        return 0.0


# Piece count evaluation
class SimulatePieceCountEdge(Simulate):
    def __call__(self, pos: Pos) -> float:
        score = piece_count_eval(play_forced_moves(pos))
        return normalize_piece_count_edge(score)

    def __str__(self):
        return 'SimulatePieceCountEdge'


def mcts(tree: MCTSTree, c: float, max_iterations, simulate: Simulate = SimulatePieceCountEdge()) -> MCTSNode:
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
    max_iterations = 3
    tree = MCTSTree(pos)
    c = 1.0 / math.sqrt(2)
    u = mcts(tree, c, max_iterations)
    m = find_move(pos, u.state)
    print(f'best move: {print_move(m, pos)}')


if __name__ == '__main__':
    init_scan()
    run()
