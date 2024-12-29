#pragma once

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/core/component.h"


/*
Mitigation to sporadic loss of updates rarely switching binary sensors.
Current assumption of the root cause:
In case of sporadic network issue which most likely prevents the sending of data to home assistant
the deduplicator value is already updated with the newest values which prevents a 2nd attempt to send the new state.

See also the discussion here:
https://community.home-assistant.io/t/esphome-entities-not-always-updating-in-homeassistant/802143/14

The RepeatingBinarySensor retries sending NEW values (max_retries times) and with a delay of retry_interval_ms between each retry.

Ideas taken from:
esphome\components\modbus_controller\binary_sensor\modbus_binarysensor.h
esphome\components\binary_sensor\binary_sensor.cpp

*/

namespace esphome {
namespace espaltherma {

class RepeatingBinarySensor : public binary_sensor::BinarySensor, public PollingComponent {
 public:
  RepeatingBinarySensor(uint32_t max_retries, uint32_t retry_interval_ms);

    /** Publish a new state to the front-end.
   *
   * @param state The new state.
   */
  void do_publish_state(bool state);

  /** Publish the initial state, this will not make the callback manager send callbacks
   * and is meant only for the initial state on boot.
   *
   * @param state The new state.
   */
  void do_publish_initial_state(bool state);

// PollingComponent
public:
  virtual void update();

protected:
  void internal_publish_state(bool state, bool is_initial);

private:
  void doSendValue(bool state, bool is_initial);
  void startPolling();

 protected:
  bool last_state_ = false;
  int repeat_counter_ = 0;
  const int MAX_RETRIES;
  const int RETRY_INTERVAL_MS;
};

}  // namespace espaltherma
}  // namespace esphome
