#!/usr/bin/env python3

#  (C) Copyright Wieger Wesselink 2022. Distributed under the GPL-3.0
#  Software License, (See accompanying file license.txt or copy at
#  https://www.gnu.org/licenses/gpl-3.0.txt)

# Alternative MCTS implementation that does not use a parent attribute in the nodes.
# Instead, a path from the root to a leaf is explicitly constructed inside the MCTS algorithm.

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


def find_move(u: Pos, v: Pos) -> Move:
    moves = generate_moves(u)
    for m in moves:
        if u.succ(m) == v:
            return m
    return move_none()


def print_move_between_positions(src: Pos, dest: Pos) -> str:
    m = find_move(src, dest)
    return print_move(m, src) if m else f'jump({print_position(dest, False, True)})'


def normalize_piece_count_score(score: int) -> float:
    if score > 0:
        return 1
    elif score < 0:
        return 0
    else:
        return 0.5


def normalize_scan_score(pos: Pos, score: int) -> float:
    if not pos.is_white_to_move():
        score = -score
    return (score - score_inf()) / (2 * score_inf())


# Base class for simulation (rollout).
# The value is normalized to the interval [0,1], and it is from the perspective of the white player.
class Simulate(object):
    def __call__(self, pos: Pos) -> float:
        raise NotImplementedError


# Piece count evaluation
class SimulatePieceCountEval(Simulate):
    def __call__(self, pos: Pos) -> float:
        score = piece_count_eval(play_forced_moves(pos))
        return normalize_piece_count_score(score)


# Scan evaluation
class SimulateScanEval(Simulate):
    def __call__(self, pos: Pos) -> float:
        score = eval_position(pos)
        return normalize_scan_score(pos, score)


# Minimax with shuffle
class SimulateMinimaxWithShuffle(Simulate):
    def __init__(self, max_depth: int):
        self.max_depth = max_depth

    def __call__(self, pos: Pos) -> float:
        score, move = minimax_search_with_shuffle(pos, self.max_depth)
        return normalize_piece_count_score(score)


# Minimax with Scan evaluation
class SimulateMinimaxScan(Simulate):
    def __init__(self, max_depth: int):
        self.max_depth = max_depth

    def __call__(self, pos: Pos) -> float:
        score, move = minimax_search_scan(pos, self.max_depth)
        return normalize_scan_score(pos, score)
