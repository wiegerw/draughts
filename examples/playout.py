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
    Scan.set("threads", "2")
    Scan.set("tt-size", "24")
    Scan.set("bb-size", "6")
    Scan.update()
    Scan.init()

    text = '''
       .   .   .   .   . 
     .   .   .   .   .   
       .   .   x   x   . 
     x   x   x   x   .   
       x   .   x   x   o 
     x   o   o   .   o   
       .   o   o   .   o 
     .   o   o   o   .   
       .   .   .   .   . 
     .   .   .   .   .   B;
    '''
    for i in range(1):
        pos = parse_position(text)
        result = playout_minimax(pos, 5, 0.1, True, 1000000000000)
        print(f'result = {result}')


if __name__ == '__main__':
    main()

