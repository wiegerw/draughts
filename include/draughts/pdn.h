// Copyright: Wieger Wesselink 2022
// Distributed under the Distributed under the GPL-3.0 Software License.
// (See accompanying file license.txt or copy at https://www.gnu.org/licenses/gpl-3.0.txt)
//
/// \file draughts/scan.h
/// \brief add your file description here.

#ifndef DRAUGHTS_PDN_H
#define DRAUGHTS_PDN_H

#include "draughts/scan.h"
#include "scan/move.hpp"
#include "scan/pos.hpp"
#include <fstream>
#include <regex>
#include <set>
#include <string>
#include <vector>

namespace draughts {

inline
std::string read_text(const std::string& filename)
{
  std::ifstream in(filename, std::ios::binary );
  in.seekg(0, std::ios::end);
  std::string text(in.tellg(), 0);
  in.seekg(0);
  in.read(text.data(), text.size());
  return text;
}

// trim from start (in place)
inline void ltrim(std::string& s)
{
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
    return !std::isspace(ch);
  }));
}

// trim from end (in place)
inline void rtrim(std::string& s)
{
  s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
    return !std::isspace(ch);
  }).base(), s.end());
}

// trim from both ends (in place)
inline void trim(std::string& s)
{
  ltrim(s);
  rtrim(s);
}

inline
std::vector<std::string> regex_split(const std::string& text, const std::string& sep)
{
  std::vector<std::string> result;
  std::regex re(sep);
  std::sregex_token_iterator i(text.begin(), text.end(), re, -1);
  std::sregex_token_iterator end;
  while (i != end)
  {
    std::string word = i->str();
    trim(word);
    if (!word.empty())
    {
      result.push_back(word);
    }
    ++i;
  }
  return result;
}

inline
std::vector<std::string> split_paragraphs(const std::string& text, const std::string& separator = "\n\n")
{
  std::vector<std::string> result;
  std::string::size_type first = 0;
  std::string::size_type last = text.size();

  while (true)
  {
    std::string::size_type next = text.find(separator, first);
    if (next == std::string::npos)
    {
      if (first < last)
      {
        result.emplace_back(text.begin() + first, text.begin() + last);
      }
      break;
    }
    result.emplace_back(text.begin() + first, text.begin() + next);
    first = next + separator.size();
  }
  return result;
}

struct pdn_game
{
  std::string white;
  std::string black;
  std::string event;
  std::string date;
  std::string result;
  std::vector<Move> moves;

  public:
    pdn_game() = default;

    pdn_game(const std::vector<Move>& moves_,
             const std::string& white_,
             const std::string& black_,
             const std::string& event_,
             const std::string& date_,
             const std::string& result_
             )
      : white(white_), black(black_), event(event_), date(date_), result(result_), moves(moves_)
    {}
};

class pdn_parser
{
  protected:
    std::vector<unsigned int> m_fields; // used by parse_fields
    std::ostringstream m_out; // used by parse_fields

    unsigned int rc2f(unsigned int r, unsigned int c)
    {
      const unsigned int columns = 10;
      return 1 + (r * (columns / 2)) + (c / 2);
    }

    unsigned int f2r(unsigned int f)
    {
      const unsigned int columns = 10;
      f = f - 1;
      return (f * 2) / columns;
    }

    unsigned int f2c(unsigned int f)
    {
      const unsigned int columns = 10;
      f = f - 1;
      unsigned int d = columns / 2;
      return 1 - ((f / d) % 2) + (2 * (f % d));
    }

    // find a non-empty field between f1 and f2
    unsigned int find_between_field(unsigned int f1, unsigned int f2, const Pos& pos)
    {
      unsigned int c1 = f2c(f1);
      unsigned int r1 = f2r(f1);
      unsigned int c2 = f2c(f2);
      unsigned int r2 = f2r(f2);
      unsigned int dr = (r2 > r1) ? 1 : -1;
      unsigned int dc = (c2 > c1) ? 1 : -1;

      unsigned int r = r1 + dr;
      unsigned int c = c1 + dc;
      while (r != r2)
      {
        unsigned int f = rc2f(r, c);
        if (!pos.is_empty_(f))
        {
          return f;
        }
        r = r + dr;
        c = c + dc;
      }
      throw std::runtime_error("Could not find a non-empty field");
    }

    // Reads an integer from the range [first, last)
    // Returns the position in the range after the integer
    // Precondition: (first != last) && is_digit(*first)
    template <typename Iterator>
    Iterator parse_natural_number(Iterator first, Iterator last, unsigned int& result)
    {
      auto is_digit = [&](Iterator i)
      {
        return '0' <= *i && *i <= '9';
      };

      Iterator i = first;
      result = *i - '0';
      ++i;

      while (i != last && is_digit(i))
      {
        result *= 10;
        result += *i - '0';
        ++i;
      }
      return i;
    }

    void parse_fields(const std::string& text, std::vector<unsigned int>& result)
    {
      auto is_separator = [&](std::string::const_iterator i)
      {
        return *i == '-' || *i == 'x';
      };

      result.clear();
      auto first = text.begin();
      auto last = text.end();
      while (first != last)
      {
        unsigned int n;
        first = parse_natural_number(first, last, n);
        m_fields.push_back(n);
        if (first != last && is_separator(first))
        {
          ++first;
        }
      }
    }

