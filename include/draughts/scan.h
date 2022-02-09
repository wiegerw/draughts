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
#include "var.hpp"

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

// bb_index.hpp
struct egdb_index
{
  static void init() { return bb::index_init(); }
  static bb::ID id_make(int wm, int bm, int wk, int bk) { return bb::id_make(wm, bm, wk, bk); }
  static bool id_is_illegal(bb::ID id) { return bb::id_is_illegal(id); }
  static bool id_is_end(bb::ID id) { return bb::id_is_end(id); }
  static int id_size(bb::ID id) { return bb::id_size(id); }
  static int id_wm(bb::ID id) { return bb::id_wm(id); }
  static int id_bm(bb::ID id) { return bb::id_bm(id); }
  static int id_wk(bb::ID id) { return bb::id_wk(id); }
  static int id_bk(bb::ID id) { return bb::id_bk(id); }
  static std::string id_name(bb::ID id) { return bb::id_name(id); }
  static bb::ID pos_id(const Pos & pos) { return bb::pos_id(pos); }
  static bb::Index pos_index(bb::ID id, const Pos & pos) { return bb::pos_index(id, pos); }
  static bb::Index index_size(bb::ID id) { return bb::index_size(id); }
};

// bb_base.hpp
struct egdb
{
  static void init() { bb::init(); }
  static bool pos_is_load(const Pos& pos) { return bb::pos_is_load(pos); }
  static bool pos_is_search(const Pos& pos, int bb_size) { return bb::pos_is_search(pos, bb_size); }
  static int probe(const Pos& pos) { return bb::probe(pos); } // QS
  static int probe_raw(const Pos& pos) { return bb::probe_raw(pos); } // quiet position
  static int value_update(int node, int child) { return bb::value_update(node, child); }
  static int value_age(int val) { return bb::value_age(val); }
  static int value_max(int v0, int v1) { return bb::value_max(v0, v1); }
  static int value_nega(int val, Side sd) { return bb::value_nega(val, sd); }
  static int value_from_nega(int val) { return bb::value_from_nega(val); }
  static std::string value_to_string(int val) { return bb::value_to_string(val); }
};

// var.hpp
struct scan_settings
{
  static void init() { var::init(); }
  static void load(const std::string& file_name) { var::load(file_name); }
  static void update() { var::update(); }
  static std::string get(const std::string& name) { return var::get(name); }
  static void set(const std::string& name, const std::string& value) { var::set(name, value); }
  static std::string variant_name() { return var::variant_name(); }
};

} // namespace draughts

#endif // DRAUGHTS_PRINT_H
