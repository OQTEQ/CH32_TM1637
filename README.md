# CH32_TM1637
# TM1637 Display Library for CH32 RISC-V (MounRiver Studio)

Optimized for CH32V003, CH32V203, CH32V307 — the world's cheapest RISC-V MCUs!

### Features
- Pure GPIO bit-banging
- 4-digit & 6-digit support
- Clock with blinking colon, decimal points, brightness
- Key reading if buttons present

Set your GPIO pins and digit count → flash and enjoy bright display on ultra-low-cost CH32!


int main(void)
{

    TM1637_Init();
    TM1637_SetBrightness(7);

    while(1)
    {
        TM1637_ShowNumber(1234, false);
                Delay_Ms(1000);
                TM1637_ShowNumberDP(5678, DOT_AFTER_3RD, false);  // 5.678
                Delay_Ms(1000);
    }
}
