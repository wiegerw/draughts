// Copyright: Wieger Wesselink 2022
// Distributed under the Distributed under the GPL-3.0 Software License.
// (See accompanying file license.txt or copy at https://www.gnu.org/licenses/gpl-3.0.txt)
//
/// \file draughts/scan.h
/// \brief add your file description here.

#ifndef DRAUGHTS_SCAN_H
#define DRAUGHTS_SCAN_H

#include <algorithm>
#include <sstream>
#include "pos.hpp"

namespace draughts {

template <typename T>
std::string print(const T& t)
{
  std::ostringstream out;
  out << t;
  return out.str();
}

inline
std::string remove_whitespace(std::string text)
{
  text.erase(std::remove_if(text.begin(), text.end(), ::isspace), text.end());
  return text;
}

inline
bool is_white_to_move(const Pos & pos)
{
  return pos.turn() == White;
}

inline
Piece_Side piece_side(const Pos & pos, Square sq) {

  int ps = (bit::bit(pos.empty(), sq) << 2)
           | (bit::bit(pos.king(),  sq) << 1)
           | (bit::bit(pos.black(), sq) << 0);

  return Piece_Side(ps); // can be Empty
}

inline
char piece_char(const Pos& pos, int f)
{
  switch (piece_side(pos, Square(square_sparse(f - 1))))
  {
    case White_Man :  return 'o';
    case Black_Man :  return 'x';
    case White_King : return 'O';
    case Black_King : return 'X';
    default :      return '.';
  }
}

inline
std::string print_position(const Pos& pos, bool print_spaces = false, bool print_to_move = true)
{
  std::ostringstream out;
  unsigned int indent = 1;

  auto space = [&](unsigned int n = 1)
  {
    if (print_spaces)
    {
      out << std::string(n, ' ');
    }
  };

  auto endline = [&]()
  {
    if (print_spaces)
    {
      out << "\n";
    }
  };

  for (int i = 0; i < 10; i++)
  {
    space(indent);
    if (!(i % 2))
    {
      space(2);
    }
    for (int j = 0; j < 5; j++)
    {
      space();
      out << piece_char(pos, 5 * i + j + 1);
      space(2);
    }
    endline();
  }
  if (print_to_move)
  {
    out << (is_white_to_move(pos) ? 'W' : 'B');
  }
  return out.str();
}

inline
Pos parse_position(const std::string& text)
{
  std::string position = remove_whitespace(text);

  auto wm = Bit(0);
  auto bm = Bit(0);
  auto wk = Bit(0);
  auto bk = Bit(0);
  Side turn = std::find(position.begin(), position.end(), 'W') == position.end() ? Black : White;

  for (int f = 0; f < 50; f++)
  {
    Square sq = square_sparse(f);
    auto b = bit::bit(sq);
    switch (position[f])
    {
      case 'O': wk |= b; break;
      case 'o': wm |= b; break;
      case 'X': bk |= b; break;
      case 'x': bm |= b; break;
    }
  }

  // N.B. The constructor of Pos cannot handle an empty board :-(
  if (wk == Bit(0) && wm == Bit(0) && bk == Bit(0) && bm == Bit(0))
  {
    return Pos();
  }

  return Pos(turn, wm, bm, wk, bk);
}

inline
std::string print_bitboard(const Bit& pieces, char piece = 'x', Side turn = White)
{
  Bit empty(0);
  switch (piece)
  {
    case 'x': return print_position(Pos(turn, empty, pieces, empty, empty), true, false);
    case 'O': return print_position(Pos(turn, pieces, empty, pieces, empty), true, false);
    case 'X': return print_position(Pos(turn, empty, pieces, empty, empty), true, false);
    default: return print_position(Pos(turn, pieces, empty, empty, empty), true, false);
  }
}

inline
Pos start_position()
{
  return Pos(White, Bit(0x7DF3EF8000000000), Bit(0x0000000000FBE7DF), Bit(0), Bit(0));
}

} // namespace draughts

#endif // DRAUGHTS_PRINT_H
