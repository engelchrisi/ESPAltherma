#include "esphome.h"
#include <ctime>

#include "setup.h"
#include "converters.h"
#include "comm.h"



// extern esphome::sntp::SNTPComponent *sntp_time;

namespace esphome {
namespace espaltherma {

const char *const TAG = "espaltherma";

Converter converter;
char registryIDs[32]; //Holds the registries to query
size_t registryIDCounter= 0; // ==_countof(registryIDs)

HardwareSerial MySerial(1);
#define SERIAL_CONFIG (SERIAL_8E1)

// #include "ALTHERMA(BIZONE_CB_04-08KW).h"
// include in main.cpp by esphome
extern LabelDef labelDefs[];

bool contains(char array[], int size, int value)
{
  for (int i = 0; i < size; i++)
  {
    if (array[i] == value)
      return true;
  }
  return false;
}

const uint16_t DEFAULT_POLLING_PERIOD_MS = 60*1000; //=60s

ESPAlthermaComponent::ESPAlthermaComponent() : PollingComponent(DEFAULT_POLLING_PERIOD_MS), _loopCounter(0)
{
}

void ESPAlthermaComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "ESP Altherma Native:");
  //ESP_LOGCONFIG(TAG, "  Address: 0x%02X", this->address_);
}

void ESPAlthermaComponent::setup() /*override*/
{
  Serial.begin(115200);
  MySerial.begin(9600, SERIAL_CONFIG, RX_PIN, TX_PIN);

  gpio_pulldown_dis(GPIO_NUM_25);
  gpio_pullup_dis(GPIO_NUM_25);

  initRegistries();
}

void ESPAlthermaComponent::do_register_entity(EntityBase * pEntity, LabelDef::EntityType entityType)
{
  auto sensor_id = pEntity->get_object_id();
  bool found= false;

  for (int j= 0; j< LabelDefsCount(); ++j)
  {
    LabelDef& labelDef= labelDefs[j];
    if (sensor_id.compare(labelDef.id) == 0)
    {
      labelDef.pEntity= pEntity;
      labelDef.entityType= entityType;
      ESP_LOGD(TAG, "Sensor with ID '%s' found in labelDefs!", sensor_id.c_str());
      found = true;
    }
  }

  if (!found)
  {
    ESP_LOGE(TAG, "Sensor with ID '%s' NOT found in labelDefs!", sensor_id.c_str());
  }
}

void ESPAlthermaComponent::do_register_sensor(sensor::Sensor* pSensor)
{
  do_register_entity(pSensor, LabelDef::ET_SENSOR);
}

void ESPAlthermaComponent::do_register_text_sensor(text_sensor::TextSensor* pSensor)
{
  do_register_entity(pSensor, LabelDef::ET_TEXTSENSOR);
}

void ESPAlthermaComponent::do_register_binary_sensor(binary_sensor::BinarySensor* pSensor)
{
  do_register_entity(pSensor, LabelDef::ET_BINARYSENSOR);
}

void ESPAlthermaComponent::initRegistries()
{
  ESP_LOGD(TAG, "initRegistries");
  memset(registryIDs, 0xff, sizeof(registryIDs));

  ESP_LOGD(TAG, "LabelDefsCount= %i", LabelDefsCount());
  registryIDCounter= 0;
  for (int j= 0; j< LabelDefsCount(); ++j)
  {
    const LabelDef& label= labelDefs[j];
    ESP_LOGV(TAG, "[%i]: %s", j, label.label);

    if (!contains(registryIDs, sizeof(registryIDs), label.registryID))
    {
      ESP_LOGD(TAG, "Adding registry {0x%2x} to be queried.", label.registryID);
      registryIDs[registryIDCounter++] = label.registryID;

      if (registryIDCounter >= _countof(registryIDs))
      {
        ESP_LOGE(TAG, "Size of registryIDs not sufficient !!!!!!!!!!!!!!!!!");
        break;
      }
    }
  }
}

