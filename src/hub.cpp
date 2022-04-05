
// includes

#include <cctype>
#include <iostream>
#include <sstream>
#include <string>

#include "scan/fen.hpp"
#include "scan/game.hpp"
#include "scan/hub.hpp"
#include "scan/libmy.hpp"
#include "scan/terminal.hpp"
#include "scan/search.hpp"
#include "scan/thread.hpp" // for get_line
#include "scan/util.hpp"
#include "scan/var.hpp"

namespace hub {

// prototypes

static std::string hub_pair  (const std::string & name, const std::string & value);
static std::string hub_value (const std::string & value);

// functions

void error(const std::string & msg) {
   write(std::string("error ") + hub_pair("message", msg));
}

std::string read() {

   std::string line;
   if (!get_line(line)) std::exit(EXIT_SUCCESS); // EOF

   return line;
}

void write(const std::string & line) {
   std::cout << line << std::endl;
}

void add_pair(std::string & line, const std::string & name, int value) {
   line += " " + hub_pair(name, std::to_string(value));
}

void add_pair(std::string & line, const std::string & name, double value, int /* precision */) { // TODO: use precision
   line += " " + hub_pair(name, std::to_string(value));
}

void add_pair(std::string & line, const std::string & name, const std::string & value) {
   line += " " + hub_pair(name, value);
}

static std::string hub_pair(const std::string & name, const std::string & value) {
   return name + "=" + hub_value(value);
}

static std::string hub_value(const std::string & value) {

   if (Scanner::is_name(value)) {
      return value;
   } else {
      return std::string("\"") + value + "\""; // TODO: protect '"'?
   }
}

Scanner::Scanner(const std::string & s) : m_string{s} {}

std::string Scanner::get_command() {
   return get_name();
}

Pair Scanner::get_pair() {

   std::string name = get_name(); // <name>

   std::string value;

   skip_blank();

   if (peek_char() == '=') { // = <value>
      skip_char();
      value = get_value();
   }

   return {name, value};
}

std::string Scanner::get_name() {

   std::string name;

   skip_blank();

   while (is_id(peek_char())) {
      name += get_char();
   }

   if (name.empty()) throw Bad_Input(); // not a name

   return name;
}

std::string Scanner::get_value() {

   std::string value;

   skip_blank();

   if (peek_char() == '"') { // "<value>"

      skip_char();

      while (peek_char() != '"') {
         if (is_end()) throw Bad_Input(); // missing closing '"'
         value += get_char();
      }

      skip_char(); // closing '"'

   } else { // <value>

      value = get_name();
   }

   return value;
}

bool Scanner::eos() {
   skip_blank();
   return is_end();
}

void Scanner::skip_blank() {
   while (is_blank(peek_char())) {
      skip_char();
   }
}

void Scanner::skip_char() {
   assert(!is_end());
   m_pos++;
}

char Scanner::get_char() {
   assert(!is_end());
   return m_string[m_pos++];
}

char Scanner::peek_char() const {
   return is_end() ? '\0' : m_string[m_pos]; // HACK but makes parsing easier
}

bool Scanner::is_end() const {
   return m_pos == int(m_string.size());
}

bool Scanner::is_name(const std::string & s) {

   for (char c : s) {
      if (!is_id(c)) return false;
   }

   return !s.empty();
}

bool Scanner::is_blank(char c) {
   return std::isspace(c);
}

bool Scanner::is_id(char c) {
   return !std::iscntrl(c) && !is_blank(c) && c != '=' && c != '"'; // excludes '\0'
}

void param_bool(const std::string & name) {

  std::string line = "param";
  hub::add_pair(line, "name", name);
  hub::add_pair(line, "value", var::get(name));
  hub::add_pair(line, "type", "bool");
  hub::write(line);
}

void param_int(const std::string & name, int min, int max) {

  std::string line = "param";
  hub::add_pair(line, "name", name);
  hub::add_pair(line, "value", var::get(name));
  hub::add_pair(line, "type", "int");
  hub::add_pair(line, "min", std::to_string(min));
  hub::add_pair(line, "max", std::to_string(max));
  hub::write(line);
}

void param_enum(const std::string & name, const std::string & values) {

  std::string line = "param";
  hub::add_pair(line, "name", name);
  hub::add_pair(line, "value", var::get(name));
  hub::add_pair(line, "type", "enum");
  hub::add_pair(line, "values", values);
  hub::write(line);
}

void hub_loop() {

  Game game;
  Search_Input si;

  while (true) {

    std::string line = hub::read();
    hub::Scanner scan(line);

    if (scan.eos()) { // empty line
      hub::error("missing command");
      continue;
    }

    std::string command = scan.get_command();

    if (false) {

    } else if (command == "go") {

      bool think = false; // ignored
      bool ponder = false;
      bool analyze = false;

      while (!scan.eos()) {

        auto p = scan.get_pair();

        if (false) {
        } else if (p.name == "think") {
          think = true;
        } else if (p.name == "ponder") {
          ponder = true;
        } else if (p.name == "analyze") {
          analyze = true;
        }
      }

      si.move = !analyze;
      si.book = !analyze;
      si.input = true;
      si.output = Output_Hub;
      si.ponder = ponder;

      Search_Output so;
      search(so, game.node(), si);

      Move move = so.move;
      Move answer = so.answer;

      if (move == move::None) move = quick_move(game.node());

      if (move != move::None && answer == move::None) {
        answer = quick_move(game.node().succ(move));
      }

      Pos p0 = game.pos();
      Pos p1 = p0;
      if (move != move::None) p1 = p0.succ(move);

      std::string line = "done";
      if (move   != move::None) hub::add_pair(line, "move",   move::to_hub(move, p0));
      if (answer != move::None) hub::add_pair(line, "ponder", move::to_hub(answer, p1));
      hub::write(line);

      si.init(); // reset level

    } else if (command == "hub") {

      std::string line = "id";
      hub::add_pair(line, "name", Engine_Name);
      hub::add_pair(line, "version", Engine_Version);
      hub::add_pair(line, "author", "Fabien Letouzey");
      hub::add_pair(line, "country", "France");
      hub::write(line);

      param_enum("variant", "normal killer bt frisian losing");
      param_bool("book");
      param_int ("book-ply", 0, 20);
      param_int ("book-margin", 0, 100);
      param_bool("ponder");
      param_int ("threads", 1, 16);
      param_int ("tt-size", 16, 30);
      param_int ("bb-size", 0, 7);

      hub::write("wait");

    } else if (command == "init") {

      init_low();
      hub::write("ready");

    } else if (command == "level") {

      int depth = -1;
      int64 nodes = -1;
      double move_time = -1.0;

      bool smart = false;
      int moves = 0;
      double game_time = 30.0;
      double inc = 0.0;

      bool infinite = false; // ignored
      bool ponder = false; // ignored

      while (!scan.eos()) {

        auto p = scan.get_pair();

        if (false) {
        } else if (p.name == "depth") {
          depth = std::stoi(p.value);
        } else if (p.name == "nodes") {
          nodes = std::stoll(p.value);
        } else if (p.name == "move-time") {
          move_time = std::stod(p.value);
        } else if (p.name == "moves") {
          smart = true;
          moves = std::stoi(p.value);
        } else if (p.name == "time") {
          smart = true;
          game_time = std::stod(p.value);
        } else if (p.name == "inc") {
          smart = true;
          inc = std::stod(p.value);
        } else if (p.name == "infinite") {
          infinite = true;
        } else if (p.name == "ponder") {
          ponder = true;
        }
      }

      if (depth >= 0) si.depth = Depth(depth);
      if (nodes >= 0) si.nodes = nodes;
      if (move_time >= 0.0) si.time = move_time;

      if (smart) si.set_time(moves, game_time, inc);

    } else if (command == "new-game") {

      G_TT.clear();

    } else if (command == "ping") {

      hub::write("pong");

    } else if (command == "ponder-hit") {

      // no-op (handled during search)

    } else if (command == "pos") {

      std::string pos = pos_hub(pos::Start);
      std::string moves;

      while (!scan.eos()) {

        auto p = scan.get_pair();

        if (false) {
        } else if (p.name == "start") {
          pos = pos_hub(pos::Start);
        } else if (p.name == "pos") {
          pos = p.value;
        } else if (p.name == "moves") {
          moves = p.value;
        }
      }

      // position

      try {
        game.init(pos_from_hub(pos));
      } catch (const Bad_Input &) {
        hub::error("bad position");
        continue;
      }

      // moves

      std::stringstream ss(moves);

      std::string arg;

      while (ss >> arg) {

        try {

          Move mv = move::from_hub(arg, game.pos());

          if (!move::is_legal(mv, game.pos())) {
            hub::error("illegal move");
            break;
          } else {
            game.add_move(mv);
          }

        } catch (const Bad_Input &) {

          hub::error("bad move");
          break;
        }
      }

      si.init(); // reset level

    } else if (command == "quit") {

      std::exit(EXIT_SUCCESS);

    } else if (command == "set-param") {

      std::string name;
      std::string value;

      while (!scan.eos()) {

        auto p = scan.get_pair();

        if (false) {
        } else if (p.name == "name") {
          name = p.value;
        } else if (p.name == "value") {
          value = p.value;
        }
      }

      if (name.empty()) {
        hub::error("missing name");
        continue;
      }

      var::set(name, value);
      var::update();

    } else if (command == "stop") {

      // no-op (handled during search)

    } else { // unknown command

      hub::error("bad command");
      continue;
    }
  }
}

} // namespace hub

