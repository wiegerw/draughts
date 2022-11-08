
#ifndef POS_HPP
#define POS_HPP

// includes

#include <array>
#include <bitset>

#include "scan/bit.hpp"
#include "scan/common.hpp"
#include "scan/gen.hpp" // for can_capture
#include "scan/libmy.hpp"
#include "draughts/utilities.h"

// types

class Pos {

private:

   std::array<Bit, Piece_Size> m_piece;
   std::array<Bit, Side_Size> m_side;
   Bit m_all;
   Side m_turn;

   int m_wolf[Side_Size];
   int m_count[Side_Size];

public:

   Pos () = default;
   Pos (Side turn, Bit wm, Bit bm, Bit wk, Bit bk);

   friend bool operator == (const Pos & p0, const Pos & p1);

   Pos succ (Move mv) const;

   Side turn () const { return m_turn; }

   Bit all   () const { return m_all; }
   Bit empty () const { return bit::Squares ^ all(); }

   Bit piece (Piece pc) const { return m_piece[pc]; }
   Bit side  (Side sd)  const { return m_side[sd]; }

   Bit piece_side (Piece pc, Side sd) const { return piece(pc) & side(sd); }

   Bit man  () const { return piece(Man); }
   Bit king () const { return piece(King); }

   Bit man  (Side sd) const { return piece_side(Man, sd); }
   Bit king (Side sd) const { return piece_side(King, sd); }

   Bit white () const { return side(White); }
   Bit black () const { return side(Black); }

   Bit wm () const { return man(White); }
   Bit bm () const { return man(Black); }
   Bit wk () const { return king(White); }
   Bit bk () const { return king(Black); }

   bool is_empty (Square sq)           const { return bit::has(empty(),   sq); }
   bool is_piece (Square sq, Piece pc) const { return bit::has(piece(pc), sq); }
   bool is_side  (Square sq, Side sd)  const { return bit::has(side(sd),  sq); }

   Square wolf  (Side sd) const { return square_make(m_wolf[sd]); }
   int    count (Side sd) const { return m_count[sd]; }

    bool is_empty_(int f) const
    {
      assert(1 <= f && f <= 50);
      Square sq = square_from_std(f);
      return is_empty(sq);
    }

    bool is_king(int f) const
    {
      assert(1 <= f && f <= 50);
      Square sq = square_from_std(f);
      return bit::has(m_piece[King], sq);
    }

    bool is_white(int f) const
    {
      assert(1 <= f && f <= 50);
      Square sq = square_from_std(f);
      return bit::has(m_side[Side::White], sq);
    }

    bool is_black(int f) const
    {
      assert(1 <= f && f <= 50);
      Square sq = square_from_std(f);
      return bit::has(m_side[Side::Black], sq);
    }

    constexpr bool is_white_to_move() const
    {
      return m_turn == Side::White;
    }

    bool opponent_has_no_pieces() const
    {
      Side side = is_white_to_move() ? Side::Black : Side::White;
      return m_side[side] == Pos().m_side[side];
    }

    void info() const
    {
      std::cout << std::bitset<64>(m_side[Side::White]) << " "
                << std::bitset<64>(m_side[Side::Black]) << " "
                << std::bitset<64>(m_piece[Piece::Man]) << " "
                << std::bitset<64>(m_piece[Piece::King])
                << std::endl;
    }

    void put_piece(int f, bool is_white, bool is_king)
    {
      assert(1 <= f && f <= 50);
      Square sq = square_from_std(f);
      bit::set(m_side[is_white ? Side::White : Side::Black], sq);
      bit::clear(m_side[is_white ? Side::Black : Side::White], sq);
      bit::set(m_piece[is_king ? Piece::King : Piece::Man], sq);
      bit::clear(m_piece[is_king ? Piece::Man : Piece::King], sq);
      m_all = m_piece[Piece::Man] ^ m_piece[Piece::King];
    }

    void flip()
    {
      auto flip_bit = [](Bit& x) { x = Bit(draughts::reverse(x) >> 1); };
      flip_bit(m_piece[Piece::Man]);
      flip_bit(m_piece[Piece::King]);
      flip_bit(m_side[Side::White]);
      flip_bit(m_side[Side::Black]);
      std::swap(m_side[Side::White], m_side[Side::Black]);
      flip_bit(m_all);
      m_turn = (m_turn == Side::White ? Side::Black : Side::White);
      for (int sd = 0; sd < Side_Size; sd++) {
        m_wolf[sd] = -1;
        m_count[sd] = 0;
      }
    }

    Pos flipped() const
    {
      Pos result = *this;
      result.flip();
      return result;
    }

    const std::array<Bit, Piece_Size>& piece_array() const
    {
      return m_piece;
    }

    const std::array<Bit, Side_Size>& side_array() const
    {
      return m_side;
    }

  private:

   Pos (Bit man, Bit king, Bit white, Bit black, Bit all, Side turn);
};

bool operator == (const Pos & p0, const Pos & p1);

class Node {

private:

   Pos m_pos;
   int m_ply;
   const Node * m_parent;

public:

   Node () = default;
   explicit Node (const Pos & pos);

   operator const Pos & () const { return m_pos; }

   Node succ (Move mv) const;

   bool is_end  ()        const;
   bool is_draw (int rep) const;

private:

   Node (const Pos & pos, int ply, const Node * parent);
};

namespace pos { // ###

// variables

extern Pos Start;

// functions

void init ();

bool is_end  (const Pos & pos);
bool is_wipe (const Pos & pos);
int  result  (const Pos & pos, Side sd);

inline bool is_capture (const Pos & pos) { return can_capture(pos, pos.turn()); }
inline bool is_threat  (const Pos & pos) { return can_capture(pos, side_opp(pos.turn())); }

inline int size (const Pos & pos) { return bit::count(pos.all()); }

inline bool has_king (const Pos & pos)          { return pos.king()   != 0; }
inline bool has_king (const Pos & pos, Side sd) { return pos.king(sd) != 0; }

Piece_Side piece_side (const Pos & pos, Square sq);

int tempo (const Pos & pos);
int skew  (const Pos & pos, Side sd);

inline int    stage (const Pos & pos) { return Stage_Size - tempo(pos); }
inline double phase (const Pos & pos) { return double(stage(pos)) / double(Stage_Size); }

void disp (const Pos & pos);

} // namespace pos

#endif // !defined POS_HPP

