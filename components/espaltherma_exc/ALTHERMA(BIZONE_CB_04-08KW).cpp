#include "esphome.h"
#include "ALTHERMA(BIZONE_CB_04-08KW).h"
//  This file is a definition file for the ESPAtherma.
//  uncomment each value you want to query for your installation.

namespace esphome {
namespace espaltherma {

#define INC_LABELS
#include "labelDefinitions.h"

size_t LabelDefsCount() { return _countof(labelDefs); }

}  // namespace espaltherma
}  // namespace esphome
