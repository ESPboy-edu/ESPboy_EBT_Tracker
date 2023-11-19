
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <conio.h>
#include <memory.h>

#include "../libsdl/include/SDL.h"


#define SAMPLE_RATE			44100


#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif



//functions to imitate Flash program memory access that is necessary on MCUs
//systems that store programs in the RAM does not not need anything special, so just reading the pointers

#ifndef PROGMEM
#define PROGMEM
#endif

uint32_t pgm_read_dword(const uint32_t* data)
{
	return *data;
}


uint8_t pgm_read_byte(const uint8_t* data)
{
	return *data;
}



#include "ESPboy_EBT_player/ebt_synth.h"			//sound synthesizer code
#include "ESPboy_EBT_player/ebt_player.h"			//music data parser code

#include "ESPboy_EBT_player/FRZNG_POINT.h"		//music data itself



void sdl_cleanup(void)
{
	SDL_Quit();
}



void sdl_audio_callback(void* userdata, uint8_t* stream, int bytes)
{
	short *ptr = (short*)stream;

	//printf(".");
	memset(ptr, 0, bytes);

	for (unsigned int i = 0; i < bytes / 2 / sizeof(short); ++i)
	{
		stereo_sample_struct s = ebt_player_render_sample();

		int l = s.l * 1024;
		int r = s.r * 1024;

		if (l < -32767) l = -32767;
		if (l > 32767) l = 32767;
		if (r < -32767) r = -32767;
		if (r > 32767) r = 32767;

		*ptr++ = l;
		*ptr++ = r;
	}
}



bool sdl_audio_init(int rate, int channels, int buffer)
{
	SDL_AudioSpec sndReq, sndObt;

	sndReq.freq = rate;
	sndReq.format = AUDIO_S16SYS;
	sndReq.samples = buffer;
	sndReq.channels = channels;
	sndReq.callback = sdl_audio_callback;
	sndReq.userdata = NULL;

	if (SDL_OpenAudio(&sndReq, &sndObt) < 0) return false;

	if ((sndReq.freq != sndObt.freq) || (sndReq.format != sndObt.format) || (sndReq.channels != sndObt.channels)) return false;

	SDL_PauseAudio(0);

	return true;
}




int main(int argc, char* argv[])
{
	atexit(sdl_cleanup);

	if (SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		printf("Error: Can't initialize SDL\n");
		exit(-1);
	}

	if (!sdl_audio_init(SAMPLE_RATE, 2, 2048))
	{
		printf("Error: %s\n", SDL_GetError());
		exit(-1);
	}

	printf("Music is playing, press any key...\n");

	ebt_player_start(song_FRZNG_POINT, SAMPLE_RATE);

	_getch();

	ebt_player_stop();

	exit(0);
}