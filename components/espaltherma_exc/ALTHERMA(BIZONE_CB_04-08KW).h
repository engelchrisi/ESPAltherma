#pragma once

#include "labeldef.h"
//  This file is a definition file for the ESPAtherma.
//  uncomment each value you want to query for your installation.

namespace esphome {
namespace espaltherma {

extern LabelDef labelDefs[];

#ifndef _countof
  #define _countof(arr) (sizeof(arr) / sizeof(arr[0]))
#endif
extern size_t LabelDefsCount();


}  // namespace espaltherma
}  // namespace esphome
