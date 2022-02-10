// Copyright: Wieger Wesselink 2022
// Distributed under the Distributed under the GPL-3.0 Software License.
// (See accompanying file license.txt or copy at https://www.gnu.org/licenses/gpl-3.0.txt)

#ifndef TERMINAL_HPP
#define TERMINAL_HPP

#include "common.hpp"
#include "game.hpp"

class Terminal {

  private:

    static constexpr double Time {5.0};

    bool m_computer[Side_Size];
    Game m_game;
    Depth m_depth;
    int64 m_nodes;
    double m_time;

    Move user_move ();
    void new_game  (const Pos & pos = pos::Start);
    void go_to     (int ply);

  public:

    void loop ();
};

void disp_game(const Game & game);

void init_high ();
void init_low  ();

void run_terminal_game();

#endif // !defined TERMINAL_HPP

