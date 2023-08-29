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

const uint16_t kRecvPin = 14;
const uint32_t kBaudRate = 115200;
const uint16_t kCaptureBufferSize = 1024;
const uint8_t kTimeout = 50;
const uint16_t kMinUnknownSize = 12;
const uint8_t kTolerancePercentage = kTolerance;  // kTolerance is normally 25%

const String commandTypes[13] = {"POWER_ON",
                                 "POWER_OFF",
                                 "CHANGE_TEMPERATURE",
                                 "INCREASE_TEMPERATURE",
                                 "DECREASE_TEMPERATURE",
                                 "USER_DEFINED_1",
                                 "USER_DEFINED_2",
                                 "USER_DEFINED_3",
                                 "USER_DEFINED_4",
                                 "USER_DEFINED_5",
                                 "USER_DEFINED_6",
                                 "USER_DEFINED_7",
                                 "USER_DEFINED_8",
};
const String protocols[128] = {"UNKNOWN", "UNUSED", "RC5", "RC6", "NEC", "SONY", "PANASONIC", "JVC", "SAMSUNG",
                               "WHYNTER",
                               "AIWA_RC_T501", "LG", "SANYO", "MITSUBISHI", "DISH", "SHARP", "COOLIX", "DAIKIN",
                               "DENON",
                               "KELVINATOR", "SHERWOOD", "MITSUBISHI_AC", "RCMM", "SANYO_LC7461", "RC5X", "GREE",
                               "PRONTO", "NEC_LIKE", "ARGO", "TROTEC", "NIKAI", "RAW", "GLOBALCACHE", "TOSHIBA_AC",
                               "FUJITSU_AC", "MIDEA", "MAGIQUEST", "LASERTAG", "CARRIER_AC", "HAIER_AC", "MITSUBISHI2",
                               "HITACHI_AC", "HITACHI_AC1", "HITACHI_AC2", "GICABLE", "HAIER_AC_YRW02", "WHIRLPOOL_AC",
                               "SAMSUNG_AC", "LUTRON", "ELECTRA_AC", "PANASONIC_AC", "PIONEER", "LG2", "MWM", "DAIKIN2",
                               "VESTEL_AC", "TECO", "SAMSUNG36", "TCL112AC", "LEGOPF", "MITSUBISHI_HEAVY_88",
                               "MITSUBISHI_HEAVY_152", "DAIKIN216", "SHARP_AC", "GOODWEATHER", "INAX", "DAIKIN160",
                               "NEOCLIMA",
                               "DAIKIN176", "DAIKIN128", "AMCOR", "DAIKIN152", "MITSUBISHI136", "MITSUBISHI112",
                               "HITACHI_AC424",
                               "SONY_38K", "EPSON", "SYMPHONY", "HITACHI_AC3", "DAIKIN64", "AIRWELL",
                               "DELONGHI_AC", "DOSHISHA", "MULTIBRACKETS", "CARRIER_AC40", "CARRIER_AC64",
                               "HITACHI_AC344", "CORONA_AC", "MIDEA24", "ZEPEAL", "SANYO_AC", "VOLTAS", "METZ",
                               "TRANSCOLD", "TECHNIBEL_AC", "MIRAGE", "ELITESCREENS", "PANASONIC_AC32", "MILESTAG2",
                               "ECOCLIM", "XMP",
                               "TRUMA", "HAIER_AC176", "TEKNOPOINT", "KELON", "TROTEC_3550", "SANYO_AC88", "BOSE",
                               "ARRIS",
                               "RHOSS", "AIRTON", "COOLIX48", "HITACHI_AC264", "KELON168", "HITACHI_AC296", "DAIKIN200",
                               "HAIER_AC160", "CARRIER_AC128", "TOTO", "CLIMABUTLER", "TCL96AC", "BOSCH144",
                               "SANYO_AC152", "DAIKIN312", "GORENJE", "WOWWEE", "CARRIER_AC84", "YORK"
};

class IrRemote {
public:
    IrRemote();

    void start();

    void dump();

    void sendIrCommand(decode_type_t protocol, byte degree, ir_command_type commandType,
                       ir_command_sending_option sending_option);

    void learnIrCommand(ir_command_type commandType);

    void saveIrCommand(ir_command_type commandType);

    void getSavedIrCommand();

    void displayRawArrayAsString();

    String getRawArrayAsString();

    String getHexAsString();
};

#endif //IRREMOTE_H
