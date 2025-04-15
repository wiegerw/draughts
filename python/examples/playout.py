#!/usr/bin/env python3

#  (C) Copyright Wieger Wesselink 2022. Distributed under the GPL-3.0
#  Software License, (See accompanying file license.txt or copy at
#  https://www.gnu.org/licenses/gpl-3.0.txt)

from draughts1 import *


def main():
    Scan.set("variant", "normal")
    Scan.set("book", "false")
    Scan.set("book-ply", "4")
    Scan.set("book-margin", "4")
    Scan.set("ponder", "false")
    Scan.set("threads", "4")
    Scan.set("tt-size", "24")
    Scan.set("bb-size", "6")
    Scan.update()
    Scan.init()

    text = '''
       .   .   .   .   . 
     .   .   .   .   .   
       .   x   x   x   . 
     x   .   x   x   .   
       x   .   x   x   o 
     x   o   o   .   o   
       .   o   o   o   o 
     .   o   o   .   .   
       .   .   .   .   . 
     .   .   .   .   .   W
    '''

    pos = parse_position(text)
    max_depth = 10
    max_time = 1.0
    max_moves = 100
    max_nodes = 1000000000000
    verbose = True

    result_minimax = playout_minimax(pos, max_depth, max_time, max_moves, max_nodes, verbose)
    print(f'result_minimax = {result_minimax}')

    result_random = playout_random(pos, max_moves, verbose)
    print(f'result_random = {result_random}')


if __name__ == '__main__':
    main()
