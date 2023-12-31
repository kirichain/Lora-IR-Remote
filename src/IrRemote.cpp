//
// Created by Tan on 8/24/2023.
//

#include "Arduino.h"
#include <IRsend.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRutils.h>
#include <IRac.h>
#include <IRtext.h>
#include "ArduinoJson.h"
#include "IrRemote.h"
#include "SPIFFS.h"

const uint16_t kRecvPin = 14;
const uint32_t kBaudRate = 115200;
const uint16_t kCaptureBufferSize = 1024;
const uint8_t kTimeout = 50;
const uint16_t kMinUnknownSize = 12;
const uint8_t kTolerancePercentage = kTolerance;  // kTolerance is normally 25%
const uint16_t kIrLed = 4;  // The ESP GPIO pin to use that controls the IR LED.

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

IRac ac(kIrLed);  // Create an A/C object using GPIO to sending messages with.
IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
IRsend irsend(kIrLed);

decode_results results;
decode_type_t _protocol;
ir_command command;

StaticJsonDocument<50000> jsonDocCommand;
StaticJsonDocument<600> jsonDocRaw;
JsonArray rawArr;
uint16_t *raw;

String output, jsonCommand, fileContent;

bool isLastReceived = false;
bool hasState;

byte pressCount;

File file;

IrRemote::IrRemote() {
    jsonDocCommand["protocol"] = "";
    jsonDocCommand["powerOnRawArray"] = "";
    jsonDocCommand["powerOffRawArray"] = "";
    jsonDocCommand["temperatureIncrementRawArray"] = "";
    jsonDocCommand["temperatureDecrementRawArray"] = "";
    jsonDocCommand["userDefined1RawArray"] = "";
    jsonDocCommand["userDefined2RawArray"] = "";
    jsonDocCommand["userDefined3RawArray"] = "";
    jsonDocCommand["userDefined4RawArray"] = "";
    jsonDocCommand["userDefined5RawArray"] = "";
    jsonDocCommand["userDefined6RawArray"] = "";
    jsonDocCommand["userDefined7RawArray"] = "";
    jsonDocCommand["userDefined8RawArray"] = "";
    jsonDocCommand["powerOnHex"] = "";
    jsonDocCommand["powerOffHex"] = "";
    jsonDocCommand["temperatureIncrementHex"] = "";
    jsonDocCommand["temperatureDecrementHex"] = "";
    jsonDocCommand["userDefined1Hex"] = "";
    jsonDocCommand["userDefined2Hex"] = "";
    jsonDocCommand["userDefined3Hex"] = "";
    jsonDocCommand["userDefined4Hex"] = "";
    jsonDocCommand["userDefined5Hex"] = "";
    jsonDocCommand["userDefined6Hex"] = "";
    jsonDocCommand["userDefined7Hex"] = "";
    jsonDocCommand["userDefined8Hex"] = "";
    Serial.begin(115200);
    while (!Serial)  // Wait for the serial connection to be establised.
        delay(50);
    Serial.println(F(""));
    Serial.println(F("Serial check done"));
    // Perform a low level sanity checks that the compiler performs bit field
    // packing as we expect and Endianness is as we expect.
    assert(irutils::lowLevelSanityCheck() == 0);

    Serial.printf("\n" D_STR_IRRECVDUMP_STARTUP "\n", kRecvPin);
#if DECODE_HASH
    // Ignore messages with less than minimum on or off pulses.
    irrecv.setUnknownThreshold(kMinUnknownSize);
#endif  // DECODE_HASH
    irrecv.setTolerance(kTolerancePercentage);  // Override the default tolerance.
}

void IrRemote::start() {
    Serial.println(F("Starting receiver"));
    irrecv.enableIRIn();  // Start the receiver
}

void IrRemote::dump() {
    // Check if the IR code has been received.
    if (irrecv.decode(&results)) {
        // Display a crude timestamp.
        uint32_t now = millis();
        Serial.printf(D_STR_TIMESTAMP " : %06u.%03u\n", now / 1000, now % 1000);
        // Check if we got an IR message that was too big for our capture buffer.
        if (results.overflow)
            Serial.printf(D_WARN_BUFFERFULL "\n", kCaptureBufferSize);
        // Display the library version the message was captured with.
        Serial.println(D_STR_LIBRARY "   : v" _IRREMOTEESP8266_VERSION_STR "\n");
        // Display the tolerance percentage if it has been change from the default.
        if (kTolerancePercentage != kTolerance)
            Serial.printf(D_STR_TOLERANCE " : %d%%\n", kTolerancePercentage);
        // Display the basic output of what we found.
        Serial.print(resultToHumanReadableBasic(&results));
        // Display any extra A/C info if we have it.
        String description = IRAcUtils::resultAcToString(&results);
        if (description.length()) Serial.println(D_STR_MESGDESC ": " + description);
        yield();  // Feed the WDT as the text output can take a while to print.
#if LEGACY_TIMING_INFO
        // Output legacy RAW timing info of the result.
    Serial.println(resultToTimingInfo(&results));
    yield();  // Feed the WDT (again)
#endif  // LEGACY_TIMING_INFO
        // Output the results as source code
        Serial.println(resultToSourceCode(&results));
        Serial.println();    // Blank line between entries
        yield();             // Feed the WDT (again)
    }
}

