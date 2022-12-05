#!/usr/bin/env python3

#  (C) Copyright Wieger Wesselink 2022. Distributed under the GPL-3.0
#  Software License, (See accompanying file license.txt or copy at
#  https://www.gnu.org/licenses/gpl-3.0.txt)

from play_games import *
import mcts_edge1
import mcts_edge2


class MCTSEdge1Player(Player):
    def __init__(self, max_iterations: int, c: float = 1.0 / math.sqrt(2), simulate: Simulate=SimulatePieceCountDiscrete()):
        self.max_iterations = max_iterations
        self.c = c
        self.simulate = simulate

    def play(self, pos: Pos) -> Move:
        tree = mcts_edge1.MCTSTree(pos)
        u = mcts_edge1.mcts(tree, self.c, self.max_iterations, self.simulate)
        return find_move(pos, u.state)

    def name(self) -> str:
        return f'MCTSEdge1Player(max_iterations={self.max_iterations},simulate={self.simulate})'


class MCTSEdge2Player(Player):
    def __init__(self, max_iterations: int, c: float = 1.0 / math.sqrt(2), simulate: Simulate=mcts_edge2.SimulatePieceCountEdge(), uct: str='uct1'):
        self.max_iterations = max_iterations
        self.c = c
        self.simulate = simulate
        self.uct = uct
        self.uct_function = mcts_edge2.uct1 if self.uct == 'uct1' else mcts_edge2.uct2

    def play(self, pos: Pos) -> Move:
        tree = mcts_edge2.MCTSTree(pos)

        u = mcts_edge2.mcts(tree, self.c, self.max_iterations, self.simulate, self.uct_function)
        return find_move(pos, u.state)

    def name(self) -> str:
        return f'MCTSEdge2Player(max_iterations={self.max_iterations},simulate={self.simulate},uct={self.uct})'


def main():
    player1 = MCTSEdge1Player(max_iterations=1000)
    player2 = MCTSEdge2Player(max_iterations=1000, uct='uct1')
    player3 = MCTSEdge2Player(max_iterations=1000, uct='uct2')
    determine_player_strength(player1, start_depth=3, verbose=True)
    determine_player_strength(player2, start_depth=3, verbose=True)
    determine_player_strength(player3, start_depth=3, verbose=True)


if __name__ == '__main__':
    init_scan(bb_size=6)
    main()
