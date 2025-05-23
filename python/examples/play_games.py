#!/usr/bin/env python3

#  (C) Copyright Wieger Wesselink 2022. Distributed under the GPL-3.0
#  Software License, (See accompanying file license.txt or copy at
#  https://www.gnu.org/licenses/gpl-3.0.txt)

import math
from pathlib import Path
from typing import List, Dict

from draughts1 import *
import mcts
import mcts_traps
from mcts_common import init_scan, Move, find_move, SimulateMinimaxWithShuffle, Simulate, SimulatePieceCountDiscrete, \
    StopWatch


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


def game_result_string(result: GameResult) -> str:
    if result == GameResult.Win:
        return '2-0'
    elif result == GameResult.Draw:
        return '1-1'
    elif result == GameResult.Loss:
        return '0-2'
    return '?'


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

    def get_end_position(self):
        pos = start_position()
        for m in self.moves:
            pos = pos.succ(m)
        return pos

    def to_pdn(self) -> str:
        printer = PDNPrinter()
        printer.write_white(self.white)
        printer.write_black(self.black)
        printer.write_result(game_result_string(self.result))
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
        return f'ScanPlayer(depth={self.max_depth})'


# minimax with a piece count evaluation in the leaves
# moves are not shuffled
class MinimaxPlayer(Player):
    def __init__(self, max_depth: int):
        self.max_depth = max_depth

    def play(self, pos: Pos) -> Move:
        score, move = minimax_search(pos, self.max_depth)
        return move

    def name(self) -> str:
        return f'MinimaxPlayer(depth={self.max_depth})'


# minimax with a piece count evaluation in the leaves
# moves are shuffled
class MinimaxPlayerWithShuffle(Player):
    def __init__(self, max_depth: int):
        self.max_depth = max_depth

    def play(self, pos: Pos) -> Move:
        score, move = minimax_search_with_shuffle(pos, self.max_depth)
        return move

    def name(self) -> str:
        return f'MinimaxPlayerWithShuffle(depth={self.max_depth})'


# minimax with a Scan evaluation in the leaves
class MinimaxPlayerScan(Player):
    def __init__(self, max_depth: int):
        self.max_depth = max_depth

    def play(self, pos: Pos) -> Move:
        score, move = minimax_search_scan(pos, self.max_depth)
        return move

    def name(self) -> str:
        return f'MinimaxPlayerScan(depth={self.max_depth})'


# uses the mcts algorithm
class MCTSPlayer(Player):
    def __init__(self, max_iterations: int, max_time: float = 3600, c: float = 1.0 / math.sqrt(2), simulate: Simulate=SimulatePieceCountDiscrete()):
        self.max_iterations = max_iterations
        self.max_time = max_time
        self.c = c
        self.simulate = simulate

    def play(self, pos: Pos) -> Move:
        tree = mcts.MCTSTree(pos)
        u = mcts.mcts(tree, self.c, self.max_iterations, self.simulate)
        return find_move(pos, u.state)

    def name(self) -> str:
        return f'MCTSPlayer(max_iterations={self.max_iterations},simulate={self.simulate})'


# uses the mcts_traps algorithm
class MCTSTrapsPlayer(Player):
    def __init__(self, max_iterations: int, max_time: float = 3600, c: float = 1.0 / math.sqrt(2), simulate: Simulate=SimulatePieceCountDiscrete()):
        self.max_iterations = max_iterations
        self.max_time = max_time
        self.c = c
        self.simulate = simulate

    def play(self, pos: Pos) -> Move:
        tree = mcts_traps.MCTSTree(pos)
        u = mcts_traps.mcts(tree, self.c, self.max_iterations, self.simulate)
        while True:
            m = find_move(pos, u.state)
            if m:
                break
            else:
                u = mcts_traps.MCTSTree.first_moves[u]
        return m

    def name(self) -> str:
        return f'MCTSTrapsPlayer(max_iterations={self.max_iterations},simulate={self.simulate})'


def play_game(player1: Player, player2: Player, moves: List[Move], max_moves: int = 150, verbose=False) -> Game:
    watch = StopWatch()
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

    if verbose:
        print(f'{game.white} - {game.black} {game_result_string(game.result):^3s}  time = {watch.seconds():6.4}s')

    return game


def match_result(games: List[Game]) -> Dict:
    wins = {}
    wins[games[0].white] = 0
    wins[games[0].black] = 0
    for game in games:
        if game.result == GameResult.Win:
            wins[game.white] += 1
        elif game.result == GameResult.Loss:
            wins[game.black] += 1
    return wins


def display_match_result(games: List[Game]) -> None:
    wins = match_result(games)
    player1, player2 = sorted(wins.keys())
    print(f'match result: {player1} - {player2}   {wins[player1]} - {wins[player2]}\n')


def play_dxp_match(player1: Player, player2: Player, max_moves: int = 150, verbose=False) -> List[Game]:
    games = []
    for moves in n_move_sequences(2):
        game = play_game(player1, player2, moves, max_moves, verbose)
        games.append(game)

        game = play_game(player2, player1, moves, max_moves, verbose)
        games.append(game)

    display_match_result(games)
    return games


def save_games(filename: str, games: List[Game]) -> None:
    text = '\n\n'.join(game.to_pdn() for game in games)
    Path(filename).write_text(text)


def determine_player_strength(player: Player,
                              start_depth=1,
                              verbose=False,
                              Opponent=MinimaxPlayerWithShuffle) -> None:
    """Determines the strength of a player. It plays DXP matches against a minimax player with increasing depth,
    until the minimax player wins. The games are saved in PDN format.

    :param player: The player that is being tested
    :param start_depth: The minimax depth used in the first DXP match
    :param verbose: If true the results of the games are displayed
    :param Opponent: The opponent that is used to measure the strength (`MinimaxPlayerWithShuffle` or `MinimaxPlayerScan`)
    """
    for depth in range(start_depth, 12):
        opponent = Opponent(max_depth=depth)
        games = play_dxp_match(player, opponent, verbose=verbose)
        filename = f'DXP match {player.name()} - {opponent.name()}.pdn'
        save_games(filename, games)
        wins = match_result(games)
        if wins[opponent.name()] > wins[player.name()]:
            break


def main():
    player1 = ScanPlayer(max_depth=5)
    player2 = MinimaxPlayer(max_depth=6)
    player3 = MinimaxPlayerWithShuffle(max_depth=6)
    player4 = MinimaxPlayerScan(max_depth=6)
    player5 = MCTSPlayer(max_iterations=500)
    player6 = MCTSTrapsPlayer(max_iterations=500)
    player7 = MCTSPlayer(max_iterations=500, simulate=SimulateMinimaxWithShuffle(4))

    game = play_game(player5, player6, [], max_moves=150)
    print(game.to_pdn(), '\n')

    game = play_game(player5, player7, [], max_moves=150)
    print(game.to_pdn(), '\n')

    play_dxp_match(player2, player3, verbose=True)
    play_dxp_match(player1, player4, verbose=True)
    determine_player_strength(MinimaxPlayerScan(max_depth=2))


if __name__ == '__main__':
    init_scan(bb_size=6)
    main()
