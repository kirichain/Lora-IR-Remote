#include "Arduino.h"
#include "IrRemote.h"

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

IrRemote remote;

void setup() {
    remote.start();
    //remote.getSavedIrCommands();
    //remote.sendIrCommand(UNUSED, 0, POWER_ON, AS_RAW);
}

void loop() {
    //remote.dump();
    remote.learnIrCommand(ir_command_type::POWER_ON, false);
    remote.learnIrCommand(ir_command_type::POWER_OFF, true);
}