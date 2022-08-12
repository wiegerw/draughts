// Copyright: Wieger Wesselink 2022
// Distributed under the Distributed under the GPL-3.0 Software License.
// (See accompanying file license.txt or copy at https://www.gnu.org/licenses/gpl-3.0.txt)

#include "scan/bb_base.hpp"
#include "scan/bb_comp.hpp"
#include "scan/bb_index.hpp"
#include "scan/eval.hpp"
#include "scan/game.hpp"
#include "scan/gen.hpp"
#include "scan/hash.hpp"
#include "scan/list.hpp"
#include "scan/terminal.hpp"
#include "scan/move.hpp"
#include "scan/pos.hpp"
#include "scan/search.hpp"
#include "scan/thread.hpp"
#include "scan/tt.hpp"
#include "draughts/egdb.h"
#include "draughts/pdn.h"
#include "draughts/scan.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <algorithm>

namespace py = pybind11;

// Converts a position into a 1-dimensional array of size 400 representing 4 10x10 board positions:
// - player men
// - opponent men
// - player kings
// - opponent kings
inline
py::array_t<int> pos_to_numpy1(const Pos& pos)
{
  py::array_t<int> result(400); // TODO: make sure array_t is initialized with zeroes
  py::buffer_info info = result.request();
  int* ptr = reinterpret_cast<int*>(info.ptr);
  std::fill(ptr, ptr + 400, 0);

  auto index = [](int f)
  {
    bool shift = ((f - 1) / 5) % 2;
    return 2*f - 1 - shift;
  };

  for (int f = 1; f <= 50; f++)
  {
    if (pos.is_empty_(f))
    {
      continue;
    }

    int i = index(f);
    bool is_king = pos.is_king(f);
    bool is_white = pos.is_white(f);
    bool is_white_to_move = pos.is_white_to_move();
    if (is_white == is_white_to_move) // the piece belongs to the player
    {
      if (is_king)
      {
        ptr[200 + i] = 1;
      }
      else
      {
        ptr[0 + i] = 1;
      }
    }
    else
    {
      if (is_king)
      {
        ptr[300 + i] = 1;
      }
      else
      {
        ptr[100 + i] = 1;
      }
    }
  }
  return result;
}

// Takes care of initialization
struct ScanModule
{
  ScanModule()
  {
    bit::init();
    hash::init();
    pos::init();
    var::init();
    bb::index_init();
    bb::comp_init();
    ml::rand_init(); // after hash keys
  }

  ~ScanModule() = default;
};

