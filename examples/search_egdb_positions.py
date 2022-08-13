#!/usr/bin/env python3

#  (C) Copyright Wieger Wesselink 2022. Distributed under the GPL-3.0
#  Software License, (See accompanying file license.txt or copy at
#  https://www.gnu.org/licenses/gpl-3.0.txt)

from draughts1 import *

import time


class Timer:
    def __init__(self):
        self._start_time = time.perf_counter()

    def reset(self):
        self._start_time = time.perf_counter()

    def elapsed(self):
        return time.perf_counter() - self._start_time


def scan_search_python(pos, max_depth, max_time):
    if not pos.can_move(pos.turn()):
        best_move = move_none()
        best_score = -score_inf() if pos.is_white_to_move() else score_inf()
        return best_score, best_move

    if pos.opponent_has_no_pieces():
        best_move = move_none()
        best_score = score_inf() if pos.is_white_to_move() else -score_inf()
        return best_score, best_move

    si = SearchInput()
    si.move = True
    si.book = False
    si.depth = max_depth
    si.nodes = 1000000000000
    si.time = max_time
    si.input = True
    si.output = OutputType.None_

    clear_global_transition_table()

    node = make_node(pos)
    so = SearchOutput()
    search(so, node, si)

    if so.score == score_none():
        best_move = so.move
        pos = play_forced_moves(pos)
        score, move = scan_search(pos, max_depth, max_time)
        return score, best_move

    return so.score, so.move


def init_scan():
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


# N.B. It turns out that this way of searching has an enormous overhead. It's not yet clear what is causing this.
def search_egdb_positions(nw, nb, nW, nB, search, max_depth, max_time, max_positions=None):
    enumerator = EGDBEnumerator(nw, nb, nW, nB)
    timer = Timer()
    count = 0
    count_score_zero = 0
    while enumerator.next():
        count += 1
        pos = enumerator.position()
        score, move = search(pos, max_depth, max_time)
        if count < 100:
            print(f'score: {score}')
        if score == 0:
            count_score_zero += 1
        if max_positions and count >= max_positions:
            break
    print(f"searched {count} positions in {timer.elapsed():0.4f} seconds ({count_score_zero} positions with score 0)")


if __name__ == '__main__':
    init_scan()
    nw = 2
    nb = 3
    nW = 0
    nB = 0
    max_depth = 15
    max_time = 1.0
    max_positions = 100
    search_egdb_positions(nw, nb, nW, nB, scan_search_python, max_depth, max_time, max_positions)
    search_egdb_positions(nw, nb, nW, nB, scan_search, max_depth, max_time, max_positions)
