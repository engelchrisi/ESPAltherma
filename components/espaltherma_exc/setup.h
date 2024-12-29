#pragma once

#define ARDUINO_M5Stick_C_Plus

#define FREQUENCY 30000 //query values every 30 sec

#if defined(ARDUINO_M5Stick_C) || defined(ARDUINO_M5Stick_C_Plus)
//Values used when **M5StickC** or **M5STickCPlus** environment is selected:
#define RX_PIN    36// Pin connected to the TX pin of X10A 
#define TX_PIN    26// Pin connected to the RX pin of X10A
#else 
//Default GPIO PINs for Serial2:
#define RX_PIN    16// Pin connected to the TX pin of X10A 
#define TX_PIN    18// Pin connected to the RX pin of X10A
#endif

#define MAX_MSG_SIZE 7120//max size of the json message sent in mqtt 


// Default to "Protocol I"
#ifndef PROTOCOL
#define PROTOCOL 'I'
#endif

#ifndef _countof
  #define _countof(arr) (sizeof(arr) / sizeof(arr[0]))
#endif