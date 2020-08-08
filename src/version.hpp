#pragma once

#include "base.hpp"

namespace anole
{
/**
 * for a released version
 *  theVersion will be like '0.0.16 2020/07/28'
 *
 * for a non-released version
 *  theVersion will contain 'HEAD' before version
 *   like 'HEAD 0.0.17 2020/08/07'
 *    and '0.0.17' is the next released version
 *
 * YYYY/MM/DD is the date when the last commit is committed
*/
constexpr auto theVersion = "HEAD 0.0.18 2020/08/08";

/**
 * theMagic stands for version of IR
 * each version of IR has an unique magic number,
 *  which is same as the time date IR is released
 *
 * YYYY'MM'DD may be followed by 'X like '1
 *  for the temporary change after the last release
*/
using Magic = Size;
constexpr Magic theMagic = 2020'07'28;
}