void IrRemote::sendIrCommand(decode_type_t protocol, float degree, ir_command_type commandType,
                             ir_command_sending_option sending_option) {
    if (protocol == decode_type_t::UNUSED) {
        Serial.println(F("Unused protocol param set"));
        protocol = _protocol;
    }
    if (protocol != decode_type_t::UNKNOWN) {
        Serial.println(F("Protocol is defined. Checking if we can send"));
        if (ac.isProtocolSupported(protocol)) {
            Serial.println("Protocol " + String(protocol) + " / " +
                           typeToString(protocol) + " is supported.");
            switch (commandType) {
                case POWER_ON:
                    ac.next.power = true;
                    break;
                case POWER_OFF:
                    ac.next.power = false;
                    break;
                case CHANGE_TEMPERATURE:
                    ac.next.degrees = degree;
                    break;
            }
            ac.next.mode = stdAc::opmode_t::kCool;  // Run in cool mode initially.
            ac.next.fanspeed = stdAc::fanspeed_t::kMedium;  // Start the fan at medium.
            ac.next.swingv = stdAc::swingv_t::kOff;  // Don't swing the fan up or down.
            ac.next.swingh = stdAc::swingh_t::kOff;  // Don't swing the fan left or right.
            ac.next.protocol = protocol;  // Change the protocol used.
            ac.sendAc();  // Have the IRac class create and send a message.
            Serial.println(F("Command has been sent"));
        } else {
            Serial.println(F("Protocol is not supported. Abort sending"));
        }
    } else {
        char *elementName = NULL;
        Serial.println(F("Unknown protocol. Start sending raw/hex with associated command from stored list"));
        switch (commandType) {
            case POWER_ON:
                elementName = (char *) "powerOnRawArray";
                break;
            case POWER_OFF:
                elementName = (char *) "powerOffRawArray";
                break;
            case INCREASE_TEMPERATURE:
                elementName = (char *) "temperatureIncrementRawArray";
                break;
            case DECREASE_TEMPERATURE:
                elementName = (char *) "temperatureDecrementRawArray";
                break;
            case USER_DEFINED_1:
                elementName = (char *) "userDefined1RawArray";
                break;
            case USER_DEFINED_2:
                elementName = (char *) "userDefined2RawArray";
                break;
            case USER_DEFINED_3:
                elementName = (char *) "userDefined3RawArray";
                break;
            case USER_DEFINED_4:
                elementName = (char *) "userDefined4RawArray";
                break;
            case USER_DEFINED_5:
                elementName = (char *) "userDefined5RawArray";
                break;
            case USER_DEFINED_6:
                elementName = (char *) "userDefined6RawArray";
                break;
            case USER_DEFINED_7:
                elementName = (char *) "userDefined7RawArray";
                break;
            case USER_DEFINED_8:
                elementName = (char *) "userDefined8RawArray";
                break;
        }
        if (elementName != nullptr) {
            if (allocateRawMemory(elementName)) {
                irsend.sendRaw(raw, rawArr.size(), 38);
                Serial.println(F("Sent raw data"));
                delete[] raw;
                Serial.println(F("Command has been sent"));
            }
        }
    }
}

