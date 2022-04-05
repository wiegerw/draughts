
// includes

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "scan/bb_base.hpp"
#include "scan/bb_comp.hpp"
#include "scan/bb_index.hpp"
#include "scan/bit.hpp"
#include "scan/book.hpp"
#include "scan/common.hpp"
#include "scan/dxp.hpp"
#include "scan/eval.hpp"
#include "scan/fen.hpp"
#include "scan/game.hpp"
#include "scan/gen.hpp"
#include "scan/hash.hpp"
#include "scan/hub.hpp"
#include "scan/libmy.hpp"
#include "scan/list.hpp"
#include "scan/move.hpp"
#include "scan/pos.hpp"
#include "scan/search.hpp"
#include "scan/sort.hpp"
#include "scan/thread.hpp"
#include "scan/tt.hpp"
#include "scan/util.hpp"
#include "scan/var.hpp"
#include "scan/terminal.hpp"

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
