#include <Arduino.h>

#define CHANNEL_MODE_THIRTYSIX
//#define CHANNEL_MODE_TWENTYEIGHT
//#define CHANNEL_MODE_TWENTYSEVEN
//#define CHANNEL_MODE_THREE
//#define CHANNEL_MODE_FOUR

#include "ws2812.h"
#include "dmx_ch32v.h"

#define WS2812_COUNT 9

const uint8_t BUILTIN_LED = PD4;
const uint8_t PIN_DMX_SEND = PD2;

const uint8_t NUM_CONFIG_BITS = 9;

const uint8_t CONFIG_SWITCH_PINS[NUM_CONFIG_BITS] = { PC0, PC1, PC2, PC3, PC4, PC5, PC6, PC7, PD3}; // LSB first

RGB LEDData[WS2812_COUNT];

void err_blink(uint32_t ms)
{
  while (1)
  {
    digitalWrite(BUILTIN_LED, HIGH);
    delay(ms/2);
    digitalWrite(BUILTIN_LED, LOW);
    delay(ms/2);
  }
}

void init_gpio(){
  ws2812_init();
  pinMode(BUILTIN_LED, OUTPUT);

  // Send DMX mode to RECEIVE
  pinMode(PIN_DMX_SEND, OUTPUT);
  digitalWrite(PIN_DMX_SEND, LOW);

  for(int i = 0; i<NUM_CONFIG_BITS; ++i){
    pinMode(CONFIG_SWITCH_PINS[i], INPUT_PULLUP);
  }
  
}

uint16_t read_config_bits() {
  uint16_t value = 0x00;
  for(int i = 0; i<NUM_CONFIG_BITS; ++i) {
    if(digitalRead(CONFIG_SWITCH_PINS[i]) == HIGH) {
      value |= 1 << i;
    }
  }
  return value;
}

void setup()
{
  init_gpio();

  SystemCoreClockUpdate();
  if (SystemCoreClock != 48000000)
    err_blink(500);

  dmx_beginRX();
}

inline uint8_t ScaleUnorm(uint8_t A, uint8_t SCALE) {
  return (uint8_t)(((uint16_t)A * (uint16_t)SCALE) >> 8);
}

uint16_t dmx_address = 0x00;

int offset = 0;
int last_update = 0;
void loop()
{
  dmx_address = (read_config_bits() - 1) & 0x1FF; // Switch position is 0x01 for the first DMX channel, while we use the index 0x00 for this. 
  
  if(dmx_newPacket() == 0xFF){

    #if defined( CHANNEL_MODE_TWENTYEIGHT)
      unsigned char buffer[WS2812_COUNT * 3 + 1];
      dmx_getValues(dmx_address, buffer, sizeof(buffer));

      for(int i = 0; i<WS2812_COUNT; ++i) {
        LEDData[i].R = ScaleUnorm(buffer[1 + (3*i) + 0], buffer[0]);
        LEDData[i].G = ScaleUnorm(buffer[1 + (3*i) + 1], buffer[0]);
        LEDData[i].B = ScaleUnorm(buffer[1 + (3*i) + 2], buffer[0]);
      }
    #elif defined(CHANNEL_MODE_TWENTYSEVEN)
      dmx_getValues(dmx_address, (unsigned char*)LEDData, 3 * WS2812_COUNT);
    #elif defined(CHANNEL_MODE_THIRTYSIX) 
      unsigned char buffer[WS2812_COUNT * 4];
      dmx_getValues(dmx_address, buffer, sizeof(buffer));
      for(int i = 0; i<WS2812_COUNT; ++i) {
        LEDData[i].R = ScaleUnorm(buffer[(4*i) + 0], buffer[(4*i) + 3]);
        LEDData[i].G = ScaleUnorm(buffer[(4*i) + 1], buffer[(4*i) + 3]);
        LEDData[i].B = ScaleUnorm(buffer[(4*i) + 2], buffer[(4*i) + 3]);
      }
    #elif defined(CHANNEL_MODE_THREE) 
      unsigned char buffer[3];
      dmx_getValues(dmx_address, buffer, sizeof(buffer));
      for(int i = 0; i<WS2812_COUNT; ++i) {
        LEDData[i].R = buffer[0];
        LEDData[i].G = buffer[1];
        LEDData[i].B = buffer[2];
      }
    #else   // CHANNEL_MODE_FOUR
      unsigned char buffer[4];
      dmx_getValues(dmx_address, buffer, sizeof(buffer));

      buffer[0] = ScaleUnorm(buffer[0], buffer[3]);
      buffer[1] = ScaleUnorm(buffer[1], buffer[3]);
      buffer[2] = ScaleUnorm(buffer[2], buffer[3]);

      for(int i = 0; i<WS2812_COUNT; ++i) {
        LEDData[i].R = buffer[0];
        LEDData[i].G = buffer[1];
        LEDData[i].B = buffer[2];
      }
    #endif

   
    ws2812_writeMultiple(LEDData, WS2812_COUNT);
  }

}
