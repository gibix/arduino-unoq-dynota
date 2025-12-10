#include "Arduino_RouterBridge.h"
#include <stdio.h>
#include "llext_loader.h"
#include <zephyr/kernel.h>

void setup() {
    Serial.begin(115200);
    while (!Serial);
    
    pinMode(LED_BUILTIN, OUTPUT);

    Bridge.begin();

    Bridge.provide("load_elf_chunk", load_elf_chunk);
    Bridge.provide("start_llext_xfer", start_llext_xfer);
    Bridge.provide("stop_llext_xfer", stop_llext_xfer);
}

void loop() {
    delay(1000);
}
