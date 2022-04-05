
#ifndef EVAL_HPP
#define EVAL_HPP

// includes

#include "scan/common.hpp"
#include "scan/libmy.hpp"

class Pos;

// functions

void eval_init ();

Score eval (const Pos & pos);

#endif // !defined EVAL_HPP

