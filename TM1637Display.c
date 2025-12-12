#include "TM1637Display.h"
#include "ch32v00x.h"   // or ch32v003.h / ch32v00x_delay.h depending on your template
//#include "ch32v00x_delay.h"   // This provides delay_us() and delay_ms()

static uint8_t g_brightness_cmd = 0x8F;
static bool colon_state = true;

static const uint8_t digitPattern[] = {
    0b00111111, 0b00000110, 0b01011011, 0b01001111,
    0b01100110, 0b01101101, 0b01111101, 0b00000111,
    0b01111111, 0b01101111, 0b01110111, 0b01111100,
    0b00111001, 0b01011110, 0b01111001, 0b01110001
};

static void tm_delay(void) {
    Delay_Us(5);  // ~5us delay - adjust if needed
}

static void tm_start(void) {
    GPIO_WriteBit(TM1637_DIO_PORT, TM1637_DIO_PIN, 1);
    GPIO_WriteBit(TM1637_CLK_PORT, TM1637_CLK_PIN, 1);
    tm_delay();
    GPIO_WriteBit(TM1637_DIO_PORT, TM1637_DIO_PIN, 0);
    tm_delay();
}

static void tm_stop(void) {
    GPIO_WriteBit(TM1637_CLK_PORT, TM1637_CLK_PIN, 0);
    tm_delay();
    GPIO_WriteBit(TM1637_DIO_PORT, TM1637_DIO_PIN, 0);
    tm_delay();
    GPIO_WriteBit(TM1637_CLK_PORT, TM1637_CLK_PIN, 1);
    tm_delay();
    GPIO_WriteBit(TM1637_DIO_PORT, TM1637_DIO_PIN, 1);
    tm_delay();
}

static void tm_write_byte(uint8_t byte) {
    for(uint8_t i = 0; i < 8; i++) {
        GPIO_WriteBit(TM1637_CLK_PORT, TM1637_CLK_PIN, 0);
        tm_delay();
        GPIO_WriteBit(TM1637_DIO_PORT, TM1637_DIO_PIN, (byte & 0x01));
        tm_delay();
        GPIO_WriteBit(TM1637_CLK_PORT, TM1637_CLK_PIN, 1);
        tm_delay();
        byte >>= 1;
    }
    // ACK - on CH32V003 we don't need to change mode for ACK (it's open-drain tolerant)
    GPIO_WriteBit(TM1637_CLK_PORT, TM1637_CLK_PIN, 0);
    tm_delay();
    GPIO_WriteBit(TM1637_CLK_PORT, TM1637_CLK_PIN, 1);
    tm_delay();
    GPIO_WriteBit(TM1637_CLK_PORT, TM1637_CLK_PIN, 0);
    tm_delay();
}

static uint8_t tm_read_byte(void) {
    uint8_t data = 0;

    // Set DIO as input (pull-up recommended in GPIO init)
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    GPIO_InitStructure.GPIO_Pin = TM1637_DIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  // or GPIO_Mode_IPU for pull-up
    GPIO_Init(TM1637_DIO_PORT, &GPIO_InitStructure);

    for(uint8_t i = 0; i < 8; i++) {
        GPIO_WriteBit(TM1637_CLK_PORT, TM1637_CLK_PIN, 0);
        tm_delay();
        if(GPIO_ReadInputDataBit(TM1637_DIO_PORT, TM1637_DIO_PIN)) data |= (1 << i);
        GPIO_WriteBit(TM1637_CLK_PORT, TM1637_CLK_PIN, 1);
        tm_delay();
    }

    // Send ACK
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(TM1637_DIO_PORT, &GPIO_InitStructure);
    GPIO_WriteBit(TM1637_DIO_PORT, TM1637_DIO_PIN, 0);
    tm_delay();
    GPIO_WriteBit(TM1637_CLK_PORT, TM1637_CLK_PIN, 0);
    tm_delay();
    GPIO_WriteBit(TM1637_CLK_PORT, TM1637_CLK_PIN, 1);
    tm_delay();
    GPIO_WriteBit(TM1637_CLK_PORT, TM1637_CLK_PIN, 0);

    return data;
}

void TM1637_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    // CLK pin
    GPIO_InitStructure.GPIO_Pin = TM1637_CLK_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(TM1637_CLK_PORT, &GPIO_InitStructure);

    // DIO pin
    GPIO_InitStructure.GPIO_Pin = TM1637_DIO_PIN;
    GPIO_Init(TM1637_DIO_PORT, &GPIO_InitStructure);

    GPIO_WriteBit(TM1637_CLK_PORT, TM1637_CLK_PIN, 1);
    GPIO_WriteBit(TM1637_DIO_PORT, TM1637_DIO_PIN, 1);

    TM1637_SetBrightness(7);
    TM1637_Clear();
}

void TM1637_SetBrightness(uint8_t b) {
    if(b > 7) b = 7;
    g_brightness_cmd = 0x88 | b;
    tm_start();
    tm_write_byte(g_brightness_cmd);
    tm_stop();
}

void TM1637_DisplayOn(void)   { g_brightness_cmd |= 0x08; TM1637_SetBrightness(g_brightness_cmd & 0x07); }
void TM1637_DisplayOff(void)  { g_brightness_cmd &= ~0x08; TM1637_SetBrightness(g_brightness_cmd & 0x07); }

void TM1637_Clear(void) {
    uint8_t blank[6] = {0};
    TM1637_SetRaw(blank);
}

void TM1637_SetRaw(const uint8_t data[6]) {
    tm_start();
    tm_write_byte(0x40);
    tm_stop();

    tm_start();
    tm_write_byte(0xC0);
    for(int i = 0; i < TM1637_DIGITS; i++) {
        tm_write_byte(data[i]);
    }
    tm_stop();

    tm_start();
    tm_write_byte(g_brightness_cmd);
    tm_stop();
}

uint8_t TM1637_EncodeDigit(uint8_t d) {
    return (d < 16) ? digitPattern[d] : 0;
}

void TM1637_ShowNumber(int num, bool leading_zeros) {
    TM1637_ShowNumberDP(num, 0, leading_zeros);
}

void TM1637_ShowNumberDP(int num, uint8_t dot_mask, bool leading_zeros) {
    if(num < 0) num = 0;
    const int max_num = (TM1637_DIGITS == 6) ? 999999 : 9999;
    if(num > max_num) num = max_num;

    uint8_t digits[6] = {0};
    bool show = leading_zeros;

    for(int pos = TM1637_DIGITS - 1; pos >= 0; pos--) {
        uint8_t d = num % 10;
        digits[pos] = digitPattern[d];

        if(dot_mask & (1 << (TM1637_DIGITS - 1 - pos))) digits[pos] |= SEG_DP;

        if(num == 0 && !show && pos > 0) digits[pos] = 0;
        else show = true;

        num /= 10;
    }
    TM1637_SetRaw(digits);
}

void TM1637_ShowTime(uint8_t h, uint8_t m, uint8_t s) {
#if TM1637_DIGITS == 6
    uint8_t digits[6];
    digits[0] = (h >= 10) ? digitPattern[h/10] : 0x00;
    digits[1] = digitPattern[h%10];
    if(colon_state) digits[1] |= SEG_DP;
    digits[2] = digitPattern[m/10];
    digits[3] = digitPattern[m%10];
    digits[4] = digitPattern[s/10];
    digits[5] = digitPattern[s%10];
    TM1637_SetRaw(digits);
#endif
}

// ... (other functions like ShowDate, single segment, key read - same as before)