//Converts to string and add the value to the JSON message
void ESPAlthermaComponent::updateValues(char regID)
{
  LabelDef *labels[128];
  int labelsCounter = 0;
  ESP_LOGI(TAG, "Updating ESPHome sensor loop #%i: {0x%2x}", _loopCounter, (int)regID);

  converter.getLabels(labelDefs, LabelDefsCount(), regID,
                      labels, labelsCounter);
  if (labelsCounter > _countof(labels))
  {
      ESP_LOGE(TAG, "Too many labels for registry %d: %i\n", regID, labelsCounter);
      return;
  }

  for (int i = 0; i < labelsCounter; i++)
  {
    bool alpha = false;
    const LabelDef& labelDef= *labels[i];
    const char* szCurrentValue= labelDef.asString;

    for (size_t j = 0; j < strlen(szCurrentValue); ++j)
    {
      char c = szCurrentValue[j];
      if (!isdigit(c) && c!='.' && !(c=='-' && j==0)){
        alpha = true;
        break;
      }
    }

    switch (labelDef.getEntityType())
    {
      case LabelDef::ET_SENSOR:
        {
          if (!alpha) {
            sensor::Sensor* pSensor= static_cast<sensor::Sensor*>(labelDef.pEntity);
            char* end;
            float value = std::strtof(szCurrentValue, &end);

            // if ((strcasecmp(labelDef.id, "fan") == 0) || (strcasecmp(labelDef.id, "fan2") == 0))
            // {
            //   ESP_LOGI(TAG, "==> ['%s']: '%s' == %f", labelDef.id, szCurrentValue, value);
            // }

            pSensor->publish_state(value);
          }
          else {
            ESP_LOGE(TAG, "Invalid float ['%s']: '%s'", labelDef.id, szCurrentValue);
          }
        }
        break;
      case LabelDef::ET_TEXTSENSOR:
        {
          text_sensor::TextSensor* pSensor= static_cast<text_sensor::TextSensor*>(labelDef.pEntity);
          pSensor->publish_state(szCurrentValue);
        }
        break;
      case LabelDef::ET_BINARYSENSOR:
        {
          bool value= false;
          espaltherma::RepeatingBinarySensor* pSensor= static_cast<espaltherma::RepeatingBinarySensor*>(labelDef.pEntity);

          if (alpha) {
            ESP_LOGD(TAG, "'%s': alpha BinarySensor '%s'", pSensor->get_name().c_str(), szCurrentValue);
            value = (strcasecmp(szCurrentValue, "ON") == 0) ||
                    (strcasecmp(szCurrentValue, "TRUE") == 0);
          }
          else {
            ESP_LOGD(TAG, "'%s': float BinarySensor '%s'", pSensor->get_name().c_str(), szCurrentValue);
            char* end;
            float fval = std::strtof(szCurrentValue, &end);
            value = fval != 0;
          }

          if (strcasecmp(labelDef.id, "heizstab_stufe_1") == 0 || strcasecmp(labelDef.id, "heizstab_stufe_2") == 0)
          {
            ESP_LOGI(TAG, "'%s': ==> '%s' == %i", pSensor->get_name().c_str(), szCurrentValue, value? 1:0);
          }

          pSensor->do_publish_state(value);
        }
        break;
      default:
        ESP_LOGE(TAG, "Unexpected sensor type ['%s']: %i", labelDef.id, labelDef.getEntityType());
        break;
    }

  }
}

void ESPAlthermaComponent::update() /*override*/
{
  const unsigned long startMs = millis();
  ESP_LOGD(TAG, ">>>>>>>>>>>>>>>>>>>> Updating ");

  //Querying all registries
  for (; _loopCounter < registryIDCounter; ++_loopCounter)
  {
    const char regID= registryIDs[_loopCounter];
    ESP_LOGI(TAG, "Altherma Serial Update loop #%i: {0x%2x}", _loopCounter, (int)regID);
    unsigned char buff[64] = {0};
    int tries = 0;
    while (!queryRegistry(regID, buff, PROTOCOL) && tries++ < 3)
    {
      ESP_LOGI(TAG, "RETRYING #%i", tries);
      App.feed_wdt(); // avoid watchdog trigger
      delay(1000);
    }
    unsigned char receivedRegistryID = PROTOCOL == 'S' ? buff[0] : buff[1];
    if (regID == receivedRegistryID) //if replied registerID is coherent with the command
    {
      converter.readRegistryValues(labelDefs, LabelDefsCount(),
                                    buff, PROTOCOL); //process all values from the register
      updateValues(regID);       //update jsonbuf
    }

    const unsigned long deltaMs = millis() - startMs;
    if (deltaMs > 10*1000)
    {
      // avoid too long loops
      ESP_LOGW(TAG, "Interrupting update loop at #%i", _loopCounter);
      break; // interrupt
    }
  }

  if (_loopCounter >= registryIDCounter)
  {
    _loopCounter= 0;
  }

  ESP_LOGD(TAG, "<<<<<<<<<<<<<<<<<<<< EOF Updating ");
}

}  // namespace espaltherma
}  // namespace esphome