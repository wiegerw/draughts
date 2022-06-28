/// \file draughts/utilities.h
/// \brief add your file description here.

#ifndef DRAUGHTS_UTILITIES_H
#define DRAUGHTS_UTILITIES_H

#include <cstdint>

namespace draughts {

inline
constexpr std::uint64_t reverse(std::uint64_t x)
{
  x = ((x & 0xAAAAAAAAAAAAAAAAULL) >> 1) | ((x & 0x5555555555555555ULL) << 1);
  x = ((x & 0xCCCCCCCCCCCCCCCCULL) >> 2) | ((x & 0x3333333333333333ULL) << 2);
  x = ((x & 0xF0F0F0F0F0F0F0F0ULL) >> 4) | ((x & 0x0F0F0F0F0F0F0F0FULL) << 4);
  x = ((x & 0xFF00FF00FF00FF00ULL) >> 8) | ((x & 0x00FF00FF00FF00FFULL) << 8);
  x = ((x & 0xFFFF0000FFFF0000ULL) >> 16) | ((x & 0x0000FFFF0000FFFFULL) << 16);
  return (x >> 32) | (x << 32);
}

} // namespace draughts

#endif // DRAUGHTS_UTILITIES_H
