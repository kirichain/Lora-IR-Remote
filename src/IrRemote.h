//
// Created by Tan on 8/24/2023.
//

#ifndef IRREMOTE_H
#define IRREMOTE_H

#include "Arduino.h"
#include <IRsend.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRutils.h>
#include <IRac.h>
#include <IRtext.h>
#include "ArduinoJson.h"
#include "SPIFFS.h"

enum ir_command_sending_option {
    AS_RAW,
    AS_HEX,
    AS_RAW_AND_HEX,
};
enum ir_command_type {
    POWER_ON,
    POWER_OFF,
    CHANGE_TEMPERATURE,
    INCREASE_TEMPERATURE,
    DECREASE_TEMPERATURE,
    USER_DEFINED_1,
    USER_DEFINED_2,
    USER_DEFINED_3,
    USER_DEFINED_4,
    USER_DEFINED_5,
    USER_DEFINED_6,
    USER_DEFINED_7,
    USER_DEFINED_8,
};

typedef struct ir_command {
    decode_type_t protocol;
    byte degree;
    ir_command_type commandType;
    uint16_t *raw;
} ir_command;

class IrRemote {
public:
    IrRemote();

    void start();

    void dump();

    void sendIrCommand(decode_type_t protocol, float degree, ir_command_type commandType,
                       ir_command_sending_option sending_option);

    void learnIrCommand(ir_command_type commandType, bool isSavedAfterLearning);

    void saveIrCommand(ir_command_type commandType, bool isClearedAfterSaving);

    void getSavedIrCommands();

    void displayRawArrayAsString();

    void writeToFile(String content);

    bool deserializeJsonString(String jsonString);

    bool allocateRawMemory(char *element);

    bool isFsAvailable(char *mode);

    decode_type_t matchProtocol(char* protocolAsString);

    String getRawArrayAsString();

    String getHexAsString();
};

#endif //IRREMOTE_H
