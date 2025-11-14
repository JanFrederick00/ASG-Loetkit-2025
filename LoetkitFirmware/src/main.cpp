#include <Arduino.h>

#include "pins.h"
#include "ws2812.h"
#include "dmx_ch32v.h"
#include <Button.h>

#define WS2812_COUNT 9

enum class DmxMode : uint8_t
{
  DMX_3CH = 1,
  DMX_4CH = 2,
  DMX_27CH = 3,
  DMX_28CH = 4,
  DMX_36CH = 5,
  DMX_HSV = 6
};

const uint32_t IndicatorLEDOnTime = 20;

Button pushbutton(PIN_PUSHBUTTON);
RGB LEDData[WS2812_COUNT];

DmxMode currentMode = DmxMode::DMX_4CH;   // current DMX mode
bool shows_dmx_mode = false;              // are we currently showing the DMX mode on the main LEDs?
bool led_blink_on_signal_recv = false;    // blink whenever we get a DMX packet
uint64_t led_blink_tmr = 0;               // timer until the red LED should be turned off
bool standaloneMode = false;              // are we in standalone mode?
uint64_t pushbuttonHeldTime = 0;          // since when is the user holding the button?
uint16_t dmx_address = 0x00;              // current DMX address

// blink forever (error condition)
void err_blink(uint32_t ms)
{
  while (1)
  {
    digitalWrite(BUILTIN_LED, HIGH);
    delay(ms / 2);
    digitalWrite(BUILTIN_LED, LOW);
    delay(ms / 2);
  }
}

// read the 9-pin dip switch
uint16_t read_config_bits()
{
  uint16_t value = 0x00;
  for (int i = 0; i < NUM_CONFIG_BITS; ++i)
  {
    if (digitalRead(CONFIG_SWITCH_PINS[i]) == HIGH)
    {
      value |= 1 << i;
    }
  }
  return ~value;
}

// show the current DMX mode on the main LEDs
void IndicateDmxMode()
{
  RGB LEDDataTemp[WS2812_COUNT];

  memset(LEDDataTemp, 0x00, sizeof(RGB) * WS2812_COUNT);

  switch (currentMode)
  {
  case DmxMode::DMX_3CH:
    LEDDataTemp[0] = {255, 0, 0};
    LEDDataTemp[1] = {0, 255, 0};
    LEDDataTemp[2] = {0, 0, 255};
    break;
  case DmxMode::DMX_4CH:
    LEDDataTemp[0] = {255, 0, 0};
    LEDDataTemp[1] = {0, 255, 0};
    LEDDataTemp[2] = {0, 0, 255};
    LEDDataTemp[3] = {255, 255, 255};
    break;
  case DmxMode::DMX_27CH:
    LEDDataTemp[0] = {255, 0, 0};
    LEDDataTemp[1] = {0, 255, 0};
    LEDDataTemp[2] = {0, 0, 255};
    LEDDataTemp[8] = {255, 255, 255};
    break;
  case DmxMode::DMX_28CH:
    LEDDataTemp[0] = {255, 255, 255};
    LEDDataTemp[1] = {255, 0, 0};
    LEDDataTemp[2] = {0, 255, 0};
    LEDDataTemp[3] = {0, 0, 255};
    LEDDataTemp[8] = {255, 255, 255};
    break;
  case DmxMode::DMX_36CH:
    LEDDataTemp[0] = {255, 0, 0};
    LEDDataTemp[1] = {0, 255, 0};
    LEDDataTemp[2] = {0, 0, 255};
    LEDDataTemp[3] = {255, 255, 255};
    LEDDataTemp[7] = {255, 255, 255};
    LEDDataTemp[8] = {255, 255, 255};
    break;
  case DmxMode::DMX_HSV:
    LEDDataTemp[0] = {255, 0, 0};
    LEDDataTemp[1] = {255, 255, 255};
    LEDDataTemp[2] = {16, 16, 16};
    break;
  }

  ws2812_writeMultiple(LEDDataTemp, WS2812_COUNT);
}

// Write the LED data to the LEDs
inline void WriteLEDs()
{
  ws2812_writeMultiple(LEDData, WS2812_COUNT);
}

void setup()
{
  // Init GPIO
  ws2812_init();
  pinMode(PIN_PUSHBUTTON, INPUT_PULLUP);
  pinMode(BUILTIN_LED, OUTPUT);
  for (int i = 0; i < NUM_CONFIG_BITS; ++i)
    pinMode(CONFIG_SWITCH_PINS[i], INPUT_PULLUP);

  // Read CPU Frequency and enter error mode on wrong frequency.
  SystemCoreClockUpdate();
  if (SystemCoreClock != 48000000)
    err_blink(500); // Wrong frequency -> blink forever.

  // begin DMX receiving
  dmx_beginRX();

  // reset all LEDs to Black (off)
  memset(LEDData, 0x00, sizeof(RGB) * WS2812_COUNT);

  // if the button is held on startup, show DMX data with the red LED
  if (pushbutton.read() == Button::PRESSED) led_blink_on_signal_recv = true; 

  // show the current DMX mode on startup
  digitalWrite(BUILTIN_LED, HIGH);
  IndicateDmxMode();
  delay(500);
  digitalWrite(BUILTIN_LED, LOW);

  // LEDs off, waiting for Data.
  WriteLEDs();
}