    // convert a PDN move into Scan format
    inline
    std::string convert_pdn_move(const std::string& text, const Pos& pos)
    {
      m_out.str("");
      bool is_capture = text.find('x') != std::string::npos;
      char sep = is_capture ? 'x' : '-';
      parse_fields(text, m_fields);
      m_out << m_fields.front() << sep << m_fields.back();
      if (m_fields.size() > 2) // ambiguous move
      {
        for (auto i = 0; i < m_fields.size() - 1; i++)
        {
          auto f = find_between_field(m_fields[i], m_fields[i + 1], pos);
          m_out << sep << f;
        }
      }
      return m_out.str();
    }

    template <typename Iterator>
    std::pair<Iterator, Iterator> parse_next_move(Iterator first, Iterator last)
    {
      auto is_digit = [&]()
      {
        return '0' <= *first && *first <= '9';
      };

      auto is_separator = [&]()
      {
        return *first == '-' || *first == 'x';
      };

      auto is_move = [&]()
      {
        return is_digit() || is_separator();
      };

      auto find_digit = [&]()
      {
        while (first != last && !is_digit())
        {
          ++first;
        }
      };

      std::pair<Iterator, Iterator> result;

      // find the beginning of a move
      find_digit();

      // no digit found, return empty move
      if (first == last)
      {
        return {first, first};
      }

      result.first = first;
      ++first;
      while (first != last && is_move())
      {
        ++first;
      }

      // it was a move number, so return empty move
      if (first != last && *first == '.')
      {
        ++first;
        return {first, first};
      }

      result.second = first;
      return result;
    }

    void parse_tag(const std::string& text, const std::string& tag, std::string& result)
    {
      auto pos = text.find(tag);
      if (pos == std::string::npos)
      {
        return;
      }

      auto pos1 = text.find('"', pos);
      if (pos1 == std::string::npos)
      {
        return;
      }

      auto pos2 = text.find('"', pos1 + 1);
      if (pos2 == std::string::npos)
      {
        return;
      }

      result = std::string(text.begin() + pos1 + 1, text.begin() + pos2);
    }

  public:
    inline
    Move parse_pdn_move(const std::string& text, const Pos& pos)
    {
      return move::from_string(convert_pdn_move(text, pos), pos);
    }

    inline
    std::vector<Move> parse_pdn_moves(const std::string& text)
    {
      std::vector<Move> result;

      Pos pos = draughts::start_position();

      for (const std::string& move_text: regex_split(text, "\\s"))
      {
        if (move_text.empty())
        {
          continue;
        }
        Move m = move::from_string(convert_pdn_move(text, pos), pos);
        result.push_back(m);
        pos = pos.succ(m);
      }
      return result;
    }

    pdn_game parse_pdn_game(const std::string& text)
    {
      pdn_game game;
      Pos position = draughts::start_position();

      // parse PDN tags
      parse_tag(text, "Event", game.event);
      parse_tag(text, "Date", game.date);
      parse_tag(text, "White", game.white);
      parse_tag(text, "Black", game.black);
      parse_tag(text, "Result", game.result);

      auto pos = text.find_last_of(']');
      if (pos == std::string::npos)
      {
        return game;
      }
      auto first = text.begin() + pos + 1;

      pos = text.find('*', pos + 1);
      auto last = (pos == std::string::npos) ? text.end() : text.begin() + pos;

      while (first != last)
      {
        auto [i1, i2] = parse_next_move(first, last);
        if (i1 != i2)
        {
          std::string move_text(i1, i2);
          if (move_text.empty())
          {
            continue;
          }
          std::string scan_move = convert_pdn_move(move_text, position);
          Move m = move::from_string(scan_move, position);
          game.moves.push_back(m);
          position = position.succ(m);
        }
        first = i2;
      }

      return game;
    }

    std::vector<pdn_game> parse_pdn_file(const std::string& filename)
    {
      std::string text = read_text(filename);
      std::vector<pdn_game> result;
      std::size_t i = 0;
      for (const std::string& paragraph: split_paragraphs(text))
      {
        result.emplace_back(parse_pdn_game(paragraph));
        i++;
        if (i % 1000 == 0)
        {
          std::cout << "." << std::flush;
        }
      }
      std::cout << "\nRead " << i << " games" << std::endl;
      return result;
    };
};

inline
Move parse_pdn_move(const std::string& text, const Pos& pos)
{
  pdn_parser parser;
  return parser.parse_pdn_move(text, pos);
}

inline
std::vector<Move> parse_pdn_moves(const std::string& text)
{
  pdn_parser parser;
  return parser.parse_pdn_moves(text);
}

pdn_game parse_pdn_game(const std::string& text)
{
  pdn_parser parser;
  return parser.parse_pdn_game(text);
}

std::vector<pdn_game> parse_pdn_file(const std::string& filename)
{
  pdn_parser parser;
  return parser.parse_pdn_file(filename);
}

} // namespace draughts

#endif // DRAUGHTS_PDN_H
