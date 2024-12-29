#include "esphome.h"
#include "repeating_binary_sensor.h"

namespace esphome {
namespace espaltherma {

static const char *const TAG = "espaltherma.binary_sensor";
// const char *const TAG = "espaltherma";

RepeatingBinarySensor::RepeatingBinarySensor(uint32_t max_retries, uint32_t retry_interval_ms) : PollingComponent(SCHEDULER_DONT_RUN),
              MAX_RETRIES(max_retries), RETRY_INTERVAL_MS(retry_interval_ms)
{
}

void RepeatingBinarySensor::startPolling()
{
  const uint32_t ms = this->RETRY_INTERVAL_MS;
  this->set_update_interval(ms);
  ESP_LOGD(TAG, "Set Polling intervall to %i ms", ms);

  this->start_poller();
}

/*virtual*/ void RepeatingBinarySensor::update()
{
  ++this->repeat_counter_;
  if (this->repeat_counter_ >= this->MAX_RETRIES)
    this->stop_poller();

  ESP_LOGD(TAG, "'%s': Sending state %s", this->get_name().c_str(), ONOFF(state));
  ESP_LOGD(TAG, "'%s': Retry #%i", this->get_name().c_str(), this->repeat_counter_);

  doSendValue(this->last_state_, false/*is_initial*/);
}

void RepeatingBinarySensor::doSendValue(bool state, bool is_initial)
{
  ESP_LOGV(TAG, "'%s': sending value %i", this->get_name().c_str(), state);

  if (this->filter_list_ == nullptr) {
    this->send_state_internal(state, is_initial);
  } else {
    this->filter_list_->input(state, is_initial);
  }
}


void RepeatingBinarySensor::internal_publish_state(bool state, bool is_initial) {
  if (!this->publish_dedup_.next(state)) {
    ESP_LOGD(TAG, "'%s': same state %i", this->get_name().c_str(), state);
    return;
  }

  ESP_LOGD(TAG, "==> '%s': NEW state %i", this->get_name().c_str(), state);

  // a new value => restart the poller with repeat_counter_ = 0
  this->stop_poller();

  this->last_state_ = state;
  doSendValue(state, is_initial);

  // repeat the sending of the value
  this->repeat_counter_ = 0;
  startPolling();
}

void RepeatingBinarySensor::do_publish_state(bool state) {
  internal_publish_state(state, false);
}

void RepeatingBinarySensor::do_publish_initial_state(bool state) {
  internal_publish_state(state, true);
}

}  // namespace espaltherma
}  // namespace esphome
