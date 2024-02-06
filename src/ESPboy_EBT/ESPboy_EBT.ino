//EBT - ESPboy tracker by shiru8bit (shiru@mail.ru) WTFPL

/*
  How to compile:

  - Install Arduino IDE https://www.arduino.cc/en/Main/Software
  - Install the WeMos D1 mini ESP866 board to your Arduino IDE https://www.instructables.com/Wemos-ESP8266-Getting-Started-Guide-Wemos-101/
  - Install all the libs for ESPboy from here https://github.com/ESPboy-edu/ESPboy_ArduinoIDE_Libraries
  - Select the "LOLIN(WEMOS) D1 R2 & mini" board in the Arduino IDE and compile this code

  IMPORTANT!
  if you are using/updating original TFTeSPI library
  you have to edit or replace usersetup.h file in the TFTeSPI folder in your Arduino libraries collection
  according to this https://github.com/ESPboy-edu/ESPboy_ArduinoIDE_Libraries/blob/master/TFT_eSPI-master/User_Setup.h
*/

//define only one of the three options below

#define USE_TFTESPI     //regular 128x128 screen via TFTeSPI and nbSPI, fastest rendering
//#define USE_LOVYANGFX   //regular 128x128 screen via LovyanGFX, no nbSPI
//#define USE_240320    //large 240x320 screen via LovyanGFX, for the custom-built EBTboy


#ifdef USE_TFTESPI
#include "lib/tftespi/ESPboyInit.h"
#include "lib/tftespi/ESPboyInit.cpp"
//#include "lib/tftespi/ESPboyTerminalGUI.h"
//#include "lib/tftespi/ESPboyTerminalGUI.cpp"
//#include "lib/tftespi/ESPboyOTA2.h"
//#include "lib/tftespi/ESPboyOTA2.cpp"

#define USE_NBSPI   //can be reverted to default TFT_eSPI method, slower but (presumably) more compatible
#endif

#ifdef USE_LOVYANGFX
#include "lib/lovyangfx/ESPboyInit.h"
#include "lib/lovyangfx/ESPboyInit.cpp"
#undef USE_NBSPI  //this library does not work with nbSPI by some reason
#endif

#ifdef USE_240320
#include "lib/lovyangfx_tft20/ESPboyInit.h"
#include "lib/lovyangfx_tft20/ESPboyInit.cpp"
#undef USE_NBSPI
#endif

#ifdef USE_NBSPI
#include "nbSPI.h"
#endif

#include <LittleFS.h>
#include <sigma_delta.h>
#include <I2S.h>



#define TARGET_ESPBOY

ESPboyInit myESPboy;
//ESPboyTerminalGUI *terminalGUIobj = NULL;
//ESPboyOTA2 *OTA2obj = NULL;

int8_t hasDAC;

#define INT_SOUND_PIN       D3  //for mono SDM
#define EXT_SOUND_PIN       D6

#define EXT_SOUND_L_PIN     D6  //for stereo PWM
#define EXT_SOUND_R_PIN     D8

#ifndef USE_240320

#include "fonts/custom_8x8.h" //128 1-bit 8x8 characters
#include "fonts/c64_8x8.h"
#include "fonts/msx_8x8.h"
#include "fonts/zx_8x8.h"

const uint8_t* font_data[4] = {
  custom_8x8,
  c64_8x8,
  msx_8x8,
  zx_8x8
};

#else

#include "fonts/custom_15x16.h" //128 1-bit 15x16 characters (stored as 16x16)
#include "fonts/c64_15x16.h"
#include "fonts/msx_15x16.h"
#include "fonts/zx_15x16.h"

const uint8_t* font_data[4] = {
  custom_15x16,
  c64_15x16,
  msx_15x16,
  zx_15x16
};

#endif


//RRRRR GGGGGG BBBBB

#define RGBC(b,g,r)       ( (((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3) )

#include "fonts/palette_1.h"  //16 16-bit RGB colors


volatile uint8_t frame_cnt = 0;
volatile uint32_t sample_cnt = 0;

#define SAMPLE_RATE   44100 //should not exceed 44100 for i2s
#define FRAME_RATE    60

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef BOOL
#define BOOL uint8_t
#endif

#define MAX_PATH          32

#ifndef USE_240320
#define CHAR_WDT          8
#define CHAR_HGT          8
#define MAX_TEXT_WDT      (128/CHAR_WDT)
#define MAX_TEXT_HGT      (128/CHAR_HGT)
#else
#define CHAR_WDT          15
#define CHAR_HGT          16
#define MAX_TEXT_WDT      (240/CHAR_WDT)
#define MAX_TEXT_HGT      (320/CHAR_HGT)
#endif

