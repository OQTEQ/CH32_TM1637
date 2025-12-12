#ifndef TM1637DISPLAY_H
#define TM1637DISPLAY_H

#include "ch32v00x.h"      // Adjust based on your chip: ch32v20x.h, ch32v00x.h, etc.
#include <stdint.h>
#include <stdbool.h>

/* ====================== USER CONFIGURATION ====================== */
// Example pins - change to your actual wiring
#define TM1637_CLK_PORT    GPIOC
#define TM1637_CLK_PIN     GPIO_Pin_0

#define TM1637_DIO_PORT    GPIOC
#define TM1637_DIO_PIN     GPIO_Pin_1

// Select display type
#define TM1637_DIGITS      6     // Change to 4 for 4-digit module

#if (TM1637_DIGITS != 4 && TM1637_DIGITS != 6)
#error "TM1637_DIGITS must be 4 or 6"
#endif

/* ====================== SEGMENT & DOT DEFINES ====================== */
#define SEG_A  0b00000001
#define SEG_B  0b00000010
#define SEG_C  0b00000100
#define SEG_D  0b00001000
#define SEG_E  0b00010000
#define SEG_F  0b00100000
#define SEG_G  0b01000000
#define SEG_DP 0b10000000

#define DOT_AFTER_1ST   0b000001
#define DOT_AFTER_2ND   0b000010
#define DOT_AFTER_3RD   0b000100
#define DOT_AFTER_4TH   0b001000
#define DOT_AFTER_5TH   0b010000
#define DOT_AFTER_6TH   0b100000

/* ====================== PUBLIC FUNCTIONS ====================== */
void TM1637_Init(void);
void TM1637_SetBrightness(uint8_t level);  // 0-7
void TM1637_DisplayOn(void);
void TM1637_DisplayOff(void);
void TM1637_Clear(void);

void TM1637_ShowNumber(int num, bool leading_zeros);
void TM1637_ShowNumberDP(int num, uint8_t dot_mask, bool leading_zeros);

void TM1637_ShowTime(uint8_t hours, uint8_t minutes, uint8_t seconds);
void TM1637_ShowDate(uint8_t day, uint8_t month, uint16_t year);

void TM1637_ShowSingleDigit(uint8_t pos);
void TM1637_ShowSingleSegment(uint8_t pos, uint8_t segment);

void TM1637_SetRaw(const uint8_t data[6]);
uint8_t TM1637_EncodeDigit(uint8_t digit);

uint8_t TM1637_ReadKeys(void);
uint8_t TM1637_GetPressedKey(void);
bool    TM1637_IsKeyPressed(uint8_t key_num);

#endif