void loop()
{
  if (standaloneMode)
  {
    uint16_t time_offset = (millis() % 3600) / 10;

    for (int y = 0; y < 3; ++y)
    {
      for (int x = 0; x < 3; ++x)
      {
        auto index = y * 3 + x;
        LEDData[index] = RGB_From_Hsv(
            time_offset + (10 * x) + (10 * y),
            StandaloneModeSaturationLookup[index % 9],
            StandaloneModeValueLookup[index % 9]);
      }
    }

    WriteLEDs();
  }
  else
  {
    // DMX address is one lower than the value selected (so the first address corresponds to setting 0x01)
    dmx_address = (read_config_bits() - 1) & 0x1FF;

    // new DMX data -> set LEDs
    if (dmx_newPacket() == 0xFF)
    {
      if (led_blink_on_signal_recv)
      {
        led_blink_tmr = millis() + IndicatorLEDOnTime;
        digitalWrite(BUILTIN_LED, HIGH);
      }

      switch (currentMode)
      {
      case DmxMode::DMX_28CH:
      {
        unsigned char buffer[WS2812_COUNT * 3 + 1];
        dmx_getValues(dmx_address, buffer, sizeof(buffer));

        for (int i = 0; i < WS2812_COUNT; ++i)
        {
          LEDData[i].R = ScaleUnorm(buffer[1 + (3 * i) + 0], buffer[0]);
          LEDData[i].G = ScaleUnorm(buffer[1 + (3 * i) + 1], buffer[0]);
          LEDData[i].B = ScaleUnorm(buffer[1 + (3 * i) + 2], buffer[0]);
        }
      }
      break;
      case DmxMode::DMX_27CH:
      {
        dmx_getValues(dmx_address, (unsigned char *)LEDData, 3 * WS2812_COUNT);
      }
      break;
      case DmxMode::DMX_36CH:
      {
        unsigned char buffer[WS2812_COUNT * 4];
        dmx_getValues(dmx_address, buffer, sizeof(buffer));
        for (int i = 0; i < WS2812_COUNT; ++i)
        {
          LEDData[i].R = ScaleUnorm(buffer[(4 * i) + 0], buffer[(4 * i) + 3]);
          LEDData[i].G = ScaleUnorm(buffer[(4 * i) + 1], buffer[(4 * i) + 3]);
          LEDData[i].B = ScaleUnorm(buffer[(4 * i) + 2], buffer[(4 * i) + 3]);
        }
      }
      break;
      case DmxMode::DMX_3CH:
      {
        unsigned char buffer[3];
        dmx_getValues(dmx_address, buffer, sizeof(buffer));
        for (int i = 0; i < WS2812_COUNT; ++i)
        {
          LEDData[i].R = buffer[0];
          LEDData[i].G = buffer[1];
          LEDData[i].B = buffer[2];
        }
      }
      break;
      case DmxMode::DMX_4CH:
      {
        unsigned char buffer[4];
        dmx_getValues(dmx_address, buffer, sizeof(buffer));

        buffer[0] = ScaleUnorm(buffer[0], buffer[3]);
        buffer[1] = ScaleUnorm(buffer[1], buffer[3]);
        buffer[2] = ScaleUnorm(buffer[2], buffer[3]);

        for (int i = 0; i < WS2812_COUNT; ++i)
        {
          LEDData[i].R = buffer[0];
          LEDData[i].G = buffer[1];
          LEDData[i].B = buffer[2];
        }
      }
      break;
      case DmxMode::DMX_HSV:
      {
        unsigned char buffer[3];
        dmx_getValues(dmx_address, buffer, sizeof(buffer));
        float fHue = (buffer[0] / 255.0f) * 360.0f;

        RGB color = RGB_From_Hsv((uint16_t)fHue, buffer[1], buffer[2]);
        for (int i = 0; i < WS2812_COUNT; ++i)
        {
          LEDData[i] = color;
        }
      }
      break;
      }

      if (!shows_dmx_mode)
        WriteLEDs();
    }
  }

  // if the button is pressed, cycle through channel modes
  if (pushbutton.pressed())
  {
    // next channel mode
    currentMode = (DmxMode)((uint8_t)currentMode + 1);
    if ((int)currentMode > (int)DmxMode::DMX_HSV)
      currentMode = DmxMode::DMX_3CH;

    IndicateDmxMode();
    shows_dmx_mode = true;
    pushbuttonHeldTime = millis();
  }
  else if (pushbutton.released())
  {
    // only resume showing the DMX data after pushbutton was released.
    shows_dmx_mode = false;
    WriteLEDs();
  }
  else if (pushbutton.read() == Button::PRESSED)
  {
    if ((millis() - pushbuttonHeldTime) > 10000)
    {
      standaloneMode = !standaloneMode;
    }
  }

  if (led_blink_tmr > 0 && millis() > led_blink_tmr)
  {
    led_blink_tmr = 0;
    digitalWrite(BUILTIN_LED, LOW);
  }
}
