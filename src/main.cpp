#include "Arduino.h"
#include "IrRemote.h"

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

IrRemote remote;

void setup() {
    remote.start();
}

void loop() {
    //remote.dump();
    remote.learnIrCommand(ir_command_type::POWER_ON);
}