#include <PubSubClient.h>
#include <EEPROM.h>
#include "restart.h"

#define MQTT_attr "espaltherma/ATTR"
#define MQTT_lwt "espaltherma/LWT"

// BYTE offsets
#define EEPROM_ADDR_H1    0
#define EEPROM_ADDR_H2    1
#define EEPROM_ADDR_EOF   2 // contains an EEPROM_OK_CHAR as marker that EEPROM has valid info
  #define EEPROM_OK_CHAR 'O' // ^ OK

#define MQTT_attr "espaltherma/ATTR"
#define MQTT_lwt "espaltherma/LWT"

#ifdef JSONTABLE
char jsonbuff[MAX_MSG_SIZE] = "[{\0";
#else
char jsonbuff[MAX_MSG_SIZE] = "{\0";
#endif

WiFiClient espClient;
PubSubClient client(espClient);

void sendValues()
{
  Serial.printf("Sending values in MQTT.\n");
#ifdef ARDUINO_M5Stick_C
  //Add M5 APX values
  snprintf(jsonbuff + strlen(jsonbuff),MAX_MSG_SIZE - strlen(jsonbuff) , "\"%s\":\"%.3gV\",\"%s\":\"%gmA\",", "M5VIN", M5.Axp.GetVinVoltage(),"M5AmpIn", M5.Axp.GetVinCurrent());
  snprintf(jsonbuff + strlen(jsonbuff),MAX_MSG_SIZE - strlen(jsonbuff) , "\"%s\":\"%.3gV\",\"%s\":\"%gmA\",", "M5BatV", M5.Axp.GetBatVoltage(),"M5BatCur", M5.Axp.GetBatCurrent());
  snprintf(jsonbuff + strlen(jsonbuff),MAX_MSG_SIZE - strlen(jsonbuff) , "\"%s\":\"%.3gmW\",", "M5BatPwr", M5.Axp.GetBatPower());
#endif
  snprintf(jsonbuff + strlen(jsonbuff),MAX_MSG_SIZE - strlen(jsonbuff) , "\"%s\":\"%ddBm\",", "WifiRSSI", WiFi.RSSI());
  snprintf(jsonbuff + strlen(jsonbuff),MAX_MSG_SIZE - strlen(jsonbuff) , "\"%s\":\"%s\",", "IP", WiFi.localIP().toString().c_str());
  snprintf(jsonbuff + strlen(jsonbuff),MAX_MSG_SIZE - strlen(jsonbuff) , "\"%s\":\"%d\",", "FreeMem", ESP.getFreeHeap());
  jsonbuff[strlen(jsonbuff) - 1] = '}';
#ifdef JSONTABLE
  strcat(jsonbuff,"]");
#endif
  client.publish(MQTT_attr, jsonbuff);
#ifdef JSONTABLE
  strcpy(jsonbuff, "[{\0");
#else
  strcpy(jsonbuff, "{\0");
#endif
}

void saveEEPROM(int addr, uint8_t state){
    EEPROM.write(addr,state);
    EEPROM.commit();
    mqttSerial.printf("Committed [%i]=%i", addr, (int)state);
}

void readEEPROM(){
  if (EEPROM_OK_CHAR == EEPROM.read(EEPROM_ADDR_EOF)){
#ifdef PIN_THERM_H1
    {
      const uint8_t state= EEPROM.read(EEPROM_ADDR_H1);
      digitalWrite(PIN_THERM_H1, state);
      const char *szState= (state != HIGH)? "OFF":"ON";
      client.publish("espaltherma/STATE_MAINZ", szState, true);
      mqttSerial.printf("Restored previous H1(main) state: %s\n",szState );
    }
#endif
#ifdef PIN_THERM_H2
    {
      const uint8_t state= EEPROM.read(EEPROM_ADDR_H2);
      digitalWrite(PIN_THERM_H2, state);
      const char *szState= (state != HIGH)? "OFF":"ON";
      client.publish("espaltherma/STATE_ADDZ", szState, true);
      mqttSerial.printf("Restored previous H2(addit) state: %s\n",szState );
    }
#endif    
  }
  else{
    mqttSerial.printf("EEPROM not initialized (%d). Initializing...",EEPROM.read(EEPROM_ADDR_EOF));
    const uint8_t state= LOW; // OFF
#ifdef PIN_THERM_H1  
    digitalWrite(PIN_THERM_H1, state);
    EEPROM.write(EEPROM_ADDR_H1, state);
#endif
#ifdef PIN_THERM_H2
    digitalWrite(PIN_THERM_H2, state);
    EEPROM.write(EEPROM_ADDR_H2, state);
#endif
    EEPROM.write(EEPROM_ADDR_EOF, EEPROM_OK_CHAR);
    EEPROM.commit();
  }
}

