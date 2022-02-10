
// includes

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "bb_base.hpp"
#include "bb_comp.hpp"
#include "bb_index.hpp"
#include "bit.hpp"
#include "book.hpp"
#include "common.hpp"
#include "dxp.hpp"
#include "eval.hpp"
#include "fen.hpp"
#include "game.hpp"
#include "gen.hpp"
#include "hash.hpp"
#include "hub.hpp"
#include "libmy.hpp"
#include "list.hpp"
#include "move.hpp"
#include "pos.hpp"
#include "search.hpp"
#include "sort.hpp"
#include "thread.hpp"
#include "tt.hpp"
#include "util.hpp"
#include "var.hpp"
#include "terminal.hpp"

// variables

static Terminal G_Terminal;

int main(int argc, char * argv[]) {

   std::string arg {};
   if (argc > 1) arg = argv[1];

   bit::init();
   hash::init();
   pos::init();
   var::init();

   bb::index_init();
   bb::comp_init();

   ml::rand_init(); // after hash keys

   var::load("scan.ini");

   if (arg.empty()) { // terminal

      listen_input();
      init_high();

      G_Terminal.loop();

   } else if (arg == "dxp") {

      init_high();

      dxp::loop();

   } else if (arg == "hub") {

      listen_input();
      bit::init(); // depends on the variant

      hub::hub_loop();

   } else {

      std::cerr << "usage: " << argv[0] << " <command>" << std::endl;
      std::exit(EXIT_FAILURE);
   }

   return EXIT_SUCCESS;
}
