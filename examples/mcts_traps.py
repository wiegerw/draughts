#!/usr/bin/env python3

#  (C) Copyright Wieger Wesselink 2022. Distributed under the GPL-3.0
#  Software License, (See accompanying file license.txt or copy at
#  https://www.gnu.org/licenses/gpl-3.0.txt)

# A modified MCTS implementation that can solve traps more easily.

from draughts1 import *

import math
import random
from typing import List
from mcts_common import init_scan, GlobalSettings, find_move, SimulatePieceCountDiscrete, best_child, print_path, \
    log_uct_scores


class MCTSNode(object):
    def __init__(self, state: Pos):
        self.state = state
        self.moves = generate_moves(state)
        self.children = []
        self.Q = 0  # the total score of the simulations through this node
        self.N = 0  # the number of simulations through this node
        self.expanded_move_count = 0  # the number of moves in self.moves that have been expanded

    def is_fully_expanded(self) -> bool:
        return self.expanded_move_count == len(self.moves)


class MCTSTree(object):
    first_moves = {}  # mapping that stores the first move of all jump moves

    def __init__(self, state: Pos):
        self.nodes = [MCTSNode(state)]

    def root(self):
        return self.nodes[0]

    # Adds a child v to node u, such that play_move(u->state, u->moves[i]) == v->state.
    # Modifies u (N.B. the order of the moves may be changed).
    def add_child(self, u: MCTSNode, i: int) -> MCTSNode:
        j = u.expanded_move_count
        if i > j:
            u.moves.swap(i, j)  # swap moves i and j
        v = MCTSNode(u.state.succ(u.moves[j]))
        u.children.append(v)
        u.expanded_move_count += 1
        return v


def expand(tree: MCTSTree, u: MCTSNode) -> MCTSNode:
    i = random.randrange(u.expanded_move_count, len(u.moves))  # u.expanded_move_count <= i < len(u.moves)
    return tree.add_child(u, i)


# add a connection from the root to the end of the path
def add_root_connection(tree: MCTSTree, path: List[MCTSNode]) -> None:
    u = tree.root()
    v = path[-1]
    if u.state.is_white_to_move() == v.state.is_white_to_move():
        v = path[-2]
    u.children.append(v)
    MCTSTree.first_moves[v] = path[1]


def mcts(tree: MCTSTree, c: float, max_iterations, simulate=SimulatePieceCountDiscrete()) -> MCTSNode:
    for i in range(max_iterations):
        if GlobalSettings.verbose and i % 1000 == 0 and i > 0:
            print(f'i = {i}')

        u = tree.root()
        path = [u]
        forced = True

        # selection of a leaf node
        while u.moves:
            if len(path) % 2 == 0 and len(u.moves) > 1:
                forced = False

            if u.is_fully_expanded():
                u = best_child(u, c)
                path.append(u)
            else:
                break

        # expansion of the tree
        if u.moves:
            u = expand(tree, u)
            path.append(u)

            if GlobalSettings.play_forced_moves:
                while len(u.moves) == 1:
                    u = tree.add_child(u, 0)
                    path.append(u)

            if GlobalSettings.debug:
                print(f'path {print_path(path)}')

            # add a root connection if the line from u to v is forced
            if forced and len(path) > 3:
                add_root_connection(tree, path)
        else:
            if GlobalSettings.debug:
                print(f'no expansion possible')

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
         .   x   x   x   x   
           .   .   .   .   . 
         .   x   .   .   x   
           .   .   .   .   . 
         o   .   .   .   .   
           .   .   .   .   o 
         .   o   o   o   o   
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
    while True:
        m = find_move(pos, u.state)
        if m:
            break
        else:
            u = MCTSTree.first_moves[u]
    print(f'best move: {print_move(m, pos)}')


if __name__ == '__main__':
    init_scan()
    run()