#ifdef PIN_THERM_H1
  const char* H1_SWITCH_CONFIG= 
        "{"
        "\"name\":\"AlthermaMainZone\","
        "\"cmd_t\":\"~/THERM_MAINZ\","
        "\"stat_t\":\"~/STATE_MAINZ\","
        "\"pl_off\":\"OFF\","
        "\"pl_on\":\"ON\","
        "\"~\":\"espaltherma\"}";
#endif
#ifdef PIN_THERM_H2
  const char* H2_SWITCH_CONFIG= 
        "{"
        "\"name\":\"AlthermaAddZone\","
        "\"cmd_t\":\"~/THERM_ADDZ\","
        "\"stat_t\":\"~/STATE_ADDZ\","
        "\"pl_off\":\"OFF\","
        "\"pl_on\":\"ON\","
        "\"~\":\"espaltherma\"}";
#endif

void reconnectMqtt()
{
  // Loop until we're reconnected
  int i = 0;
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");

    if (client.connect("ESPAltherma-dev", MQTT_USERNAME, MQTT_PASSWORD, MQTT_lwt, 0, true, "Offline"))
    {
      Serial.println("connected!");
      #if 1
      client.publish("homeassistant/sensor/espAltherma/config", "{\"name\":\"AlthermaSensors\",\"stat_t\":\"~/LWT\",\"avty_t\":\"~/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"espaltherma\",\"device\":{\"identifiers\":[\"ESPAltherma\"]}, \"~\":\"espaltherma\",\"json_attr_t\":\"~/ATTR\"}", true);
      #else
      client.publish("homeassistant/sensor/espAltherma/config", "{"
                                                                " \"unique_id\":\"esp_altherma\","
                                                                " \"name\":\"Altherma Sensors\","
                                                                " \"stat_t\":\"~/LWT\","
                                                                " \"avty_t\":\"~/LWT\","
                                                                " \"pl_avail\":\"Online\","
                                                                " \"pl_not_avail\":\"Offline\","
                                                                // " \"uniq_id\":\"espaltherma\","
                                                                " \"device\":{\"identifiers\":[\"ESPAltherma\"]},"
                                                                " \"~\":\"espaltherma\","
                                                                " \"json_attr_t\":\"~/ATTR\""
                                                                "}"
                                                                , true);
      #endif
      client.publish(MQTT_lwt, "Online", true);
#ifdef PIN_THERM_H1
      client.publish("homeassistant/switch/espAltherma00/config", H1_SWITCH_CONFIG, true);      
      client.subscribe("espaltherma/THERM_MAINZ");
#else
      // Publish empty retained message so discovered entities are removed from HA
      client.publish("homeassistant/switch/espAltherma00/config", "", true);      
#endif
#ifdef PIN_THERM_H2
      client.publish("homeassistant/switch/espAltherma01/config", H2_SWITCH_CONFIG, true);
      client.subscribe("espaltherma/THERM_ADDZ");
#else
      // Publish empty retained message so discovered entities are removed from HA
      client.publish("homeassistant/switch/espAltherma01/config", "", true);      
#endif

#ifdef PIN_SG1
      // Smart Grid
      client.publish("homeassistant/select/espAltherma/sg/config", "{\"availability\":[{\"topic\":\"espaltherma/LWT\",\"payload_available\":\"Online\",\"payload_not_available\":\"Offline\"}],\"availability_mode\":\"all\",\"unique_id\":\"espaltherma_sg\",\"device\":{\"identifiers\":[\"ESPAltherma\"],\"manufacturer\":\"ESPAltherma\",\"model\":\"M5StickC PLUS ESP32-PICO\",\"name\":\"ESPAltherma\"},\"icon\":\"mdi:solar-power\",\"name\":\"EspAltherma Smart Grid\",\"command_topic\":\"espaltherma/sg/set\",\"command_template\":\"{% if value == 'Free Running' %} 0 {% elif value == 'Forced Off' %} 1 {% elif value == 'Recommended On' %} 2 {% elif value == 'Forced On' %} 3 {% else %} 0 {% endif %}\",\"options\":[\"Free Running\",\"Forced Off\",\"Recommended On\",\"Forced On\"],\"state_topic\":\"espaltherma/sg/state\",\"value_template\":\"{% set mapper = { '0':'Free Running', '1':'Forced Off', '2':'Recommended On', '3':'Forced On' } %} {% set word = mapper[value] %} {{ word }}\"}", true);
      client.subscribe("espaltherma/sg/set");
      client.publish("espaltherma/sg/state", "0");
#else
      // Publish empty retained message so discovered entities are removed from HA
      client.publish("homeassistant/select/espAltherma/sg/config", "", true);
#endif
    }
    else
    {
      Serial.printf("failed, rc=%d, try again in 5 seconds", client.state());
      unsigned long start = millis();
      while (millis() < start + 5000)
      {
        ArduinoOTA.handle();
      }

      if (i++ == 100) {
        Serial.printf("Tried for 500 sec, rebooting now.");
        restart_board();
      }
    }
  }
  mqttSerial.println("MQTT reconnected");
}

