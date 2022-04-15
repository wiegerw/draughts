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