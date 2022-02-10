#include <sstream>
#include "bb_base.hpp"
#include "book.hpp"
#include "eval.hpp"
#include "fen.hpp"
#include "game.hpp"
#include "list.hpp"
#include "terminal.hpp"
#include "move.hpp"
#include "pos.hpp"
#include "search.hpp"
#include "thread.hpp"
#include "var.hpp"

void disp_game(const Game & game) {

  Pos pos = game.start_pos();

  for (int i = 0; i < game.ply(); i++) {

    Move mv = game.move(i);

    if (i != 0) std::cout << " ";
    std::cout << move::to_string(mv, pos);

    pos = pos.succ(mv);
  }

  std::cout << '\n';
  std::cout << std::endl;
}

void Terminal::loop() {

  m_computer[White] = false;
  m_computer[Black] = true;

  m_depth = Depth_Max;
  m_nodes = 1E12;
  m_time = Time;

  Game & game = m_game;
  game.clear();

  new_game();

  while (true) {

    bool computer = m_computer[game.turn()];

    Move mv;

    if (computer && !m_game.is_end()) {

      Search_Input si;
      si.move = true;
      si.book = true;
      si.depth = m_depth;
      si.nodes = m_nodes;
      si.time = m_time;
      si.input = true;
      si.output = Output_Terminal;

      Search_Output so;
      search(so, game.node(), si);

      mv = so.move;
      if (mv == move::None) mv = quick_move(game.node());

    } else {

      mv = user_move();
      if (mv == move::None) continue; // assume command
    }

    std::string move_string = move::to_string(mv, game.pos());

    game.add_move(mv);
    pos::disp(game.pos());

    if (computer) {
      std::cout << "I play " << move_string << '\n';
      std::cout << std::endl;
    }
  }
}

Move Terminal::user_move() {

  std::cout << "> " << std::flush;

  std::string line;
  if (!get_line(line)) std::exit(EXIT_SUCCESS); // EOF

  std::cout << std::endl;

  std::stringstream ss(line);

  std::string command;
  ss >> command;

  if (false) {

  } else if (command.empty() && !m_game.is_end()) { // forced move?

    List list;
    gen_moves(list, m_game.pos());
    if (list.size() == 1) return list[0];

  } else if (command == "0") {

    m_computer[White] = false;
    m_computer[Black] = false;

  } else if (command == "1") {

    m_computer[m_game.turn()] = false;
    m_computer[side_opp(m_game.turn())] = true;

  } else if (command == "2") {

    m_computer[White] = true;
    m_computer[Black] = true;

  } else if (command == "b") {

    pos::disp(m_game.pos());

  } else if (command == "depth") {

    std::string arg;
    ss >> arg;

    m_depth = Depth(std::stoi(arg));

  } else if (command == "fen") {

    std::string arg;
    ss >> arg;

    if (arg.empty()) {

      std::cout << pos_fen(m_game.pos()) << '\n';
      std::cout << std::endl;

    } else {

      try {
        new_game(pos_from_fen(arg));
      } catch (const Bad_Input &) {
        std::cout << "bad FEN\n";
        std::cout << std::endl;
      }
    }

  } else if (command == "g") {

    m_computer[m_game.turn()] = true;
    m_computer[side_opp(m_game.turn())] = false;

  } else if (command == "game") {

    disp_game(m_game);

  } else if (command == "h") {

    std::cout << "(0) human players\n";
    std::cout << "(1) human vs. computer\n";
    std::cout << "(2) computer players\n";
    std::cout << "(b)oard\n";
    std::cout << "(g)o\n";
    std::cout << "(h)elp\n";
    std::cout << "(n)ew game\n";
    std::cout << "(q)uit\n";
    std::cout << "(r)edo\n";
    std::cout << "(r)edo (a)ll\n";
    std::cout << "(u)ndo\n";
    std::cout << "(u)ndo (a)ll\n";
    std::cout << '\n';

    std::cout << "depth <n>\n";
    std::cout << "fen [<FEN>]\n";
    std::cout << "game\n";
    std::cout << "nodes <n>\n";
    std::cout << "time <seconds per move>\n";
    std::cout << std::endl;

  } else if (command == "n") {

    new_game();

  } else if (command == "nodes") {

    std::string arg;
    ss >> arg;

    m_nodes = std::stoll(arg);

  } else if (command == "q") {

    std::exit(EXIT_SUCCESS);

  } else if (command == "r") {

    go_to(m_game.ply() + 1);

  } else if (command == "ra") {

    go_to(m_game.size());

  } else if (command == "time") {

    std::string arg;
    ss >> arg;

    m_time = std::stod(arg);

  } else if (command == "u") {

    go_to(m_game.ply() - 1);

  } else if (command == "ua") {

    go_to(0);

  } else if (!m_game.is_end()) {

    try {

      Move mv = move::from_string(command, m_game.pos());

      if (!move::is_legal(mv, m_game.pos())) {
        std::cout << "illegal move\n";
        std::cout << std::endl;
        return move::None;
      } else {
        return mv;
      }

    } catch (const Bad_Input &) {

      std::cout << "???\n";
      std::cout << std::endl;
    }
  }

  return move::None;
}

void Terminal::new_game(const Pos & pos) {

  bool opp = m_computer[side_opp(m_game.turn())];

  m_game.init(pos);
  pos::disp(m_game.pos());

  m_computer[m_game.turn()] = false;
  m_computer[side_opp(m_game.turn())] = opp;

  G_TT.clear();
}

void Terminal::go_to(int ply) {

  if (ply >= 0 && ply <= m_game.size() && ply != m_game.ply()) {

    bool opp = m_computer[side_opp(m_game.turn())];

    m_game.go_to(ply);
    pos::disp(m_game.pos());

    m_computer[m_game.turn()] = false;
    m_computer[side_opp(m_game.turn())] = opp;
  }
}

void init_high() {

  std::cout << std::endl;

  init_low();

  std::cout << "done\n";
  std::cout << std::endl;
}

void init_low() {

  bit::init(); // depends on the variant
  if (var::Book) book::init();
  if (var::BB) bb::init();

  eval_init();
  G_TT.set_size(var::TT_Size);
}

void run_terminal_game()
{
  Terminal terminal;
  listen_input();
  init_high();
  terminal.loop();
}
