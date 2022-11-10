#!/usr/bin/env python3

#  (C) Copyright Wieger Wesselink 2022. Distributed under the GPL-3.0
#  Software License, (See accompanying file license.txt or copy at
#  https://www.gnu.org/licenses/gpl-3.0.txt)

# Alternative MCTS implementation that does not use a parent attribute in the nodes.
# Instead, a path from the root to a leaf is explicitly constructed inside the MCTS algorithm.

import math
import io
from typing import Dict, Tuple, List, Any, Set
import graphviz
from draughts1 import *

Move = int  # moves are stored as integers


def init_scan(bb_size = 0):
    Scan.set("variant", "normal")
    Scan.set("book", "false")
    Scan.set("book-ply", "4")
    Scan.set("book-margin", "4")
    Scan.set("ponder", "false")
    Scan.set("threads", "1")
    Scan.set("tt-size", "24")
    Scan.set("bb-size", f"{bb_size}")
    Scan.update()
    Scan.init()


class GlobalSettings(object):
    verbose = False
    debug = False


def print_path(path: List[Any]) -> str:
    out = io.StringIO()
    for i in range(len(path) - 1):
        u = path[i]
        v = path[i + 1]
        out.write(print_move_between_positions(u.state, v.state) + ' ')
    return out.getvalue()


def opponent_score(u) -> float:
    return u.N - u.Q if u.state.is_white_to_move() else u.Q


def uct(u, v, c: float = 0):
    return opponent_score(v) / v.N + c * math.sqrt(2 * math.log(u.N) / v.N)


def best_child(u, c: float):
    assert u.children

    def key(i: int) -> float:
        v = u.children[i]
        return uct(u, v, c)

    i = max(range(len(u.children)), key=key)
    return u.children[i]


# print the uct scores of the root of the tree
def log_uct_scores(tree, c: float = 0) -> None:
    print('uct scores')
    u = tree.root()
    for v in u.children:
        print(f'{print_move_between_positions(u.state, v.state):>5s}  uct({c}) = {uct(u, v, c):.4f}  N = {v.N:5d}  Q = {v.Q:5.3f}')
    print('')


def find_move(u: Pos, v: Pos) -> Move:
    moves = generate_moves(u)
    for m in moves:
        if u.succ(m) == v:
            return m
    return move_none()


def print_move_between_positions(src: Pos, dest: Pos) -> str:
    m = find_move(src, dest)
    return print_move(m, src) if m else f'jump({print_position(dest, False, True)})'


# Normalize the value to 0 (black wins), 0.5 (draw), 1.0 (white wins)
def normalize_piece_count_discrete(score: int) -> float:
    if score > 0:
        return 1
    elif score < 0:
        return 0
    else:
        return 0.5


# Normalize the value to 0 (black wins), 0.5 (draw), 1.0 (white wins)
def normalize_scan_discrete(pos: Pos, score: int) -> float:
    if abs(score) < 50:
        return 0.5
    if not pos.is_white_to_move():
        score = -score
    return 0 if score < 0 else 1


# Normalize the value to the interval [0,1].
def normalize_piece_count_continuous(score: int) -> float:
    return math.tanh(score / 5) / 2 + 0.5


# Normalize the value to the interval [0,1].
def normalize_scan_continuous(pos: Pos, score: int) -> float:
    if not pos.is_white_to_move():
        score = -score
    return math.tanh(score / 100) / 2 + 0.5


# Base class for simulation (rollout).
# The value is normalized to the interval [0,1], and it is from the perspective of the white player.
class Simulate(object):
    def __call__(self, pos: Pos) -> float:
        raise NotImplementedError


# Piece count evaluation
class SimulatePieceCountDiscrete(Simulate):
    def __call__(self, pos: Pos) -> float:
        score = piece_count_eval(play_forced_moves(pos))
        return normalize_piece_count_discrete(score)

    def __str__(self):
        return 'SimulatePieceCountDiscrete'


# Continuous piece count evaluation
class SimulatePieceCountContinuous(Simulate):
    def __call__(self, pos: Pos) -> float:
        score = piece_count_eval(play_forced_moves(pos))
        return normalize_piece_count_continuous(score)

    def __str__(self):
        return 'SimulatePieceCountContinuous'


# Scan evaluation
class SimulateScanDiscrete(Simulate):
    def __call__(self, pos: Pos) -> float:
        score = eval_position(pos)
        return normalize_scan_discrete(pos, score)

    def __str__(self):
        return 'SimulateScanDiscrete'


# Scan evaluation
class SimulateScanContinuous(Simulate):
    def __call__(self, pos: Pos) -> float:
        score = eval_position(pos)
        return normalize_scan_continuous(pos, score)

    def __str__(self):
        return 'SimulateScanContinuous'


# Minimax with shuffle
class SimulateMinimaxWithShuffle(Simulate):
    def __init__(self, max_depth: int):
        self.max_depth = max_depth

    def __call__(self, pos: Pos) -> float:
        score, move = minimax_search_with_shuffle(pos, self.max_depth)
        return normalize_piece_count_discrete(score)

    def __str__(self):
        return f'SimulateMinimaxWithShuffle({self.max_depth})'


# Minimax with Scan evaluation
class SimulateMinimaxScan(Simulate):
    def __init__(self, max_depth: int):
        self.max_depth = max_depth

    def __call__(self, pos: Pos) -> float:
        score, move = minimax_search_scan(pos, self.max_depth)
        return normalize_scan_discrete(pos, score)

    def __str__(self):
        return f'SimulateMinimaxScan({self.max_depth})'


# Find the node after playing the given list of moves
def find_mcts_node(u, moves: List[Move]):

    def find_successor(u, pos):
        for v in u.children:
            if v.state == pos:
                return v
        raise RuntimeError('find_successor failed to find the position')

    for move in moves:
        pos = u.state.succ(move)
        u = find_successor(u, pos)

    return u


def mcts_tree_nodes(u, max_depth: int) -> Tuple[Dict, Set[Any]]:
    nodes = {}
    source_nodes = set([])

    def collect(v, depth):
        if v not in nodes:
            nodes[v] = f'u{len(nodes)}'
        if depth > 0:
            source_nodes.add(v)
            for w in v.children:
                collect(w, depth - 1)

    collect(u, max_depth)
    return nodes, source_nodes


def parse_moves(pos: Pos, move_text: str) -> List[Move]:
    result = []
    for word in move_text.split():
        move = parse_move(word, pos)
        result.append(move)
        pos = pos.succ(move)
    return result


def mcts_to_dot(root, max_depth: int, initial_moves: str = '', skipzero=False) -> graphviz.Digraph:
    if initial_moves:
        moves = parse_moves(root.state, initial_moves)
        root = find_mcts_node(root, moves)

    graph = graphviz.Digraph()
    graph.attr(rankdir='LR')

    nodes, source_nodes = mcts_tree_nodes(root, max_depth)

    # create the nodes
    for u, u_id in nodes.items():
        if skipzero and u.Q == 0:
            continue
        label = f'N={u.N}\nQ={u.Q:5.2f}'
        graph.node(u_id, label=label, shape='box', margin='0', width='0.3', height='0.2', fontname='Times-Italic', fontsize='5')

    # create the edges
    for u in source_nodes:
        u_id = nodes[u]
        for v in u.children:
            if skipzero and v.Q == 0:
                continue
            v_id = nodes[v]
            label = print_move_between_positions(u.state, v.state)
            graph.edge(u_id, v_id, label=label, fontname='Times-Italic', fontsize='5')

    return graph