#if defined(PIN_THERM_H1) || defined(PIN_THERM_H2)
void callbackTherm(unsigned int pin, int eepromAddr, const char* answerTopic, byte *payload, unsigned int length)
{
  payload[length] = '\0';

  // Is it ON or OFF?
  // Ok I'm not super proud of this, but it works :p
  if (payload[1] == 'F')
  { //turn off
    if (digitalRead(pin) != LOW) {
      digitalWrite(pin, LOW);
      saveEEPROM(eepromAddr, LOW);
      client.publish(answerTopic, "OFF", true);
      mqttSerial.printf("[%i]: Turned OFF => %s", eepromAddr, answerTopic);
    }
    else {
      mqttSerial.printf("[%i]: Was already turned OFF => %s", eepromAddr, answerTopic);
    }
  }
  else if (payload[1] == 'N')
  { //turn on
    if (digitalRead(pin) != HIGH) {
      digitalWrite(pin, HIGH);
      saveEEPROM(eepromAddr, HIGH);
      client.publish(answerTopic, "ON", true);
      mqttSerial.printf("[%i]: Turned ON => %s", eepromAddr, answerTopic);
    }
    else {
      mqttSerial.printf("[%i]: Was already turned ON => %s", eepromAddr, answerTopic);
    }      
  }
  else if (payload[0] == 'R')//R(eset/eboot)
  {
    mqttSerial.println("Rebooting");
    delay(100);
    restart_board();
  }
  else
  {
    Serial.printf("Unknown message: %s\n", payload);
  }
}
#endif

#ifdef PIN_SG1
//Smartgrid callbacks
void callbackSg(byte *payload, unsigned int length)
{
  payload[length] = '\0';

  if (payload[0] == '0')
  {
    // Set SG 0 mode => SG1 = INACTIVE, SG2 = INACTIVE
    digitalWrite(PIN_SG1, SG_RELAY_INACTIVE_STATE);
    digitalWrite(PIN_SG2, SG_RELAY_INACTIVE_STATE);
    client.publish("espaltherma/sg/state", "0");
    Serial.println("Set SG mode to 0 - Normal operation");
  }
  else if (payload[0] == '1')
  {
    // Set SG 1 mode => SG1 = INACTIVE, SG2 = ACTIVE
    digitalWrite(PIN_SG1, SG_RELAY_INACTIVE_STATE);
    digitalWrite(PIN_SG2, SG_RELAY_ACTIVE_STATE);
    client.publish("espaltherma/sg/state", "1");
    Serial.println("Set SG mode to 1 - Forced OFF");
  }
  else if (payload[0] == '2')
  {
    // Set SG 2 mode => SG1 = ACTIVE, SG2 = INACTIVE
    digitalWrite(PIN_SG1, SG_RELAY_ACTIVE_STATE);
    digitalWrite(PIN_SG2, SG_RELAY_INACTIVE_STATE);
    client.publish("espaltherma/sg/state", "2");
    Serial.println("Set SG mode to 2 - Recommended ON");
  }
  else if (payload[0] == '3')
  {
    // Set SG 3 mode => SG1 = ACTIVE, SG2 = ACTIVE
    digitalWrite(PIN_SG1, SG_RELAY_ACTIVE_STATE);
    digitalWrite(PIN_SG2, SG_RELAY_ACTIVE_STATE);
    client.publish("espaltherma/sg/state", "3");
    Serial.println("Set SG mode to 3 - Forced ON");
  }
  else
  {
    Serial.printf("Unknown message: %s\n", payload);
  }
}
#endif

void callback(char *topic, byte *payload, unsigned int length)
{
  char buf[16]= "";
  const int cnt= min(length, sizeof(buf)-1);
  strncpy(buf, (const char*)payload, cnt);
  buf[cnt]= 0;

  mqttSerial.printf("Message arrived [%s[%i]] : %s\n", topic, length, buf);

#ifdef PIN_THERM_H1
  if (strcmp(topic, "espaltherma/THERM_MAINZ") == 0)
  {
    callbackTherm(PIN_THERM_H1, EEPROM_ADDR_H1, "espaltherma/STATE_MAINZ", 
                  payload, length);
  }
  else 
#endif
#ifdef PIN_THERM_H2
  if (strcmp(topic, "espaltherma/THERM_ADDZ") == 0)
  {
    callbackTherm(PIN_THERM_H2, EEPROM_ADDR_H2, "espaltherma/STATE_ADDZ", 
                  payload, length);
  }  
  else 
#endif
#ifdef PIN_SG1
  if (strcmp(topic, "espaltherma/sg/set") == 0)
  {
    callbackSg(payload, length);
  }
  else
#endif
  {
    Serial.printf("Unknown topic: %s\n", topic);
  }
}
