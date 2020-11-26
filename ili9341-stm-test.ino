 #include "Adafruit_ILI9341_STM.h"
#include <SPI.h>

SPIClass             spi_1 { 1        };
HardwareTimer        tim_1 { 1        };
Adafruit_ILI9341_STM tft   { PA4, PB0 };

void timer_isr() {
 // In a real program, we might be doing something useful like generating
 // audio samples, but to keep this simple we'll just waste a few cycles:
 
 for (volatile size_t ix = 0; ix < 64; ix++);
}

void setup() {
 Serial.begin(115200);

 tft.begin(spi_1);
 tft.fillScreen(ILI9341_BLACK);

 // Set up a timer to call sample_rate() at 48khz:
 tim_1.pause();
 tim_1.setPrescaleFactor(F_CPU/48000/2); 
 tim_1.setOverflow(1);
 tim_1.setChannel1Mode(TIMER_OUTPUT_COMPARE);
 tim_1.setCompare(TIMER_CH1, 0);
 tim_1.attachCompare1Interrupt(timer_isr);
 tim_1.refresh();
 tim_1.resume();
}
  
void loop() {
 constexpr uint16_t draw_area_width = 240;
 static uint16_t    column          = 0; 

 // We're going accumulate the time taken by 8192 writes
 // and take the average:
 
 constexpr uint8_t  fx_shift        = 13;
 constexpr size_t   max_count       = 1 << fx_shift;
 static size_t      counter         = 0;
 static uint32_t    accum           = 0;

 uint32_t           start           = micros(); 
 
 tft.drawFastVLine(
  column % draw_area_width,
  0,
  column % draw_area_width,
  ILI9341_YELLOW
 );

 accum   += micros() - start;
 column  ++;
 
 if (counter++ < max_count) return;

 // If the accumulator's captured 8192 events, 
 // print the average: 

 accum  >>= fx_shift; 
 
 Serial.print(accum);

 // ^ Without forcing DMA, this prints a steady stream of 218s and 219s.
 //   With forced DMA, this prints a steady stream of 121s.
 
 Serial.println();
 
 accum    = 0;
 counter  = 0;
}
