#pragma once

#include <Arduino.h>
#include <HardwareSerial.h>

namespace esphome {
namespace espaltherma {
  
extern bool queryRegistry(char regID, unsigned char *buffer, char protocol='I');

extern HardwareSerial MySerial;

}  // namespace espaltherma
}  // namespace esphome