struct {
  uint8_t text[MAX_TEXT_WDT * MAX_TEXT_HGT];
  uint8_t attr[MAX_TEXT_WDT * MAX_TEXT_HGT];
  uint8_t text_prev[MAX_TEXT_WDT * MAX_TEXT_HGT];
  uint8_t attr_prev[MAX_TEXT_WDT * MAX_TEXT_HGT];
  uint8_t color;
  uint8_t width;
  uint8_t height;
  uint16_t palette[16]; //to store a gamma-corrected palette
} Text;



void wait_timer_frame(void)
{
  uint8_t prev = frame_cnt;

  while (frame_cnt == prev)
  {
    delay(0);
    yield();
  }
}

void screen_clear(void)
{
  memset(Text.text, 0x20 | 0x80, sizeof(Text.text));
  memset(Text.attr, 0x0f, sizeof(Text.attr));
}

void set_font_color(uint8_t ink)
{
  Text.color = (Text.color & 0xf0) | (ink & 0x0f);
}

void set_back_color(uint8_t paper)
{
  Text.color = (Text.color & 0x0f) | ((paper << 4) & 0xf0);
}

void put_char(signed char x, signed char y, char c)
{
  int off = y * Text.width + x;

  Text.text[off] = c;
  Text.attr[off] = Text.color;
}

uint8_t get_attr(signed char x, signed char y)
{
  int off = y * Text.width + x;

  return Text.attr[off];
}

void put_attr(signed char x, signed char y, uint8_t attr)
{
  int off = y * Text.width + x;

  Text.text[off] |= 0x80;
  Text.attr[off] = attr;
}

void put_str(signed char x, signed char y, const char* str)
{
  if (y < 0 || y >= Text.height) return;

  while (x < 16)
  {
    char c = *str++;

    if (!c) break;

    put_char(x, y, c);

    ++x;
  }
}

uint8_t ebt_config_get_font(void);



void screen_update(void)
{
  static uint16_t _buf1[CHAR_WDT * CHAR_HGT] __attribute__((aligned(32)));
  static uint16_t _buf2[CHAR_WDT * CHAR_HGT] __attribute__((aligned(32)));

  uint16_t* render_buf = _buf1;
  uint16_t* transfer_buf = _buf2;

  //int tiles_rendered = 0;

#if defined(USE_LOVYANGFX) || defined(USE_240320)
  myESPboy.tft.startWrite();
#endif

  for (int cy = 0; cy < Text.height; ++cy)
  {
    for (int cx = 0; cx < Text.width; ++cx)
    {
      int ptr = cy * Text.width + cx;

      if ((Text.text[ptr] != Text.text_prev[ptr]) || (Text.attr[ptr] != Text.attr_prev[ptr])) //only redraw the characters that actually has been changed
      {
        Text.text_prev[ptr] = Text.text[ptr];
        Text.attr_prev[ptr] = Text.attr[ptr];

        //rendering a character while a previous one gets transferred via the nbSPI DMA

        uint8_t sym = Text.text[ptr] & 127;
        uint8_t col = Text.attr[ptr];

        uint16_t ink = Text.palette[col & 0x0f];
        uint16_t pap = Text.palette[col >> 4];

#ifdef USE_NBSPI
        ink = (ink << 8) | (ink >> 8);
        pap = (pap << 8) | (pap >> 8);
#endif

#ifndef USE_240320
        int sym_size = 1 * 8;
#else
        int sym_size = 2 * 16;
#endif

		int font_id = ebt_config_get_font();
        const uint8_t* src = font_data[font_id] + (sym * sym_size);
        uint16_t* dst = render_buf;

        for (int py = 0; py < CHAR_HGT; ++py)
        {
          //unrolled CHAR_WDT loop

#ifndef USE_240320

          uint8_t row = pgm_read_byte(src + py);

          *dst++ = row & 0x01 ? ink : pap;
          *dst++ = row & 0x02 ? ink : pap;
          *dst++ = row & 0x04 ? ink : pap;
          *dst++ = row & 0x08 ? ink : pap;
          *dst++ = row & 0x10 ? ink : pap;
          *dst++ = row & 0x20 ? ink : pap;
          *dst++ = row & 0x40 ? ink : pap;
          *dst++ = row & 0x80 ? ink : pap;
#else

          uint8_t row1 = pgm_read_byte(src + py * 2 + 0);
          uint8_t row2 = pgm_read_byte(src + py * 2 + 1);

          *dst++ = row1 & 0x01 ? ink : pap;
          *dst++ = row1 & 0x02 ? ink : pap;
          *dst++ = row1 & 0x04 ? ink : pap;
          *dst++ = row1 & 0x08 ? ink : pap;
          *dst++ = row1 & 0x10 ? ink : pap;
          *dst++ = row1 & 0x20 ? ink : pap;
          *dst++ = row1 & 0x40 ? ink : pap;
          *dst++ = row1 & 0x80 ? ink : pap;
          *dst++ = row2 & 0x01 ? ink : pap;
          *dst++ = row2 & 0x02 ? ink : pap;
          *dst++ = row2 & 0x04 ? ink : pap;
          *dst++ = row2 & 0x08 ? ink : pap;
          *dst++ = row2 & 0x10 ? ink : pap;
          *dst++ = row2 & 0x20 ? ink : pap;
          *dst++ = row2 & 0x40 ? ink : pap;
#endif
        }

#ifdef USE_NBSPI
        while (nbSPI_isBusy()); //wait a previous DMA transfer to finish
#endif
        myESPboy.tft.setAddrWindow(cx * CHAR_WDT, cy * CHAR_HGT, CHAR_WDT, CHAR_HGT);

        uint16_t* temp = transfer_buf; //exchange pointers
        transfer_buf = render_buf;
        render_buf = temp;

#ifdef USE_NBSPI
        nbSPI_writeBytes((uint8_t*)transfer_buf, CHAR_WDT * CHAR_HGT * sizeof(uint16_t)); //start a new transfer
#else

#ifdef USE_TFTESPI
        myESPboy.tft.pushColors(transfer_buf, CHAR_WDT * CHAR_HGT);
#endif
#if defined(USE_LOVYANGFX) || defined(USE_240320)
        myESPboy.tft.pushPixels(transfer_buf, CHAR_WDT * CHAR_HGT);
#endif

#endif

        //++tiles_rendered;
      }
    }
  }

#if defined(USE_LOVYANGFX) || defined(USE_240320)
  myESPboy.tft.endWrite();
#endif

  //if (tiles_rendered) Serial.println(tiles_rendered);
}



