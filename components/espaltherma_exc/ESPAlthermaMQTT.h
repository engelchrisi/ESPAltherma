#pragma once

#include "esphome.h"

namespace esphome {
namespace espaltherma {

class ESPAlthermaComponent : public PollingComponent
{
public:
  ESPAlthermaComponent();

  void do_register_sensor(sensor::Sensor* pSensor);
  void do_register_text_sensor(text_sensor::TextSensor* pSensor);
  void do_register_binary_sensor(binary_sensor::BinarySensor* pSensor);

protected:
  virtual void setup();

  void initRegistries();

  //Converts to string and add the value to the JSON message
  void updateValues(char regID);

  void update() override;

  void dump_config();

  float get_setup_priority() const override {
    return esphome::setup_priority::DATA;
  }

  void do_register_entity(EntityBase * pEntity, LabelDef::EntityType entityType);

private:
  void  setPollingIntervallMs(uint16_t ms);

  size_t  _loopCounter;
};

}  // namespace espaltherma
}  // namespace esphome

