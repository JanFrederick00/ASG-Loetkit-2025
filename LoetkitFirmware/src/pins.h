#pragma once
#include <Arduino.h>

const uint8_t BUILTIN_LED = PD4;
const uint8_t PIN_PUSHBUTTON = PD2;
const uint8_t NUM_CONFIG_BITS = 9;
const uint8_t CONFIG_SWITCH_PINS[NUM_CONFIG_BITS] = {PD3, PC7, PC6, PC5, PC4, PC3, PC2, PC1, PC0}; // LSB first
