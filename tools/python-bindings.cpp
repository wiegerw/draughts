#include "eval.hpp"
#include "gen.hpp"
#include "list.hpp"
#include "move.hpp"
#include "pos.hpp"
#include "draughts/scan.h"
#include <pybind11/pybind11.h>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;

struct ScanModule
{
  ScanModule()
  {
    bit::init();
//    hash::init();
    pos::init();
//    var::init();
//    bb::index_init();
//    bb::comp_init();
//    ml::rand_init(); // after hash keys
    eval_init();
  }

  ~ScanModule() = default;
};

PYBIND11_MODULE(draughts1, m)
{
  static ScanModule module;

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

  py::class_<List, std::shared_ptr<List>>(m, "MoveList")
    .def(py::init<>(), py::return_value_policy::copy)
    .def("clear", &List::clear)
    .def("add", &List::add)
    .def("add_move", &List::add_move)
    .def("add_capture", [](List& list, int from, int to, Bit caps, const Pos& pos, int king) { list.add_capture(Square(from), Square(to), caps, pos, king); })
    .def("set_size", &List::set_size)
    .def("set_score", &List::set_score)
    .def("move_to_front", &List::move_to_front)
    .def("sort", &List::sort)
    .def("sort_static", &List::sort_static)
    .def("move", [](const List& list, int i) { return uint64(list.move(i)); })
    .def("score", &List::score)
    .def("__getitem__", [](const List& list, int i) { std::cout << "get " << i << std::endl; if (i >= list.size()) return uint64(0); return uint64(list.move(i)); })
    .def("__iter__", [](const List& list) { return py::make_iterator(list.begin(), list.end()); },
         py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */)
    .def("__len__", [](const List& list) { return list.size(); })
    ;

  py::class_<Pos, std::shared_ptr<Pos>>(m, "Pos")
    .def(py::init<>(), py::return_value_policy::copy)
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
    .def("has_king", [](const Pos& pos) { return pos::has_king(pos); })
    .def("has_king_side", [](const Pos& pos, Side sd) { return pos::has_king(pos, sd); })
    // .def("piece_side", [](const Pos& pos, int sq) { return pos::piece_side(pos, Square(sq)); })
    .def("tempo", [](const Pos& pos) { return pos::tempo(pos); })
    .def("skew", [](const Pos& pos, Side sd) { return pos::skew(pos, sd); })
    .def("stage", [](const Pos& pos) { return pos::stage(pos); })
    .def("phase", [](const Pos& pos) { return pos::phase(pos); })
    .def("size", [](const Pos& pos) { return pos::size(pos); })
    .def("result", [](const Pos& pos, Side sd) { return pos::result(pos, sd); })
    .def("is_end", [](const Pos& pos) { return pos::is_end(pos); })
    .def("is_wipe", [](const Pos& pos) { return pos::is_wipe(pos); })
    .def("is_capture", [](const Pos& pos) { return pos::is_capture(pos); })
    .def("is_threat", [](const Pos& pos) { return pos::is_threat(pos); })
    .def("__repr__", [](const Pos& pos) { return draughts::print_position(pos, false, true); })
    .def("__str__", [](const Pos& pos) { return draughts::print_position(pos, true, true); })
    .def("__eq__", [](const Pos& pos1, const Pos& pos2) { return pos1 == pos2; })
    ;

  m.def("make_position", [](Side turn, Bit wm, Bit bm, Bit wk, Bit bk) { return Pos(turn, wm, bm, wk, bk); });
  m.def("start_position", draughts::start_position);
  m.def("print_position", draughts::print_position);
  m.def("parse_position", draughts::parse_position);
  m.def("display_position", pos::disp);
  m.def("eval_position", [](const Pos& pos) { return int(eval(pos)); });
  m.def("can_move", can_move);
  m.def("can_capture", can_capture);

  // moves
  m.def("make_move", [](int from, int to, Bit captured = Bit(0)) { return move::make(Square(from), Square(to), captured); });
  m.def("parse_move", [](const std::string & s, const Pos & pos) { return move::from_string(s, pos); });
  m.def("print_move", move::to_string);
  m.def("move_none", []() { return move::None; });
  m.def("move_from", move::from);
  m.def("move_to", move::to);
  m.def("move_captured", move::captured);
  m.def("move_index", move::index);
  m.def("move_is_capture", move::is_capture);
  m.def("move_is_promotion", move::is_promotion);
  m.def("move_is_conversion", move::is_conversion);
  m.def("move_is_forcing", move::is_forcing);
  m.def("move_is_legal", move::is_legal);
  m.def("move_to_hub", move::to_hub);
  m.def("move_from_hub", move::from_hub);

  // move generation
  m.def("generate_moves", [](const Pos& pos) { List list; gen_moves(list, pos); return list; });
  m.def("generate_captures", [](const Pos& pos) { List list; gen_captures(list, pos); return list; });
  m.def("generate_promotions", [](const Pos& pos) { List list; gen_promotions(list, pos); return list; });
  m.def("add_sacs", [](const Pos& pos) { List list; add_sacs(list, pos); return list; });

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