PYBIND11_MODULE(draughts1, m)
{
  static ScanModule module;

  m.doc() = "Python bindings for Scan";

  py::enum_<Piece_Side>(m, "PieceSide", "White or Black")
    .value("White_Man", White_Man, "White_Man")
    .value("Black_Man", Black_Man, "Black_Man")
    .value("White_King", White_King, "White_King")
    .value("Black_King", Black_King, "Black_King")
    .value("Empty", Empty, "Empty")
    ;

  py::enum_<Piece>(m, "Piece", "Man or King")
    .value("Man", Man, "Man")
    .value("King", King, "King")
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
    .def("add_capture", &List::add_capture)
    .def("set_size", &List::set_size)
    .def("set_score", &List::set_score)
    .def("move_to_front", &List::move_to_front)
    .def("sort", &List::sort)
    .def("sort_static", &List::sort_static)
    .def("move", [](const List& list, int i) { return list.move(i); })
    .def("score", &List::score)
    .def("__getitem__", [](const List& list, int i) { return list.move(i); })
    .def("__iter__", [](const List& list) { return py::make_iterator(list.begin(), list.end()); }, py::keep_alive<0, 1>())
    .def("__len__", [](const List& list) { return list.size(); })
    ;

  // pos.hpp
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
    .def("wolf", &Pos::wolf)
    .def("count", &Pos::count)
    // added functions
    .def("is_empty_", &Pos::is_empty_)
    .def("is_king", &Pos::is_king)
    .def("is_white", &Pos::is_white)
    .def("is_black", &Pos::is_black)
    .def("is_white_to_move", &Pos::is_white_to_move)
    .def("flip", &Pos::flip)
    .def("put_piece", &Pos::put_piece)
    .def("white_man_count", [](const Pos& pos) { return bit::count(pos.wm()); })
    .def("black_man_count", [](const Pos& pos) { return bit::count(pos.bm()); })
    .def("white_king_count", [](const Pos& pos) { return bit::count(pos.wk()); })
    .def("black_king_count", [](const Pos& pos) { return bit::count(pos.bk()); })

    // The following are global functions in Scan
    .def("has_king", [](const Pos& pos) { return pos::has_king(pos); })
    .def("has_king_side", [](const Pos& pos, Side sd) { return pos::has_king(pos, sd); })
    .def("piece_side_square", [](const Pos& pos, int sq) { return pos::piece_side(pos, Square(sq)); })
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
    .def("can_move", can_move)
    .def("can_capture", can_capture)

    .def("__repr__", [](const Pos& pos) { return draughts::print_position(pos, false, true); })
    .def("__str__", [](const Pos& pos) { return draughts::print_position(pos, true, true); })
    .def("__eq__", [](const Pos& pos1, const Pos& pos2) { return pos1 == pos2; })
    .def("__hash__", hash::key)
    .def(py::pickle(
      [](const Pos &pos)
      {
        return py::make_tuple(pos.turn(), uint64(pos.wm()), uint64(pos.bm()), uint64(pos.wk()), uint64(pos.bk()));
      },
      [](const py::tuple& t)
      {
        if (t.size() != 5)
        {
          throw std::runtime_error("Invalid state!");
        }
        Side side = t[0].cast<int>() == 0 ? Side::White : Side::Black;
        Bit wm(t[1].cast<uint64>());
        Bit bm(t[2].cast<uint64>());
        Bit wk(t[3].cast<uint64>());
        Bit bk(t[4].cast<uint64>());
        return Pos(side, wm, bm, wk, bk);
      }))
    ;

  m.def("make_position", [](Side turn, Bit wm, Bit bm, Bit wk, Bit bk) { return Pos(turn, wm, bm, wk, bk); });
  m.def("start_position", draughts::start_position);
  m.def("print_position", draughts::print_position);
  m.def("parse_position", draughts::parse_position);
  m.def("display_position", pos::disp);
  m.def("eval_position", [](const Pos& pos) { return int(eval(pos)); });

  py::class_<Node, std::shared_ptr<Node>>(m, "Node")
    .def(py::init<>(), py::return_value_policy::copy)
    .def("position", [](const Node& node) { return Pos(node); })
    .def("succ", &Node::succ)
    .def("is_end", &Node::is_end)
    .def("is_draw", &Node::is_draw)
    ;
  m.def("make_node", [](const Pos& pos) { return Node(pos); });

  py::class_<Line, std::shared_ptr<Line>>(m, "Line")
    .def(py::init<>(), py::return_value_policy::copy)
    .def("clear", &Line::clear)
    .def("add", &Line::add)
    .def("set", &Line::set)
    .def("concat", &Line::concat)
    .def("to_string", &Line::to_string)
    .def("to_hub", &Line::to_hub)
    .def("__getitem__", [](const Line& line, int i) { return line[i]; })
    .def("__iter__", [](const Line& line) { return py::make_iterator(line.begin(), line.end()); }, py::keep_alive<0, 1>())
    .def("__len__", [](const Line& line) { return line.size(); })
  ;

  py::enum_<Output_Type>(m, "OutputType", "The output type")
    .value("None", Output_None, "No output")
    .value("Terminal", Output_Terminal, "Print output to terminal")
    .value("Hub", Output_Hub, "Print output to hub")
    ;

  // search
  py::class_<Search_Input, std::shared_ptr<Search_Input>>(m, "SearchInput")
    .def(py::init<>(), py::return_value_policy::copy)
    .def("init", &Search_Input::init)
    .def("set_time", &Search_Input::set_time)
    .def_readwrite("move", &Search_Input::move)
    .def_readwrite("book", &Search_Input::book)
    .def_readwrite("depth", &Search_Input::depth)
    .def_readwrite("nodes", &Search_Input::nodes)
    .def_readwrite("input", &Search_Input::input)
    .def_readwrite("output", &Search_Input::output)
    .def_readwrite("smart", &Search_Input::smart)
    .def_readwrite("moves", &Search_Input::moves)
    .def_readwrite("time", &Search_Input::time)
    .def_readwrite("inc", &Search_Input::inc)
    .def_readwrite("ponder", &Search_Input::ponder)
    ;

  py::class_<Search_Output, std::shared_ptr<Search_Output>>(m, "SearchOutput")
    .def(py::init<>(), py::return_value_policy::copy)
    .def("init", &Search_Output::init)
    .def("end", &Search_Output::end)
//    .def("start_iter", &Search_Output::start_iter)
//    .def("end_iter", &Search_Output::end_iter)
    .def("new_best_move", [](Search_Output& output, Move mv, Score sc = score::None) { output.new_best_move(mv, sc); })
    .def("new_best_move_full", [](Search_Output& output, Move mv, Score sc, Flag flag, Depth depth, const Line& pv) { output.new_best_move(mv, sc, flag, depth, pv); })
    .def("ply_avg", &Search_Output::ply_avg)
    .def("time", &Search_Output::time)
    .def_readwrite("move", &Search_Output::move)
    .def_readwrite("answer", &Search_Output::answer)
    .def_readwrite("score", &Search_Output::score)
    .def_readwrite("flag", &Search_Output::flag)
    .def_readwrite("depth", &Search_Output::depth)
    .def_readwrite("pv", &Search_Output::pv)
    .def_readwrite("node", &Search_Output::node)
    .def_readwrite("leaf", &Search_Output::leaf)
    .def_readwrite("ply_sum", &Search_Output::ply_sum)
    ;

  // special values for the score of a search
  m.def("score_inf", []{ return score::Inf; });
  m.def("score_bb_inf", []{ return score::BB_Inf; });
  m.def("score_eval_inf", []{ return score::Eval_Inf; });
  m.def("score_none", []{ return score::None; });

  m.def("search", search);
  m.def("quick_move", quick_move);
  m.def("quick_score", quick_score);

  // move
  m.def("make_move", move::make);
  m.def("parse_move", move::from_string);
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

  // tt.h
  py::enum_<Flag>(m, "TTFlag", py::arithmetic(), "Transposition table flag")
    .value("None", Flag::None, "None")
    .value("Upper", Flag::Upper, "Upper")
    .value("Lower", Flag::Lower, "Lower")
    .value("Exact", Flag::Exact, "Exact")
    ;

  py::class_<TT, std::shared_ptr<TT>>(m, "TranspositionTable", "Transposition table")
    .def(py::init<>(), py::return_value_policy::copy)
    .def("set_size", &TT::set_size)
    .def("clear", &TT::clear)
    .def("inc_date", &TT::inc_date)
    .def("store", &TT::store)
    .def("probe", &TT::probe)
    ;

  m.def("tt_is_lower", is_lower);
  m.def("tt_is_upper", is_upper);
  m.def("tt_is_exact", is_exact);

  // game.hpp
  py::class_<Game, std::shared_ptr<Game>>(m, "Game", "A draughts game")
    .def(py::init<>(), py::return_value_policy::copy)
    .def("clear", &Game::clear)
    .def("init", [](Game& game, const Pos& pos) { game.init(pos); })
    .def("init_full", [](Game& game, const Pos& pos, int moves, double time, double inc) { game.init(pos, moves, time, inc); })
    .def("add_move", &Game::add_move)
    .def("goto", &Game::go_to)
    .def("turn", &Game::turn)
    .def("is_end", &Game::is_end)
    .def("result", &Game::result)
    .def("size", &Game::size)
    .def("ply", &Game::ply)
    .def("move", &Game::move)
    .def("__getitem__", [](const Game& game, int i) { return game[i]; })
    .def("__len__", [](const Game& game) { return game.size(); })
    .def("moves", &Game::moves)
    .def("time", &Game::time)
    .def("inc", &Game::inc)
    .def("start_pos", &Game::start_pos)
    .def("pos", &Game::pos)
    .def("turn", &Game::turn)
    .def("node", &Game::node)
    ;

  m.def("result_to_string", result_to_string);

  // hash.hpp
  m.def("hash_key", hash::key);
  m.def("hash_index", hash::index);
  m.def("hash_lock", hash::lock);

  // bb_comp.hpp
  py::class_<bb::Index_, std::shared_ptr<bb::Index_>>(m, "EGDBCompressedIndex", "Endgame database compresssed index")
    .def(py::init<>(), py::return_value_policy::copy)
    .def("load", &bb::Index_::load)
    .def("__getitem__", [](const bb::Index_& index, int i) { return index[i]; })
    .def("__len__", [](const bb::Index_& index) { return index.size(); })
    ;

  // bb_base.hpp
  py::enum_<bb::Value>(m, "EGDBValue")  // endgame database value
    .value("Draw", bb::Draw, "Draw")
    .value("Loss", bb::Loss, "Loss")
    .value("Win", bb::Win, "Win")
    .value("Unknown", bb::Unknown, "Unknown")
    ;

  py::class_<draughts::egdb, std::shared_ptr<draughts::egdb>>(m, "EGDB", "Endgame database")
    .def(py::init<>(), py::return_value_policy::copy)
    .def("init", &draughts::egdb::init)
    .def("pos_is_load", &draughts::egdb::pos_is_load)
    .def("pos_is_search", &draughts::egdb::pos_is_search)
    .def("probe", &draughts::egdb::probe)
    .def("probe_raw", &draughts::egdb::probe_raw)
    .def("value_update", &draughts::egdb::value_update)
    .def("value_age", &draughts::egdb::value_age)
    .def("value_max", &draughts::egdb::value_max)
    .def("value_nega", &draughts::egdb::value_nega)
    .def("value_from_nega", &draughts::egdb::value_from_nega)
    .def("value_to_string", &draughts::egdb::value_to_string)
    ;

  py::class_<draughts::egdb_enumerator, std::shared_ptr<draughts::egdb_enumerator>>(m, "EGDBEnumerator", "Endgame database enumerator")
    .def(py::init<int, int, int, int>(), py::return_value_policy::copy)
    .def("next", &draughts::egdb_enumerator::next)
    .def("position", &draughts::egdb_enumerator::position)
    ;

  // bb_index.hpp
  py::class_<draughts::egdb_index, std::shared_ptr<draughts::egdb_index>>(m, "EGDBIndex", "Endgame database index")
    .def(py::init<>(), py::return_value_policy::copy)
    .def("init", &draughts::egdb_index::init)
    .def("id_make", &draughts::egdb_index::id_make)
    .def("id_is_illegal", &draughts::egdb_index::id_is_illegal)
    .def("id_is_end", &draughts::egdb_index::id_is_end)
    .def("id_size", &draughts::egdb_index::id_size)
    .def("id_wm", &draughts::egdb_index::id_wm)
    .def("id_bm", &draughts::egdb_index::id_bm)
    .def("id_wk", &draughts::egdb_index::id_wk)
    .def("id_bk", &draughts::egdb_index::id_bk)
    .def("id_name", &draughts::egdb_index::id_name)
    .def("pos_id", &draughts::egdb_index::pos_id)
    .def("pos_index", &draughts::egdb_index::pos_index)
    .def("index_size", &draughts::egdb_index::index_size)
    ;

  // var.hpp
  py::class_<draughts::scan_settings, std::shared_ptr<draughts::scan_settings>>(m, "Scan")
    .def(py::init<>(), py::return_value_policy::copy)
    .def("init", &draughts::scan_settings::init)
    .def("load", &draughts::scan_settings::load)
    .def("update", &draughts::scan_settings::update)
    .def("get", &draughts::scan_settings::get)
    .def("set", &draughts::scan_settings::set)
    .def("variant_name", &draughts::scan_settings::variant_name)
    ;

  // thread.hpp
  m.def("listen_input", listen_input);

  // terminal.hpp
  m.def("terminal_init_low", init_low);
  m.def("terminal_init_high", init_high);

  // added
  m.def("run_terminal_game", run_terminal_game);
  m.def("playout_minimax", draughts::playout_minimax);
  m.def("playout_random", draughts::playout_random);
  m.def("play_forced_moves", draughts::play_forced_moves);
  m.def("piece_count_eval", draughts::piece_count_eval);
  m.def("naive_rollout", draughts::naive_rollout);

  // pdn
  py::class_<draughts::pdn_game, std::shared_ptr<draughts::pdn_game>>(m, "PDNGame")
    .def(py::init<>(), py::return_value_policy::copy)
    .def_readwrite("event", &draughts::pdn_game::event)
    .def_readwrite("date", &draughts::pdn_game::date)
    .def_readwrite("white", &draughts::pdn_game::white)
    .def_readwrite("black", &draughts::pdn_game::black)
    .def_readwrite("result", &draughts::pdn_game::result)
    .def_readwrite("moves", &draughts::pdn_game::moves)
    ;

  m.def("parse_pdn_game", draughts::parse_pdn_game);
  m.def("parse_pdn_file", draughts::parse_pdn_file);
  m.def("scan_search", draughts::scan_search);
  m.def("pos_to_numpy1", pos_to_numpy1, py::return_value_policy::move);
}
