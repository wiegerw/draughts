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
  std::ifstream in(filename.c_str());
  if (!in)
  {
    throw std::runtime_error("Could not open input file: " + filename);
  }
  in.unsetf(std::ios::skipws); //  Turn of white space skipping on the stream

  std::string text;
  std::copy(std::istream_iterator<char>(in), std::istream_iterator<char>(), std::back_inserter(text));
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
std::vector<std::string> split_paragraphs(const std::string& text)
{
  std::vector<std::string> result;

  // find multiple line endings
  std::regex sep(R"(\n\s*\n)");

  // the -1 below directs the token iterator to display the parts of
  // the string that did NOT match the regular expression.
  std::sregex_token_iterator cur(text.begin(), text.end(), sep, -1);
  std::sregex_token_iterator end;

  for (; cur != end; ++cur)
  {
    std::string paragraph = *cur;
    trim(paragraph);
    if (!paragraph.empty())
    {
      result.push_back(paragraph);
    }
  }
  return result;
}

inline
unsigned int rc2f(unsigned int r, unsigned int c)
{
  const unsigned int columns = 10;
  return 1 + (r * (columns / 2)) + (c / 2);
}

inline
unsigned int f2r(unsigned int f)
{
  const unsigned int columns = 10;
  f = f - 1;
  return (f * 2) / columns;
}

inline
unsigned int f2c(unsigned int f)
{
  const unsigned int columns = 10;
  f = f - 1;
  unsigned int d = columns / 2;
  return 1 - ((f / d) % 2) + (2 * (f % d));
}

// find a non-empty field between f1 and f2
inline
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

inline
std::vector<unsigned int> parse_fields(const std::string& text)
{
  std::vector<unsigned int> result;
  for (const std::string& f: regex_split(text, "[-x]"))
  {
    result.push_back(std::stoi(f));
  }
  return result;
}

// convert a PDN move into Scan format
inline
std::string convert_pdn_move(const std::string& text, const Pos& pos)
{
  bool is_capture = text.find('x') != std::string::npos;
  char sep = is_capture ? 'x' : '-';
  std::vector<unsigned int> fields = parse_fields(text);
  std::string scan_text = std::to_string(fields.front()) + sep + std::to_string(fields.back());
  if (fields.size() > 2) // ambiguous move
  {
    for (auto i = 0; i < fields.size() - 1; i++)
    {
      auto f = find_between_field(fields[i], fields[i + 1], pos);
      scan_text = scan_text + sep + std::to_string(f);
    }
  }
  return scan_text;
}

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
    Move m = parse_pdn_move(move_text, pos);
    result.push_back(m);
    pos = pos.succ(m);
  }
  return result;
}

class pdn_game
{
  private:
    std::vector<Move> m_moves;

  public:
    pdn_game() = default;

    pdn_game(const std::vector<Move>& moves)
     : m_moves(moves)
    {}

    const std::vector<Move>& moves() const
    {
      return m_moves;
    }
};

inline
pdn_game parse_pdn_game(const std::string& text)
{
  auto pos = text.find_last_of(']');
  std::string move_text = (pos == std::string::npos) ? text : text.substr(pos + 1);
  trim(move_text);

  // remove the result
  move_text = std::regex_replace(move_text, std::regex("((2-0)|(1-1)|(0-2)|\\*)$"), "");

  // remove the move numbers
  move_text = std::regex_replace(move_text, std::regex("\\d*\\."), "");

  auto moves = parse_pdn_moves(move_text);
  return pdn_game(moves);
}

inline
std::vector<pdn_game> parse_pdn_file(const std::string& filename)
{
  std::string text = read_text(filename);
  std::vector<pdn_game> result;
  for (const std::string& paragraph: split_paragraphs(text))
  {
    result.push_back(parse_pdn_game(paragraph));
  }
  return result;
};

} // namespace draughts

#endif // DRAUGHTS_PDN_H