void IrRemote::learnIrCommand(ir_command_type commandType, bool isSavedAfterLearning) {
    Serial.println(F("Start learning new IR command"));
    Serial.println(F("Waiting for pressing button on remote"));
    while (pressCount < 3) {
        bool isReceived = false;

        if (isLastReceived) {
            Serial.print(F("Last received command as raw array: "));
            displayRawArrayAsString();
            isLastReceived = false;
        }

        while (!isReceived) {
            // Check if the IR code has been received.
            if (irrecv.decode(&results)) {
                ++pressCount;
                Serial.print(F("Button pressed "));
                Serial.print(pressCount);
                Serial.println(F(" time"));

                isLastReceived = true;
                hasState = hasACState(results.decode_type);
                // The capture has stopped at this point.
                decode_type_t protocol = results.decode_type;
                if (protocol == decode_type_t::UNKNOWN) {
                    Serial.println(F("Unknown protocol. Will save raw data"));
                } else {
                    Serial.println("Protocol: " + typeToString(protocol, false));
                }
                Serial.print(F("Raw data: "));
                displayRawArrayAsString();

                Serial.print(F("Command as HEX: 0x"));
                output = uint64ToString(results.command, 16);
                Serial.println(output);

                yield();  // Feed the WDT as the text output can take a while to print.
                // Output the results as source code
                isReceived = true;
                yield();             // Feed the WDT (again)
            }
        }
    }
    if (isSavedAfterLearning) {
        Serial.println(F("Reached 3 times. Now start saving command as json string to file"));
        saveIrCommand(commandType, true);
    } else {
        Serial.println(F("Reached 3 times. Learning completes"));
        saveIrCommand(commandType, false);
    }
    pressCount = 0;
}

void IrRemote::saveIrCommand(ir_command_type commandType, bool isClearedAfterSaving) {
    jsonDocCommand["protocol"] = typeToString(results.decode_type, false);
    getRawArrayAsString();
    rawArr = jsonDocRaw.as<JsonArray>();
    switch (commandType) {
        case POWER_ON:
            jsonDocCommand["powerOnRawArray"] = rawArr;
            jsonDocCommand["powerOnHex"] = getHexAsString();
            break;
        case POWER_OFF:
            jsonDocCommand["powerOffRawArray"] = rawArr;
            jsonDocCommand["powerOffHex"] = getHexAsString();
            break;
        case INCREASE_TEMPERATURE:
            jsonDocCommand["temperatureIncrementRawArray"] = rawArr;
            jsonDocCommand["temperatureIncrementHex"] = getHexAsString();
            break;
        case DECREASE_TEMPERATURE:
            jsonDocCommand["temperatureDecrementRawArray"] = rawArr;
            jsonDocCommand["temperatureDecrementHex"] = getHexAsString();
            break;
        case USER_DEFINED_1:
            jsonDocCommand["userDefined1RawArray"] = rawArr;
            jsonDocCommand["userDefined1Hex"] = getHexAsString();
            break;
        case USER_DEFINED_2:
            jsonDocCommand["userDefined2RawArray"] = rawArr;
            jsonDocCommand["userDefined2Hex"] = getHexAsString();
            break;
        case USER_DEFINED_3:
            jsonDocCommand["userDefined3RawArray"] = rawArr;
            jsonDocCommand["userDefined3Hex"] = getHexAsString();
            break;
        case USER_DEFINED_4:
            jsonDocCommand["userDefined4RawArray"] = rawArr;
            jsonDocCommand["userDefined4Hex"] = getHexAsString();
            break;
        case USER_DEFINED_5:
            jsonDocCommand["userDefined5RawArray"] = rawArr;
            jsonDocCommand["userDefined5Hex"] = getHexAsString();
            break;
        case USER_DEFINED_6:
            jsonDocCommand["userDefined6RawArray"] = rawArr;
            jsonDocCommand["userDefined6Hex"] = getHexAsString();
            break;
        case USER_DEFINED_7:
            jsonDocCommand["userDefined7RawArray"] = rawArr;
            jsonDocCommand["userDefined7Hex"] = getHexAsString();
            break;
        case USER_DEFINED_8:
            jsonDocCommand["userDefined8RawArray"] = rawArr;
            jsonDocCommand["userDefined8Hex"] = getHexAsString();
            break;
    }
    jsonCommand = "";
    serializeJson(jsonDocCommand, jsonCommand);
    Serial.print(F("Serialized json string command:"));
    Serial.println(jsonCommand);

    if (isClearedAfterSaving) {
        Serial.println(F("Start saving command to file"));

        if (isFsAvailable((char *) "w")) {
            writeToFile(jsonCommand);
        }

        jsonDocCommand.clear();
    }
}

void IrRemote::getSavedIrCommands() {
    if (isFsAvailable((char *) "r")) {
        Serial.println(F("Command storage reading is ok. Start getting command"));
        Serial.println(F("Storage list: "));
        Serial.println(fileContent);
        if (deserializeJsonString(fileContent)) {
            Serial.print(F("Protocol: "));
            Serial.println(jsonDocCommand["protocol"].as<const char *>());
            Serial.print(F("Power On Hex: "));
            Serial.println(jsonDocCommand["powerOnHex"].as<const char *>());
            Serial.print(F("Power Off Hex: "));
            Serial.println(jsonDocCommand["powerOffHex"].as<const char *>());

            _protocol = matchProtocol((char *) jsonDocCommand["protocol"].as<const char *>());
        }
    }
}

