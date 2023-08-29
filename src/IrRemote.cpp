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

IRrecv irrecv(14, 1024, 50, true);
decode_results results;
ir_command command;
StaticJsonDocument<38400> jsonDocCommand;

String output, jsonCommand;

bool isLastReceived = false;
bool hasState;
byte pressedCount;

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
        // Check if we got an IR message that was to big for our capture buffer.
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

void IrRemote::sendIrCommand(decode_type_t protocol, byte degree, ir_command_type commandType,
                             ir_command_sending_option sending_option) {
}

void IrRemote::learnIrCommand(ir_command_type commandType) {
    Serial.println(F("Start learning new IR command"));
    Serial.println(F("Waiting for pressing Power On on remote"));
    bool isReceived = false;
    if (isLastReceived) {
        Serial.print(F("Last received command as raw array: "));
        displayRawArrayAsString();
        isLastReceived = false;
    }
    if (pressedCount < 3) {
        Serial.print(F("Button pressed "));
        Serial.print(pressedCount);
        Serial.println(F(" time"));
        ++pressedCount;
    } else {
        Serial.println(F("Reached 3 times. Now start saving command as json string"));
        jsonDocCommand.clear();
        saveIrCommand(ir_command_type::POWER_ON);
        pressedCount = 0;
    }
    while (!isReceived) {
        // Check if the IR code has been received.
        if (irrecv.decode(&results)) {
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

void IrRemote::saveIrCommand(ir_command_type commandType) {
    jsonDocCommand["protocol"] = typeToString(results.decode_type, false);
    switch (commandType) {
        case POWER_ON:
            jsonDocCommand["powerOnRawArray"] = getRawArrayAsString();
            jsonDocCommand["powerOnHex"] = getHexAsString();
            break;
        case POWER_OFF:
            jsonDocCommand["powerOffRawArray"] = getRawArrayAsString();
            jsonDocCommand["powerOffHex"] = getHexAsString();
            break;
        case INCREASE_TEMPERATURE:
            jsonDocCommand["temperatureIncrementRawArray"] = getRawArrayAsString();
            jsonDocCommand["temperatureIncrementHex"] = getHexAsString();
            break;
        case DECREASE_TEMPERATURE:
            jsonDocCommand["temperatureDecrementRawArray"] = getRawArrayAsString();
            jsonDocCommand["temperatureDecrementHex"] = getHexAsString();
            break;
        case USER_DEFINED_1:
            jsonDocCommand["userDefined1RawArray"] = getRawArrayAsString();
            jsonDocCommand["userDefined1Hex"] = getHexAsString();
            break;
        case USER_DEFINED_2:
            jsonDocCommand["userDefined2RawArray"] = getRawArrayAsString();
            jsonDocCommand["userDefined2Hex"] = getHexAsString();
            break;
        case USER_DEFINED_3:
            jsonDocCommand["userDefined3RawArray"] = getRawArrayAsString();
            jsonDocCommand["userDefined3Hex"] = getHexAsString();
            break;
        case USER_DEFINED_4:
            jsonDocCommand["userDefined4RawArray"] = getRawArrayAsString();
            jsonDocCommand["userDefined4Hex"] = getHexAsString();
            break;
        case USER_DEFINED_5:
            jsonDocCommand["userDefined5RawArray"] = getRawArrayAsString();
            jsonDocCommand["userDefined5Hex"] = getHexAsString();
            break;
        case USER_DEFINED_6:
            jsonDocCommand["userDefined6RawArray"] = getRawArrayAsString();
            jsonDocCommand["userDefined6Hex"] = getHexAsString();
            break;
        case USER_DEFINED_7:
            jsonDocCommand["userDefined7RawArray"] = getRawArrayAsString();
            jsonDocCommand["userDefined7Hex"] = getHexAsString();
            break;
        case USER_DEFINED_8:
            jsonDocCommand["userDefined8RawArray"] = getRawArrayAsString();
            jsonDocCommand["userDefined8Hex"] = getHexAsString();
            break;
    }
    jsonCommand = "";
    serializeJson(jsonDocCommand, jsonCommand);
    Serial.print(F("Serialized json string command:"));
    Serial.println(jsonCommand);
}

void IrRemote::getSavedIrCommand() {

}

void IrRemote::displayRawArrayAsString() {
    getRawArrayAsString();
    Serial.println(output);
}

String IrRemote::getRawArrayAsString() {
    output = "";
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
        if (i < results.rawlen - 1)
            output += kCommaSpaceStr;            // ',' not needed on the last one
        if (i % 2 == 0) output += ' ';  // Extra if it was even.
    }
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