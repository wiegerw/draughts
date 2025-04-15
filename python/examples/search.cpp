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
    "   .   .   x   x   . "
    " x   x   x   x   .   "
    "   x   .   x   x   o "
    " x   o   o   .   o   "
    "   .   o   o   .   o "
    " .   o   o   o   .   "
    "   .   .   .   .   . "
    " .   .   .   .   .   B";

  Pos pos = draughts::parse_position(position);
  Node node(pos);

  Search_Input si;
  si.move = true;
  si.book = true;
  si.depth = 20;
  si.nodes = 1E12;
  si.time = 5.0;
  si.input = true;
  si.output = Output_Terminal;

  Search_Output so;
  search(so, node, si);

  return 0;
}