void sound_output_init(void);
void sound_output_shut(void);

#include "ebt_input.h"
#include "ebt_vars.h"
#include "ebt_osd.h"
#include "ebt_main.h"
#include "ebt_file.h"
#include "ebt_song.h"
#include "ebt_load_save.h"
#include "ebt_export.h"
#include "ebt_edit_config.h"
#include "ebt_synth.h"
#include "ebt_player.h"
#include "ebt_vumeter.h"
#include "ebt_dlg_confirm.h"
#include "ebt_dlg_file_browser.h"
#include "ebt_dlg_input_name.h"
#include "ebt_edit_info.h"
#include "ebt_edit_song.h"
#include "ebt_edit_order.h"
#include "ebt_edit_pattern.h"
#include "ebt_edit_instrument.h"




void IRAM_ATTR timer_sdm_ISR()
{
  static uint8_t sample_dac = 0;

  sigmaDeltaWrite(0, sample_dac);

  stereo_sample_struct s = ebt_player_render_sample();

  sample_dac = ((s.l + s.r) * (128 / 4 / 2)); //256 is the range, 4 is the channels count, 2 is stereo

  ++sample_cnt;

  if (sample_cnt >= (SAMPLE_RATE / FRAME_RATE))
  {
    sample_cnt = 0;
    ++frame_cnt;
  }
}



void IRAM_ATTR timer_pwm_ISR()
{
  static uint8_t sample_dac_l = 0;
  static uint8_t sample_dac_r = 0;

  for (uint8_t i = 0; i < 16; ++i)
  {
    if (i < sample_dac_l)
    {
      GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, _BV(EXT_SOUND_L_PIN));   //set L pin
    }
    else
    {
      GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, _BV(EXT_SOUND_L_PIN));   //reset L pin
    }

    if (i < sample_dac_r)
    {
      GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, _BV(EXT_SOUND_R_PIN));   //set R pin
    }
    else
    {
      GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, _BV(EXT_SOUND_R_PIN));   //reset R pin
    }

    __asm__ __volatile__ ("nop");
    __asm__ __volatile__ ("nop");
    __asm__ __volatile__ ("nop");
    __asm__ __volatile__ ("nop");
  }

  stereo_sample_struct s = ebt_player_render_sample();

  sample_dac_l = s.l;
  sample_dac_r = s.r;

  if (sample_dac_l > 15) sample_dac_l = 15;
  if (sample_dac_r > 15) sample_dac_r = 15;

  ++sample_cnt;

  if (sample_cnt >= (SAMPLE_RATE / FRAME_RATE))
  {
    sample_cnt = 0;
    ++frame_cnt;
  }
}



