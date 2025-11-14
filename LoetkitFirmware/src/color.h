#pragma once
#include <cstdint>
#include <stdlib.h>

struct RGB
{
    uint8_t R;
    uint8_t G;
    uint8_t B;
};


// multiply one byte by another, pretending that the value range is from 0 to 1 (unorm8)
inline uint8_t ScaleUnorm(uint8_t A, uint8_t SCALE)
{
  return (uint8_t)(((uint16_t)A * (uint16_t)SCALE) >> 8);
}

inline RGB RGB_From_Hsv(uint16_t hue, uint8_t saturation, uint8_t value) {
  while(hue >= 360) hue -= 360;

  uint8_t C = ScaleUnorm(value, saturation);

  int32_t fractional_hue = (((int32_t)hue)*255) / 60;
  
  uint8_t X = ScaleUnorm(C, (uint8_t)(255 - abs((fractional_hue % 511) - 255)));
  uint8_t m = value - C;

  uint8_t Rp = 0x00;
  uint8_t Gp = 0x00;
  uint8_t Bp = 0x00;

  uint8_t quadrant = hue / 60;
  switch(quadrant) {
    case 0: 
      Rp = C; Gp = X; Bp = 0x00;
    break;
    case 1: 
      Rp = X; Gp = C; Bp = 0x00;
    break;
    case 2: 
      Rp = 0x00; Gp = C; Bp = X;
    break;
    case 3: 
      Rp = 0x00; Gp = X; Bp = C;
    break;
    case 4: 
      Rp = X; Gp = 0x00; Bp = C;
    break;
    case 5: 
      Rp = C; Gp = 0x00; Bp = X;
    break;
  }

  return {(uint8_t)(Rp + m) , (uint8_t)(Gp + m), (uint8_t)(Bp + m)};
}

uint8_t StandaloneModeSaturationLookup[9] = {
  0xFF, 0xE0, 0xFF,
  0xE0, 0xC0, 0xE0,
  0xFF, 0xE0, 0xFF
};

uint8_t StandaloneModeValueLookup[9] = {
  0xFF, 0xFF, 0xFF,
  0xE0, 0xE0, 0xE0,
  0xC0, 0xC0, 0xC0,
};