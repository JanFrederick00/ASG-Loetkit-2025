// Adapted from: https://github.com/Blakesands/CH32V003

#include <Arduino.h>

// Important: The WS2812-chain is attached to pin PD0.
#include "color.h"

void ws2812_init()
{
    static_assert(sizeof(RGB) == 3);

    GPIO_InitTypeDef GPIO_InitStructure = {0};
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
}

// The LEDs work by sending a high pulse followed by a space until the next positive pulse.
// the length of the high pulse and the space between decides if the bit is a 1 or 0.
// the exact timings vary by LED model number.
// this uses the timings from here: https://cdn-shop.adafruit.com/datasheets/WS2812.pdf
// to send a "1" bit, the high pulse must be 700nS (+-150nS), and the low pulse 600nS (+-100nS)
// to send a "0" bit, the high pulse must be 250nS (+-150nS), and the low pulse 800nS (+-100nS)
// each "nop" assembly instruction takes exactly one cycle (on RISC-V); @48MHz = 20nS/cycle
// Some time is taken up by the loop around this function, so the amount of nop instructions
// was fine-tuned with an oscilloscope.
inline void ws2812_SendBit(uint8_t bit)
{
    
    if (bit)
    {
        //// Send a 1 bit
        GPIOD->BSHR = 1 << 0; // Set PD0 HIGH

        asm volatile(
            "nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; "
            "nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; "
            "nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; "
            "nop; nop; nop; nop; nop; nop; nop;");

        GPIOD->BCR = 1 << 0; // Set PD0 LOW

        asm volatile(
            "nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; "
            "nop; nop; nop; nop; nop;");
    }
    else
    {
        // Send a 0 bit
        GPIOD->BSHR = 1 << 0; // Set PD0 HIGH

        asm volatile(
            "nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; "
            "nop; nop;");

        GPIOD->BCR = 1 << 0; // Set PD0 LOW

        asm volatile(
            "nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; "
            "nop; nop; nop; nop; nop; nop; nop; nop; ");
    }
}

// write a single byte to the LEDs. 
// bytes are written MSB first
inline void ws2812_writeByte(uint8_t u)
{
    // if an interrupt occurs (for example from the UART) while we are sending data to the LEDs, execution is paused for a short time.
    // this may turn a "0" bit into a "1".
    // we cannot disable interrupts during the whole period of sending data, because that would skip interrupts.
    __disable_irq();  
    for (int8_t i = 7; i >= 0; i--)
    {
        ws2812_SendBit((u >> i) & 1);
    }
    __enable_irq();
}

// write 3 bytes for a single LED.
inline void ws2812_writeColor(const RGB &rgb)
{
    ws2812_writeByte(rgb.G);
    ws2812_writeByte(rgb.R);
    ws2812_writeByte(rgb.B);
}


void wait_us(uint32_t us)
{
    while (us > 0)
    {
        --us;
        // 1x nop = 20ns @48MHz
        // 50x20ns = 1us
        asm volatile(
            "nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; "
            "nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; "
            "nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; "
            "nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; "
            "nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; ");
    }
}

void ws2812_writeMultiple(const RGB *pData, const uint32_t numLeds)
{
    //__disable_irq();
    for (uint32_t i = 0; i < numLeds; ++i)
    {
        ws2812_writeColor(pData[i]);
    }
    //__enable_irq();
    
    wait_us(100);
}