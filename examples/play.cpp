#include "scan/bb_base.hpp"
#include "scan/bb_comp.hpp"
#include "scan/book.hpp"
#include "scan/eval.hpp"
#include "scan/fen.hpp"
#include "scan/game.hpp"
#include "scan/hash.hpp"
#include "scan/list.hpp"
#include "scan/terminal.hpp"
#include "scan/move.hpp"
#include "scan/pos.hpp"
#include "scan/search.hpp"
#include "scan/thread.hpp"
#include "scan/var.hpp"
#include "draughts/pdn.h"
#include "draughts/scan.h"

int main()
{
  bit::init();
  hash::init();
  pos::init();
  var::init();
  bb::index_init();
  bb::comp_init();
  ml::rand_init(); // after hash keys
  var::load("scan.ini");
  init_high();

  std::string position =
    "   .   .   .   .   . "
    " .   .   .   .   .   "
    "   .   x   x   x   . "
    " x   .   x   x   .   "
    "   x   .   x   x   o "
    " x   o   o   .   o   "
    "   .   o   o   o   o "
    " .   o   o   .   .   "
    "   .   .   .   .   . "
    " .   .   .   .   .   W";

  Pos pos = draughts::parse_position(position);

  Depth max_depth = 20;
  double max_time = 5.0;
  int max_moves = 100;
  int64 max_nodes = 1E12;
  bool verbose = true;

  int result_minimax = draughts::playout_minimax(pos, max_depth, max_time, max_moves, max_nodes, verbose);
  std::cout << "result_minimax = " << result_minimax << std::endl;

  int result_random = draughts::playout_random(pos, max_moves, verbose);
  std::cout << "result_random = " << result_random << std::endl;

  return 0;
}
