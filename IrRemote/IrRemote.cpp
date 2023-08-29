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
#include "IrRemote.h"

IRrecv irrecv(14, 1024, 50, true);
decode_results results;

IrRemote::IrRemote() {
    Serial.begin(115200);
    while (!Serial)  // Wait for the serial connection to be establised.
        delay(50);
    Serial.println(F(""));
    Serial.println(F("Serial check"));
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

void IrRemote::sendIrCommand(decode_type_t protocol, byte degree, bool isOn, bool isOff) {

}

void IrRemote::sendRaw() {

}

void IrRemote::learnIrCommand() {

}

void IrRemote::saveIrCommand(String command) {

}

void IrRemote::getIrCommandByFunctionName() {

}