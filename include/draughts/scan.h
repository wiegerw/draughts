// Copyright: Wieger Wesselink 2022
// Distributed under the Distributed under the GPL-3.0 Software License.
// (See accompanying file license.txt or copy at https://www.gnu.org/licenses/gpl-3.0.txt)
//
/// \file draughts/scan.h
/// \brief add your file description here.

#ifndef DRAUGHTS_SCAN_H
#define DRAUGHTS_SCAN_H

#include <algorithm>
#include <cassert>
#include <random>
#include <sstream>
#include "scan/bb_base.hpp"
#include "scan/bb_index.hpp"
#include "scan/bit.hpp"
#include "scan/gen.hpp"
#include "scan/list.hpp"
#include "scan/pos.hpp"
#include "scan/search.hpp"
#include "scan/terminal.hpp"
#include "scan/tt.hpp"
#include "scan/var.hpp"
#include <pybind11/numpy.h>

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
  static void init() { init_high(); }
  static void load(const std::string& file_name) { var::load(file_name); }
  static void update() { var::update(); }
  static std::string get(const std::string& name) { return var::get(name); }
  static void set(const std::string& name, const std::string& value) { var::set(name, value); }
  static std::string variant_name() { return var::variant_name(); }
};

// Looks up the result of a position in the endgame database.
// Returns 1 if white wins, 0 for draw and -1 if black wins.
inline
int egdb_lookup(const Pos& pos)
{
  assert(bb::pos_is_load(pos));
  switch(bb::probe(pos))
  {
    case bb::Value::Draw: return 0;
    case bb::Value::Loss: return pos.turn() == White ? -1 : 1;
    case bb::Value::Win: return pos.turn() == White ? 1 : -1;
  }
  throw std::runtime_error("egdb_lookup: unknown result");
}

class ScanPlayer
{
  private:
    Game m_game;

    void new_game(const Pos& pos = pos::Start)
    {
      m_game.init(pos);
      G_TT.clear();
    }

  public:
    int play(const Pos& pos, Depth max_depth = Depth_Max, double max_time = 1.0, int max_moves = 100, int64 max_nodes = 1E12, bool verbose = false)
    {
      Game& game = m_game;
      game.clear();
      new_game(pos);

      Search_Input si;
      si.move = true;
      si.book = false;
      si.depth = max_depth;
      si.nodes = max_nodes;
      si.time = max_time;
      si.input = true;

      if (verbose)
      {
        pos::disp(pos);
      }

      for (auto i = 0; i < max_moves; i++)
      {
        // si.output = verbose ? Output_Terminal : Output_None;
        si.output = Output_None;

        Search_Output so;
        search(so, game.node(), si);

        Move mv = so.move;
        if (mv == move::None) mv = quick_move(game.node());

        if (verbose)
        {
          std::cout << move::to_string(mv, game.pos()) << "\n";
        }

        game.add_move(mv);

        if (!can_move(game.pos(), game.pos().turn()))
        {
          return game.pos().turn() == White ? -1 : 1;
        }

        if (bb::pos_is_load(game.pos()))
        {
          return egdb_lookup(game.pos());
        }
      }

      return 0;
    }
};

// Plays at most max_moves random moves starting in position pos.
// Returns 1 if the end position is winning for white, -1 if the end position
// is winning for black and otherwise 0.
inline
int playout_random(Pos pos, int max_moves, bool verbose)
{
  std::random_device rd;
  std::mt19937 mt(rd());

  // returns a random integer in the range [0, ..., n)
  auto random_int = [&mt](int n)
  {
    std::uniform_int_distribution<int> dist(0, n - 1);
    return dist(mt);
  };

  if (verbose)
  {
    pos::disp(pos);
  }

  if (!can_move(pos, pos.turn()))
  {
    return pos.turn() == White ? -1 : 1;
  }

  if (bb::pos_is_load(pos))
  {
    return egdb_lookup(pos);
  }

  for (int i = 0; i < max_moves; i++)
  {
    List moves;
    gen_moves(moves, pos);
    int k = random_int(moves.size());

    if (verbose)
    {
      std::cout << move::to_string(moves.move(k), pos) << "\n";
    }

    pos = pos.succ(moves.move(k));

    if (!can_move(pos, pos.turn()))
    {
      return pos.turn() == White ? -1 : 1;
    }

    if (bb::pos_is_load(pos))
    {
      return egdb_lookup(pos);
    }
  }

  return 0; // return 0 if the game did not end after max_moves moves
}

// Plays a game using the Scan minimax search
// Returns 1 if the end position is winning for white, -1 if the end position
// is winning for black and otherwise 0.
inline
int playout_minimax(const Pos& pos, Depth depth = Depth_Max, double max_time = 1.0, int max_moves = 100, int64 max_nodes = 1E12, bool verbose = false)
{
  ScanPlayer player;
  return player.play(pos, depth, max_time, max_moves, max_nodes, verbose);
}

// Plays all forced moves and returns the resulting position
inline
Pos play_forced_moves(Pos pos)
{
  List moves;
  gen_moves(moves, pos);
  while (moves.size() == 1)
  {
    pos = pos.succ(moves.move(0));
    gen_moves(moves, pos);
  }
  return pos;
}

// Does a minimax search with the given maximal depth and maximal time.
// Forced moves are played and do not attribute to the depth.
// Returns the score and the best move.
inline
std::pair<int, Move> scan_search(Pos pos, int max_depth, double max_time)
{
  // if there are no possible moves, determine the result directly
  if (!can_move(pos, pos.turn()))
  {
    Move best_move = move::None;
    Score best_score = pos.turn() == White ? -score::Inf : score::Inf;
    return {best_score, best_move};
  }

  Search_Input si;
  si.move = true;
  si.book = false;
  si.depth = max_depth;
  si.nodes = 1E12; // fixed
  si.time = max_time;
  si.input = true;
  si.output = Output_None;

  G_TT.clear(); // clear the transposition table

  Node node(pos);

  Search_Output so;
  search(so, node, si);

  // if there was only one possible move, the search does not provide a score
  if (so.score == score::None)
  {
    Move best_move = so.move;
    pos = play_forced_moves(pos);
    auto [score, move] = scan_search(pos, max_depth, max_time);
    return {score, best_move};
  }

  return {so.score, so.move};
}

// The return value ranges between 0 (black wins) and 1 (white wins).
inline
double normalize_eval(double x)
{
  if (x > 0)
  {
    return 1;
  }
  else if (x < 0)
  {
    return 0;
  }
  else
  {
    return 0.5;
  }
}

// Returns the white piece count minus the black piece count, where a king counts for 3 pieces.
inline
int piece_count_eval(const Pos& pos)
{
  int wm = bit::count(pos.wm());
  int bm = bit::count(pos.bm());
  int wk = bit::count(pos.wk());
  int bk = bit::count(pos.bk());
  return wm + 3*wk - bm - 3*bk;
}

// Uses a naive approach to obtain a win/draw/loss result for a given position.
// All forced moves are played, and then a piece count is done to determine the result.
// Returns 1, 0, -1 for win/draw/loss.
inline
double naive_rollout(const Pos& pos)
{
  return normalize_eval(piece_count_eval(play_forced_moves(pos)));
}

} // namespace draughts

#endif // DRAUGHTS_SCAN_H
