#include <sstream>
#include "pos.hpp"
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

PYBIND11_MODULE(draughtslib, m)
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
;

#ifdef VERSION_INFO
m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
m.attr("__version__") = "dev";
#endif
}
