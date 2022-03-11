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
    "   .   .   .   .   X "
    " .   .   .   .   .   "
    "   .   .   .   .   . "
    " .   .   .   .   .   "
    "   .   .   .   .   . "
    " .   .   .   o   .   "
    "   .   .   .   .   . "
    " x   .   .   .   .   "
    "   .   .   .   .   O "
    " .   o   .   .   O   W";

  Pos pos = draughts::parse_position(position);
  pos::disp(pos);
  std::cout << "value = " << bb::probe(pos) << std::endl;
  std::cout << "value = " << bb::probe_raw(pos) << std::endl;
  std::cout << "value = " << draughts::egdb::probe(pos) << std::endl;
  std::cout << "value = " << draughts::egdb::probe_raw(pos) << std::endl;

  return 0;
}