#!/usr/bin/env python3

#  (C) Copyright Wieger Wesselink 2022. Distributed under the GPL-3.0
#  Software License, (See accompanying file license.txt or copy at
#  https://www.gnu.org/licenses/gpl-3.0.txt)

# Alternative MCTS implementation that does not use a parent attribute in the nodes.
# Instead, a path from the root to a leaf is explicitly constructed inside the MCTS algorithm.

from draughts1 import *

import math
import io
import random
from typing import List
from mcts_common import init_scan, GlobalSettings, find_move, print_move_between_positions, normalize


class MCTSNode(object):
    def __init__(self, state: Pos):
        self.state = state
        self.moves = generate_moves(state)
        self.children = []
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
        return v


def print_path(path: List[MCTSNode]) -> str:
    out = io.StringIO()
    for i in range(len(path) - 1):
        u = path[i]
        v = path[i + 1]
        out.write(print_move_between_positions(u.state, v.state) + ' ')
    return out.getvalue()


def opponent_score(u: MCTSNode) -> float:
    return u.N - u.Q if u.state.is_white_to_move() else u.Q


def uct(u: MCTSNode, v: MCTSNode, c: float = 0):
    return opponent_score(v) / v.N + c * math.sqrt(2 * math.log(u.N) / v.N)


def best_child(u: MCTSNode, c: float) -> MCTSNode:
    assert u.children

    def key(i: int) -> float:
        v = u.children[i]
        return uct(u, v, c)

    i = max(range(len(u.children)), key=key)
    return u.children[i]


# print the uct scores of the root of the tree
def log_uct_scores(tree: MCTSTree, c: float) -> None:
    print('uct scores')
    u = tree.root()
    for v in u.children:
        print(f'{print_move_between_positions(u.state, v.state)} uct = {uct(u, v, c):.4f} N = {v.N}')
    print('')


def expand(tree: MCTSTree, u: MCTSNode) -> MCTSNode:
    i = random.randrange(len(u.children), len(u.moves))  # u.expanded_move_count <= i < len(u.moves)
    return tree.add_child(u, i)


# use a piece count evaluation and normalize it to values in the interval [0,1]
def simulate(u: MCTSNode) -> float:
    value = piece_count_eval(play_forced_moves(u.state))
    return normalize(value)


def mcts(tree: MCTSTree, c: float, max_iterations) -> MCTSNode:
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
                u = tree.add_child(u, 0)
                path.append(u)

            if GlobalSettings.debug:
                print(f'path {print_path(path)}')
        else:
            if GlobalSettings.debug:
                print(f'no expansion possible')

        # simulation
        Delta = simulate(u)

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