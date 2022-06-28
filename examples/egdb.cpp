#include "draughts/egdb.h"
#include "draughts/scan.h"

int main()
{
  draughts::egdb_enumerator enumerator(1, 1, 0, 0);
  while (enumerator.next())
  {
    std::cout << draughts::print_position(enumerator.position(), true, true) << std::endl;
  }

  return 0;
}