void IrRemote::displayRawArrayAsString() {
    getRawArrayAsString();
    Serial.println(output);
}

bool IrRemote::deserializeJsonString(String jsonString) {
    jsonDocCommand.clear();

    DeserializationError error = deserializeJson(jsonDocCommand, jsonString);

    if (error) {
        Serial.print(F("Deserialize json string failed: "));
        Serial.println(error.f_str());
        return false;
    } else {
        Serial.println(F("Deserialize json string successfully"));
    }
    return true;
}

void IrRemote::writeToFile(String content) {
    file.print(content);
    Serial.println(F("Wrote to file"));
    file.close();
}

bool IrRemote::allocateRawMemory(char *element) {
    rawArr = jsonDocCommand[element].as<JsonArray>();
    Serial.print(element);
    Serial.print(F(" count: "));
    Serial.println(rawArr.size());
    raw = new uint16_t[rawArr.size()];
    if (raw == nullptr) {
        Serial.println(F("Out of RAM. Stop sending"));
        return false;
    }
    copyArray(rawArr, raw, rawArr.size());
    for (int i = 0; i < sizeof(raw) - 1; i++) {
        Serial.println(raw[i]);
    }
    return true;
}

bool IrRemote::isFsAvailable(char *mode) {
    fileContent = "";

    if (!SPIFFS.begin(true)) {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return false;
    }

    file = SPIFFS.open("/data.txt", (char *) mode);
    if (!file) {
        Serial.println("Failed to open file for reading");
        return false;
    } else {
        if (mode == "r") {
            Serial.println("File Content:");
            //while (file.available()) {
            //    Serial.write(file.read());
            //}
            while (file.available()) {
                fileContent += char(file.read());
            }
            file.close();
        } else if (mode == "w") {
            Serial.println(F("File is opened in writing mode"));
        }
        return true;
    }
}

decode_type_t IrRemote::matchProtocol(char *protocolAsString) {
    byte protocolIndex;

    Serial.print(F("Matching protocol with string: "));
    Serial.println((char *) protocolAsString);
    for (int i = 0; i <= 127; i++) {
        if (protocols[i] == (char *) protocolAsString) {
            protocolIndex = i - 1;
            break;
        }
    }
    for (int i = 0; i != kLastDecodeType; i++) {
        decode_type_t protocol = (decode_type_t) i;
        if (protocol == protocolIndex) {
            Serial.print(F("Matched with index = "));
            Serial.println(protocol);
            return protocol;
        }
    }
    return decode_type_t::UNKNOWN;
}

String IrRemote::getRawArrayAsString() {
    output = "[";
    for (uint16_t i = 1; i < results.rawlen; i++) {
        uint32_t usecs;
        for (usecs = results.rawbuf[i] * kRawTick; usecs > UINT16_MAX;
             usecs -= UINT16_MAX) {
            output += uint64ToString(UINT16_MAX);
            if (i % 2)
                output += F(", 0,  ");
            else
                output += F(",  0, ");
        }
        output += uint64ToString(usecs, 10);
        jsonDocRaw.add(usecs);
        if (i < results.rawlen - 1)
            output += kCommaSpaceStr;            // ',' not needed on the last one
        if (i % 2 == 0) output += ' ';  // Extra if it was even.
    }
    output += "]";
    return output;
}

String IrRemote::getHexAsString() {
    output = "";
    // Now dump "known" codes
    if (results.decode_type != UNKNOWN) {
        if (hasState) {
#if DECODE_AC
            uint16_t nbytes = ceil(static_cast<float>(results.bits) / 8.0);
            for (uint16_t i = 0; i < nbytes; i++) {
                output += F("0x");
                if (results.state[i] < 0x10) output += '0';
                output += uint64ToString(results.state[i], 16);
                if (i < nbytes - 1) output += kCommaSpaceStr;
            }
#endif  // DECODE_AC
        } else {
            // Simple protocols
            // Some protocols have an address &/or command.
            // NOTE: It will ignore the atypical case when a message has been
            // decoded but the address & the command are both 0.
            if (results.address > 0 || results.command > 0) {
                output += F("0x");
                output += uint64ToString(results.command, 16);
            }
        }
    }
    return output;
}