#!/usr/bin/env python3

#  (C) Copyright Wieger Wesselink 2022. Distributed under the GPL-3.0
#  Software License, (See accompanying file license.txt or copy at
#  https://www.gnu.org/licenses/gpl-3.0.txt)

import math
from typing import List

from draughts1 import *
import mcts
import mcts_traps
from mcts_common import init_scan, Move, find_move, SimulateMinimaxWithShuffle, Simulate, SimulatePieceCountEval


def print_pdn_moves(moves: List[str]):
    text = ''
    for i, m in enumerate(moves):
        if i % 2 == 0:
            text = text + '%3d' % (i / 2 + 1) + '.'
        text = text + m
        if i % 2 == 0:
            text = text + ' '
        if i % 10 == 9:
            text = text + '\n'
    return text


class PDNPrinter:
    def __init__(self):
        self.text = ''

    def write_tag(self, key, value):
        if value != "":
            self.text = self.text + '[%s "%s"]\n' % (key, value)

    def write_white(self, player):
        self.write_tag("White", player)

    def write_black(self, player):
        self.write_tag("Black", player)

    def write_event(self, event):
        self.write_tag("Event", event)

    def write_date(self, date):
        self.write_tag("Date", date)

    def write_site(self, site):
        self.write_tag("Site", site)

    def write_round(self, round):
        self.write_tag("Round", round)

    def write_result(self, result):
        self.write_tag("Result", result)

    def write_white_clock(self, clock):
        self.write_tag("WhiteClock", clock)

    def write_black_clock(self, clock):
        self.write_tag("BlackClock", clock)

    def write_moves(self, moves):
        self.text = self.text + print_pdn_moves(moves)


class Game(object):
    def __init__(self, white: str, black: str, moves=None, result=GameResult.Unknown):
        self.white = white
        self.black = black
        self.moves = moves if moves else []
        self.result = result

    def get_moves(self):
        result = []
        pos = start_position()
        for m in self.moves:
            result.append(f'{print_move(m, pos):5}')
            pos = pos.succ(m)
        return result

    def get_result(self):
        if self.result == GameResult.Win:
            return '2-0'
        elif self.result == GameResult.Draw:
            return '1-1'
        elif self.result == GameResult.Loss:
            return '0-2'
        return '?'

    def get_end_position(self):
        pos = start_position()
        for m in self.moves:
            pos = pos.succ(m)
        return pos

    def to_pdn(self) -> str:
        printer = PDNPrinter()
        printer.write_white(self.white)
        printer.write_black(self.black)
        printer.write_result(self.get_result())
        return printer.text + print_pdn_moves(self.get_moves())


# Returns all games with n moves, except the ones in which one of the players loses material
def n_move_sequences(n) -> List[List[Move]]:
    games = [(start_position(), [])]

    for i in range(n):
        new_games = []
        for (pos, moves) in games:
            for move in generate_moves(pos):
               new_games.append((pos.succ(move), moves + [move]))
        games = new_games

    return [moves for (pos, moves) in games if minimax_search_with_shuffle(pos, 4)[0] == 0]


# base class for players
class Player(object):
    def play(self, pos: Pos) -> Move:
        pass

    def name(self) -> str:
        pass


# uses the scan search function
class ScanPlayer(Player):
    def __init__(self, max_depth: int, max_time: float = 3600):
        self.max_depth = max_depth
        self.max_time = max_time

    def play(self, pos: Pos) -> Move:
        score, move = scan_search(pos, self.max_depth, self.max_time)
        return move

    def name(self) -> str:
        return f'Scan player depth = {self.max_depth}'


# minimax with a piece count evaluation in the leaves
# moves are not shuffled
class MinimaxPlayer(Player):
    def __init__(self, max_depth: int):
        self.max_depth = max_depth

    def play(self, pos: Pos) -> Move:
        score, move = minimax_search(pos, self.max_depth)
        return move

    def name(self) -> str:
        return f'Minimax player depth = {self.max_depth}'


# minimax with a piece count evaluation in the leaves
# moves are shuffled
class MinimaxPlayerWithShuffle(Player):
    def __init__(self, max_depth: int):
        self.max_depth = max_depth

    def play(self, pos: Pos) -> Move:
        score, move = minimax_search_with_shuffle(pos, self.max_depth)
        return move

    def name(self) -> str:
        return f'Minimax player with shuffle depth = {self.max_depth}'