void IRAM_ATTR timer_i2s_ISR()
{
  static uint32_t stereo_dac = 0;

  i2s_write_sample_nb(stereo_dac);

  stereo_sample_struct s = ebt_player_render_sample();

  int32_t vol = 2048 * config.i2s_volume / 9;

  int32_t l = s.l * vol;
  int32_t r = s.r * vol;

  if (l < -32767) l = -32767;
  if (l > 32767) l = 32767;
  if (r < -32767) r = -32767;
  if (r > 32767) r = 32767;

  stereo_dac = ((r << 16) & 0xffff0000) | (l & 0x0000ffff);

  ++sample_cnt;

  if (sample_cnt >= (SAMPLE_RATE / FRAME_RATE))
  {
    sample_cnt = 0;
    ++frame_cnt;
  }
}



void sound_output_init(void)
{
  myESPboy.myLED.setRGB(0, 0, 0);
  myESPboy.myLED.off();

  noInterrupts();

  if ((config.output == CFG_OUTPUT_INT_SDM) || (config.output == CFG_OUTPUT_EXT_SDM) || (config.output == CFG_OUTPUT_ALL_SDM))
  {
    sigmaDeltaSetup(0, F_CPU / 256);

    if ((config.output == CFG_OUTPUT_INT_SDM) || (config.output == CFG_OUTPUT_ALL_SDM)) sigmaDeltaAttachPin(INT_SOUND_PIN);
    if ((config.output == CFG_OUTPUT_EXT_SDM) || (config.output == CFG_OUTPUT_ALL_SDM)) sigmaDeltaAttachPin(EXT_SOUND_PIN);

    sigmaDeltaEnable();

    timer1_attachInterrupt(timer_sdm_ISR);
  }

  if (config.output == CFG_OUTPUT_EXT_PWM)
  {
    pinMode(EXT_SOUND_L_PIN, OUTPUT);
    pinMode(EXT_SOUND_R_PIN, OUTPUT);

    timer1_attachInterrupt(timer_pwm_ISR);
  }

  if (config.output == CFG_OUTPUT_EXT_I2S)
  {
    i2s_begin();
    i2s_set_rate(SAMPLE_RATE);

    timer1_attachInterrupt(timer_i2s_ISR);
  }

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
  sigmaDeltaDetachPin(EXT_SOUND_PIN);
  i2s_end();
  interrupts();
}



void setup()
{
  Serial.begin(115200);
  delay(300);
  myESPboy.begin("ESPboy tracker v1.2");
  hasDAC = myESPboy.mcp.writeDAC(4096, false);  //if present, DAC controls the display backlight brightness
/*
  if (myESPboy.getKeys()&PAD_ACT || myESPboy.getKeys()&PAD_ESC) { 
     terminalGUIobj = new ESPboyTerminalGUI(&myESPboy.tft, &myESPboy.mcp);
     OTA2obj = new ESPboyOTA2(terminalGUIobj);
  }
*/
  printf("RAM at power up %i bytes\n", ESP.getFreeHeap());
  
  memset(&Text.text, 0, sizeof(Text.text));
  memset(&Text.attr, 0, sizeof(Text.attr));
  Text.color = 0;
  Text.width = MAX_TEXT_WDT;
  Text.height = MAX_TEXT_HGT;

  //convert palette with gamma-correction applied (important for low quality TFT screens)

  const uint8_t* pal_rgb = palette_1;
  const float gamma = 0.8f;
  const float gamma_inv = 1.0f / gamma;

  for (int id = 0; id < 16; ++id)
  {
    int r = *pal_rgb++;
    int g = *pal_rgb++;
    int b = *pal_rgb++;

    r = (int)(255.0f * pow(((float)r) / 255.0f, gamma_inv));
    g = (int)(255.0f * pow(((float)g) / 255.0f, gamma_inv));
    b = (int)(255.0f * pow(((float)b) / 255.0f, gamma_inv));

    Text.palette[id] = RGBC(r, g, b);
  }

  printf("littlefs init\n");

  LittleFS.begin();

  memset(Text.text_prev, 0, sizeof(Text.text_prev));
  memset(Text.attr_prev, 0, sizeof(Text.attr_prev));

  printf("ebt_init\n");

  ebt_init();

  printf("sound_output_init\n");

  sound_output_init();

  printf("RAM before entering work mode %i bytes\n", ESP.getFreeHeap());
}



void loop()
{
  while (ebt_is_active())
  {
    ebt_input_update_pad(myESPboy.getKeys());
    ebt_input_update_kb(KB_NONE); //for physical keyboard
 
    ebt_update();
    screen_update();

    wait_timer_frame();
  }

  ebt_shut();
  sound_output_shut();

  WiFiFileManager();
}
