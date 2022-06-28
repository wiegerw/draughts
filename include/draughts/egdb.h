// Copyright: Wieger Wesselink 2022
// Distributed under the Distributed under the GPL-3.0 Software License.
// (See accompanying file license.txt or copy at https://www.gnu.org/licenses/gpl-3.0.txt)
//
/// \file draughts/egdb.h
/// \brief add your file description here.

#ifndef DRAUGHTS_EGDB_H
#define DRAUGHTS_EGDB_H

#include "scan/pos.hpp"
#include <iterator>
#include <numeric>
#include <tuple>
#include <vector>

namespace draughts {

// Enumerates non-capture positions in an endgame database slice nw, nb, nW, nB.
// See https://mdgsoft.home.xs4all.nl/draughts/stats/index.html for full statistics.
// N.B. Note that this page uses the order nw, nW, nb, nB for the slices.
class egdb_enumerator
{
  private:
    using container_iterator = std::vector<std::vector<int>>::const_iterator;
    using sequence_iterator = std::vector<int>::const_iterator;
    using out_iterator = std::vector<int>::iterator;

    std::vector<std::vector<int>> m_squares; // the possible squares for each piece
    std::vector<int> m_position;             // the squares of the current position
    std::vector<Piece_Side> m_pieces;        // the pieces of the slice
    Pos m_pos;                               // the current position

    container_iterator last;
    std::vector<std::tuple<container_iterator, out_iterator, sequence_iterator>> stack;

    // @pre: first != last
    bool find_next()
    {
      auto [first, i, j] = stack.back();
      stack.pop_back();
      *i = *j;
      ++first;
      ++i;
      if (first == last)
      {
        return true;
      }
      for (auto k = first->begin(); k != first->end(); ++k)
      {
        stack.emplace_back(first, i, k);
      }
      return false;
    }

    void add_squares(int min, int max)
    {
      std::vector<int> squares;
      squares.reserve(max - min + 1);
      for (int f = max; f >= min; f--)
      {
        squares.push_back(f);
      }
      m_squares.push_back(squares);
    }

    bool make_position()
    {
      m_pos = Pos(); // clear the position
      for (unsigned int i = 0; i < m_pieces.size(); i++)
      {
        int f = m_position[i];
        Piece_Side p = m_pieces[i];
        if (!m_pos.is_empty_(f))
        {
          return false;
        }
        bool is_white = p == White_Man || p == White_King;
        bool is_king = p == White_King || p == Black_King;
        m_pos.put_piece(f, is_white, is_king);
      }
      return !pos::is_capture(m_pos);
    }

  public:
    egdb_enumerator(int nw, int nb, int nW, int nB)
     : m_position(nw + nb + nW + nB)
    {
      for (auto i = 0; i < nw; i++)
      {
        m_pieces.push_back(White_Man);
        add_squares(6, 50);
      }
      for (auto i = 0; i < nb; i++)
      {
        m_pieces.push_back(Black_Man);
        add_squares(1, 45);
      }
      for (auto i = 0; i < nW; i++)
      {
        m_pieces.push_back(White_King);
        add_squares(1, 50);
      }
      for (auto i = 0; i < nB; i++)
      {
        m_pieces.push_back(Black_King);
        add_squares(1, 50);
      }

      last = m_squares.end();
      auto first = m_squares.begin();
      if (first != last)
      {
        for (auto k = first->begin(); k != first->end(); ++k)
        {
          stack.emplace_back(first, m_position.begin(), k);
        }
      }
    }

    // Advances the sequence to the next solution. Returns false if there is no next solution.
    bool next()
    {
      while (!stack.empty())
      {
        if (find_next() && make_position())
        {
          return true;
        }
      }
      return false;
    }

    std::vector<int> squares() const
    {
      return m_position;
    }

    const Pos& position() const
    {
      return m_pos;
    }
};

} // namespace draughts

#endif // DRAUGHTS_EGDB_H
