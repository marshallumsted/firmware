#include "variant.h"
#include "IoExpanderXL9555.hpp"
extern IoExpanderXL9555 io;

void earlyInitVariant()
{
    pinMode(LORA_CS, OUTPUT);
    digitalWrite(LORA_CS, HIGH);
    pinMode(SDCARD_CS, OUTPUT);
    digitalWrite(SDCARD_CS, HIGH);
    pinMode(TFT_CS, OUTPUT);
    digitalWrite(TFT_CS, HIGH);
    pinMode(KB_INT, INPUT_PULLUP);
    // io expander
    io.begin(Wire, XL9555_SLAVE_ADDRESS0, SDA, SCL);
    io.pinMode(EXPANDS_DRV_EN, OUTPUT);
    io.digitalWrite(EXPANDS_DRV_EN, HIGH);
    io.pinMode(EXPANDS_AMP_EN, OUTPUT);
    io.digitalWrite(EXPANDS_AMP_EN, LOW);
    io.pinMode(EXPANDS_LORA_EN, OUTPUT);
    io.digitalWrite(EXPANDS_LORA_EN, HIGH);
    io.pinMode(EXPANDS_GPS_EN, OUTPUT);
    io.digitalWrite(EXPANDS_GPS_EN, HIGH);
    // EXPANDS_GPS_RST was declared in variant.h but never driven, so the
    // expander left it as an input (high-Z) and the GNSS module stayed in
    // reset. The probe then found nothing and the GPS was powered straight
    // back down ("GPS power state ACTIVE -> OFF" in the same second).
    // Pulse reset low, then release and give the module time to come up.
    io.pinMode(EXPANDS_GPS_RST, OUTPUT);
    io.digitalWrite(EXPANDS_GPS_RST, LOW);
    delay(10);
    io.digitalWrite(EXPANDS_GPS_RST, HIGH);
    delay(150);
    io.pinMode(EXPANDS_KB_EN, OUTPUT);
    io.digitalWrite(EXPANDS_KB_EN, HIGH);
    io.pinMode(EXPANDS_SD_EN, OUTPUT);
    io.digitalWrite(EXPANDS_SD_EN, HIGH);
    io.pinMode(EXPANDS_GPIO_EN, OUTPUT);
    io.digitalWrite(EXPANDS_GPIO_EN, HIGH);
    io.pinMode(EXPANDS_SD_PULLEN, INPUT);
}