#ifdef MESHTASTIC_INCLUDE_NICHE_GRAPHICS

#include "./GDEQ031T10.h"

using namespace NicheGraphics::Drivers;

// Command values follow GxEPD2_310_GDEQ031T10, which drives this panel on the T-Deck Pro.

void GDEQ031T10::reset()
{
    if (pin_rst != (uint8_t)-1) {
        digitalWrite(pin_rst, LOW);
        delay(20);
        digitalWrite(pin_rst, HIGH);
        delay(20);
        wait(3000);
        return;
    }

    // Software reset lives in the panel setting register here; 0x12 would start a refresh instead
    sendCommand(0x00);
    sendData(0x1E);
    sendData(0x0D);
    delay(10);
    wait(3000);
}

void GDEQ031T10::configCommon()
{
    sendCommand(0x00); // Panel setting: KW mode, scan direction, booster on
    sendData(0x1F);
    sendData(0x0D);
}

void GDEQ031T10::configFull()
{
    sendCommand(0xE0); // Cascade setting: use the forced temperature below
    sendData(0x02);

    sendCommand(0xE5); // Force temperature, which selects the waveform
    sendData(0x5A);

    sendCommand(0x50); // VCOM and data interval
    sendData(0x97);

    powerOn();
}

void GDEQ031T10::configFast()
{
    sendCommand(0xE0);
    sendData(0x02);

    sendCommand(0xE5); // A higher temperature gives the shorter partial waveform
    sendData(0x79);

    sendCommand(0x50);
    sendData(0xD7);

    powerOn();
}

void GDEQ031T10::detachFromUpdate()
{
    switch (updateType) {
    case FAST:
        return beginPolling(50, 400);
    case FULL:
    default:
        return beginPolling(100, 1100);
    }
}

void GDEQ031T10::finalizeUpdate()
{
    // No deep sleep: without a reset line the panel could not be woken again
    powerOff();
}

#endif // MESHTASTIC_INCLUDE_NICHE_GRAPHICS
