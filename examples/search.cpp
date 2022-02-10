#include "bb_base.hpp"
#include "bb_comp.hpp"
#include "book.hpp"
#include "eval.hpp"
#include "fen.hpp"
#include "game.hpp"
#include "hash.hpp"
#include "list.hpp"
#include "terminal.hpp"
#include "move.hpp"
#include "pos.hpp"
#include "search.hpp"
#include "thread.hpp"
#include "var.hpp"
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