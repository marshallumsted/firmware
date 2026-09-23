/*

E-Ink display driver
    - GDEQ031T10
    - Manufacturer: Good Display
    - Size: 3.1 inch
    - Resolution: 240px x 320px
    - Controller IC: UC8253

    Used by: t-deck-pro (V1.0 and V1.1).

    The board breaks out no reset line, so the panel is never put into deep sleep: waking from it
    needs that pin. The controller's software reset is a panel-setting write, not command 0x12,
    which on this family triggers a refresh.

*/

#pragma once

#ifdef MESHTASTIC_INCLUDE_NICHE_GRAPHICS

#include "configuration.h"

#include "./UC8175.h"

namespace NicheGraphics::Drivers
{

class GDEQ031T10 : public UC8175
{
  private:
    static constexpr uint32_t width = 240;
    static constexpr uint32_t height = 320;
    static constexpr UpdateTypes supported = (UpdateTypes)(FULL | FAST);

  public:
    GDEQ031T10() : UC8175(width, height, supported) {}

  protected:
    void reset() override;
    void configCommon() override;
    void configFull() override;
    void configFast() override;
    void detachFromUpdate() override;
    void finalizeUpdate() override;
};

} // namespace NicheGraphics::Drivers

#endif
