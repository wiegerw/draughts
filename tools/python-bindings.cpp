#include <sstream>
#include "pos.hpp"
#include "draughts/print.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;

template <typename T>
std::string print(const T& t)
{
  std::ostringstream out;
  out << t;
  return out.str();
}

inline
bool is_white_to_move(const Pos & pos)
{
  return pos.turn() == White;
}

inline
Piece_Side piece_side(const Pos & pos, Square sq) {

  int ps = (bit::bit(pos.empty(), sq) << 2)
           | (bit::bit(pos.king(),  sq) << 1)
           | (bit::bit(pos.black(), sq) << 0);

  return Piece_Side(ps); // can be Empty
}

inline
char piece_char(const Pos& pos, int sq)
{
  switch (piece_side(pos, Square(sq))) {
    case White_Man :  return 'o';
    case Black_Man :  return 'x';
    case White_King : return 'O';
    case Black_King : return 'X';
    default :      return '.';
  }
}

inline
std::string print_bitboard(const Bit& pieces, char piece = 'x', Side turn = White)
{
  Bit empty(0);
  switch (piece)
  {
    case 'x': return draughts::print_board(Pos(turn, empty, pieces, empty, empty), true, false);
    case 'O': return draughts::print_board(Pos(turn, pieces, empty, pieces, empty), true, false);
    case 'X': return draughts::print_board(Pos(turn, empty, pieces, empty, empty), true, false);
    default: return draughts::print_board(Pos(turn, pieces, empty, empty, empty), true, false);
  }
}

inline
Pos start_position()
{
  return Pos(White, Bit(0x7DF3EF8000000000), Bit(0x0000000000FBE7DF), Bit(0), Bit(0));
}

inline
Pos empty_position()
{
  Bit empty(0);
  return Pos(White, empty, empty, empty, empty);
}

PYBIND11_MODULE(draughts1, m)
{
  m.doc() = R"pbdoc(
        Pybind11 example plugin
        -----------------------

        .. currentmodule:: python_example

        .. autosummary::
           :toctree: _generate

           add
           subtract
  )pbdoc";

  py::enum_<Piece_Side>(m, "PieceSide", "White or Black")
    .value("White_Man", White_Man, "White_Man")
    .value("Black_Man", Black_Man, "Black_Man")
    .value("White_King", White_King, "White_King")
    .value("Black_King", Black_King, "Black_King")
    .value("Empty", Empty, "Empty")
    ;

  py::enum_<Side>(m, "Side", "White or Black")
    .value("White", White, "White")
    .value("Black", Black, "Black")
    ;

  py::class_<Bit, std::shared_ptr<Bit>>(m, "Bit")
    .def(py::init<>(), py::return_value_policy::copy)
    .def(py::init<int>(), py::return_value_policy::copy)
    ;

  py::class_<Pos, std::shared_ptr<Pos>>(m, "Pos")
    .def(py::init<>(), py::return_value_policy::copy)
    .def(py::init<Side, Bit, Bit, Bit, Bit>(), py::return_value_policy::copy)
    .def("succ", &Pos::succ)
    .def("turn", &Pos::turn)
    .def("all", &Pos::all)
    .def("empty", &Pos::empty)
    .def("piece", &Pos::piece)
    .def("side", &Pos::side)
    .def("piece_side", &Pos::piece_side)
    .def("man", [](const Pos& pos) { return pos.man(); })
    .def("king", [](const Pos& pos) { return pos.king(); })
    .def("man_side", [](const Pos& pos, int side) { return pos.man(Side(side)); })
    .def("king_side", [](const Pos& pos, int side) { return pos.king(Side(side)); })
    .def("white", &Pos::white)
    .def("black", &Pos::black)
    .def("wm", &Pos::wm)
    .def("bm", &Pos::bm)
    .def("wk", &Pos::wk)
    .def("bk", &Pos::bk)
    .def("is_empty", &Pos::is_empty)
    .def("is_piece", &Pos::is_piece)
    .def("is_side", &Pos::is_side)
    .def("count", &Pos::count)
    .def("__str__", [](const Pos& pos) { return draughts::print_board(pos, true, true); })
    ;

//    .def_property("X",
//                  [](const dataset& D) { return D.X(); },
//                  [](dataset& D, const matrix<double>& X) { D.X() = X; }
//    )
//    .def("__str__", [](const dataset& D) { return print(D); })
//    .def("__eq__", [](const dataset& D1, const dataset& D2) { return D1 == D2; })
//    .def_readwrite("min_samples_leaf", &decision_tree_options::min_samples_leaf)

#ifdef VERSION_INFO
m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
m.attr("__version__") = "dev";
#endif
}