# minimax with a Scan evaluation in the leaves
class MinimaxPlayerScan(Player):
    def __init__(self, max_depth: int):
        self.max_depth = max_depth

    def play(self, pos: Pos) -> Move:
        score, move = minimax_search_scan(pos, self.max_depth)
        return move

    def name(self) -> str:
        return f'Minimax player Scan depth = {self.max_depth}'


# uses the mcts algorithm
class MCTSPlayer(Player):
    def __init__(self, max_iterations: int, max_time: float = 3600, c: float = 1.0 / math.sqrt(2), simulate: Simulate=SimulatePieceCountEval()):
        self.max_iterations = max_iterations
        self.max_time = max_time
        self.c = c

    def play(self, pos: Pos) -> Move:
        tree = mcts.MCTSTree(pos)
        u = mcts.mcts(tree, self.c, self.max_iterations)
        return find_move(pos, u.state)

    def name(self) -> str:
        return f'MCTS max_iterations = {self.max_iterations}'


# uses the mcts_traps algorithm
class MCTSTrapsPlayer(Player):
    def __init__(self, max_iterations: int, max_time: float = 3600, c: float = 1.0 / math.sqrt(2), simulate: Simulate=SimulatePieceCountEval()):
        self.max_iterations = max_iterations
        self.max_time = max_time
        self.c = c

    def play(self, pos: Pos) -> Move:
        tree = mcts_traps.MCTSTree(pos)
        u = mcts_traps.mcts_traps(tree, self.c, self.max_iterations)
        while True:
            m = find_move(pos, u.state)
            if m:
                break
            else:
                u = mcts_traps.MCTSTree.first_moves[u]
        return m

    def name(self) -> str:
        return f'MCTS Traps max_iterations = {self.max_iterations}'


def play_game(player1: Player, player2: Player, moves: List[Move], max_moves: int = 150) -> Game:
    game = Game(player1.name(), player2.name(), moves[:])
    pos = game.get_end_position()
    for i in range(max_moves):
        result = compute_position_result(pos)
        if result != GameResult.Unknown:
            game.result = result
            break

        m = player1.play(pos) if i % 2 == 0 else player2.play(pos)
        game.moves.append(m)
        pos = pos.succ(m)

    if game.result == GameResult.Unknown:
        game.result = compute_position_result(pos)

    return game


def display_match_result(games: List[Game]) -> None:
    wins = {}
    wins[games[0].white] = 0
    wins[games[0].black] = 0
    for game in games:
        if game.result == GameResult.Win:
            wins[game.white] += 1
        elif game.result == GameResult.Loss:
            wins[game.black] += 1
    player1, player2 = sorted(wins.keys())
    print(f'match result: {player1} - {player2} {wins[player1]} - {wins[player2]}')


def play_dxp_match(player1: Player, player2: Player, max_moves: int = 150) -> None:
    games = []
    for moves in n_move_sequences(2):
        game = play_game(player1, player2, moves, max_moves)
        games.append(game)
        print(game.to_pdn())
        print('')

        game = play_game(player2, player1, moves, max_moves)
        games.append(game)
        print(game.to_pdn())
        print('')

    display_match_result(games)


def main():
    max_moves = 150
    player1 = ScanPlayer(max_depth=6)
    player2 = MinimaxPlayer(max_depth=7)
    player3 = MinimaxPlayerWithShuffle(max_depth=7)
    player4 = MinimaxPlayerScan(max_depth=7)
    player5 = MCTSPlayer(max_iterations=1000)
    player6 = MCTSTrapsPlayer(max_iterations=1000)
    player7 = MCTSPlayer(max_iterations=1000, simulate=SimulateMinimaxWithShuffle(5))

    game = play_game(player5, player6, [], max_moves)
    print(game.to_pdn())
    print('')

    game = play_game(player5, player7, [], max_moves)
    print(game.to_pdn())
    print('')

    play_dxp_match(player2, player3)
    print('')

    play_dxp_match(player1, player4)


if __name__ == '__main__':
    init_scan(bb_size=6)
    main()
