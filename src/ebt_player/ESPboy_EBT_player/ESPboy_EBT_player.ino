
#include "lib/lovyangfx/ESPboyInit.h"
#include "lib/lovyangfx/ESPboyInit.cpp"

#include <sigma_delta.h>

ESPboyInit myESPboy;
int8_t hasDAC;

#define INT_SOUND_PIN   D3

#define SAMPLE_RATE     44100     //you can lower this value to reduce CPU load in trade for sound quality
#define FRAME_RATE      60

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef BOOL
#define BOOL uint8_t
#endif


#include "ebt_synth.h"    //sound synthesizer code matches the one in the tracker 100%
#include "ebt_player.h"   //music data parser code is much different

#include "FRZNG_POINT.h"



void IRAM_ATTR timer_sdm_ISR()
{
  static uint8_t sample_dac = 0;    //this buffer var is needed to reduce the jitter, as player logic may take variable time

  sigmaDeltaWrite(0, sample_dac);

  stereo_sample_struct s = ebt_player_render_sample();

  sample_dac = ((s.l + s.r) * (128 / 4 / 2)); //256 is the range, 4 is the channels count, 2 is stereo
}



void sound_output_init(void)
{
  noInterrupts();

  sigmaDeltaSetup(0, F_CPU / 256);
  sigmaDeltaAttachPin(INT_SOUND_PIN);
  sigmaDeltaEnable();

  timer1_attachInterrupt(timer_sdm_ISR);
  timer1_enable(TIM_DIV1, TIM_EDGE, TIM_LOOP);
  timer1_write(80000000 / SAMPLE_RATE);  //timer function considers 80 Mhz even in 160 MHz mode

  interrupts();
}



void sound_output_shut(void)
{
  noInterrupts();
  timer1_disable();
  sigmaDeltaDisable();
  sigmaDeltaDetachPin(INT_SOUND_PIN);
  interrupts();
}



void setup()
{
  Serial.begin(115200);

  myESPboy.begin("ESPboy player");
  hasDAC = myESPboy.mcp.writeDAC(4096, false);  //if present, DAC controls the display backlight brightness

  printf("RAM free at power up %i bytes\n", ESP.getFreeHeap());

  ebt_player_start(song_FRZNG_POINT, SAMPLE_RATE);

  sound_output_init();

  printf("RAM free while playing %i bytes\n", ESP.getFreeHeap());
}



void loop()
{
  delay(1000);
  printf("playing...\n");
}
