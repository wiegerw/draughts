#!/usr/bin/env python3

#  (C) Copyright Wieger Wesselink 2022. Distributed under the GPL-3.0
#  Software License, (See accompanying file license.txt or copy at
#  https://www.gnu.org/licenses/gpl-3.0.txt)

import unittest
from draughts1 import *

def print_moves(moves):
    result = []
    pos = start_position()
    for move in moves:
        result.append(print_move(move, pos))
        pos = pos.succ(move)
    return result


class Test(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
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

    def test_game(self):
        text = '''[Event "NLD-chT Ereklasse"]
[Date "2005.01.15"]
[White "Harm Wiersma"]
[Black "Martijn Vissers"]
[Result "1-1"]
[WhiteClock "1.59"]
[BlackClock "1.59"]
  1.33-28 18-22  2.38-33 12-18  3.31-26  7-12  4.37-31 19-23  5.28x19 14x23
  6.32-27 23-28  7.34-30 20-24  8.30x19 13x24  9.40-34  1-7  10.34-30  9-13
 11.30x19 13x24 12.45-40  4-9  13.40-34  9-13 14.44-40  3-9  15.34-30 15-20
 16.30x19 13x24 17.42-38 10-15 18.40-34  5-10 19.47-42  9-14 20.38-32 28x37
 21.41x32 24-29 22.33x24 20x40 23.35x44 14-19 24.46-41 10-14 25.41-37  8-13
 26.42-38  2-8  27.44-40 22-28 28.32x23 18x29 29.27-22 17x28 30.26-21 16x27
 31.31x24 19x30 32.40-34 30-35 33.39-33 12-18 34.37-32  7-12 35.33-28 11-17
 36.36-31 13-19 37.38-33 14-20 38.43-38 19-24 39.31-27 24-29 40.34x23 18x29
 41.33x24 20x29 42.38-33 29x38 43.32x43 12-18 44.43-39  8-13 45.48-42  6-11
 46.42-38 11-16 47.38-32 13-19 48.49-43 15-20 49.43-38 17-21 50.39-34 20-25
 51.38-33'''
        game = parse_pdn_game(text)
        moves = print_moves(game.moves)
        self.assertEqual('33-28 18-22 38-33 12-18 31-26 7-12 37-31 19-23 28x19 14x23 32-27 23-28 34-30 20-24 30x19 13x24 40-34 1-7 34-30 9-13 30x19 13x24 45-40 4-9 40-34 9-13 44-40 3-9 34-30 15-20 30x19 13x24 42-38 10-15 40-34 5-10 47-42 9-14 38-32 28x37 41x32 24-29 33x24 20x40 35x44 14-19 46-41 10-14 41-37 8-13 42-38 2-8 44-40 22-28 32x23 18x29 27-22 17x28 26-21 16x27 31x24 19x30 40-34 30-35 39-33 12-18 37-32 7-12 33-28 11-17 36-31 13-19 38-33 14-20 43-38 19-24 31-27 24-29 34x23 18x29 33x24 20x29 38-33 29x38 32x43 12-18 43-39 8-13 48-42 6-11 42-38 11-16 38-32 13-19 49-43 15-20 43-38 17-21 39-34 20-25 38-33', ' '.join(moves))
        self.assertEqual(game.event, "NLD-chT Ereklasse")
        self.assertEqual(game.date, "2005.01.15")
        self.assertEqual(game.white, "Harm Wiersma")
        self.assertEqual(game.black, "Martijn Vissers")
        self.assertEqual(game.result, "1-1")

    def test_file(self):
        games = parse_pdn_file('../games/wiersma.pdn')
        self.assertEqual(len(games), 34)


if __name__ == '__main__':
    import unittest
    unittest.main()

