#ifdef WIN32
#define _CRT_SECURE_NO_WARNINGS
#include "windows.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#ifdef WIN32
#include <windows.h>
#include <direct.h>
#include "dirent.h"
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#endif

#include "libsdl/include/SDL.h"


#define TITLE	"EBT /ESPboy Tracker/ SDL"

#define VIEWPORT_SCALE		6

#define SAMPLE_RATE			44100

#define FRAME_RATE			60



struct {
	SDL_Window* window;
	SDL_Surface* surface;
	SDL_Renderer* renderer;
	SDL_Texture* texture;
	SDL_Surface* font[4];

	int width;
	int height;
	int scale;

	bool windowed;

} Screen;

char dataDirectory[MAX_PATH];

FILE *logFile=NULL;

void log_open(const char *name)
{
	logFile=fopen(name,"wt");
}



void log_add(const char *format,...)
{
	va_list arg;

	if(!logFile) return;

	va_start(arg,format);
	vfprintf(logFile,format,arg);
	va_end(arg);
	fflush(logFile);
}



void log_close(void)
{
	if(logFile) fclose(logFile);
}



void fail(const char *format,...)
{
	va_list arg;

	va_start(arg,format);
	log_add(format,arg);
	va_end(arg);

	exit(-1);
}



bool video_init(int width,int height,int scale,bool windowed)
{
	Screen.windowed=windowed;
	Screen.scale=scale;
	Screen.width   =width;
	Screen.height  =height;

	Screen.surface = SDL_CreateRGBSurface(0, Screen.width, Screen.height, 32, 0, 0, 0, 0);

	return Screen.surface?true:false;
}



void video_render(void)
{
	SDL_Rect scaled;

	scaled.x=0;
	scaled.y=0;
	scaled.w=Screen.surface->w*Screen.scale;
	scaled.h=Screen.surface->h*Screen.scale;

	SDL_UpdateTexture(Screen.texture, NULL, Screen.surface->pixels,Screen.surface->pitch);

	SDL_RenderCopy(Screen.renderer,Screen.texture,NULL,&scaled);
	SDL_RenderPresent(Screen.renderer);
}



void video_switch_mode(void)
{
	if(Screen.windowed)
	{
		SDL_SetWindowFullscreen(Screen.window,0);
		SDL_ShowCursor(SDL_ENABLE);
	}
	else
	{
		SDL_SetWindowFullscreen(Screen.window,SDL_WINDOW_FULLSCREEN);
		SDL_ShowCursor(SDL_DISABLE);
	}

	video_render();
}



void cleanup(void)
{
	SDL_FreeSurface(Screen.surface);
	SDL_DestroyRenderer(Screen.renderer);
	SDL_DestroyWindow(Screen.window);
	SDL_DestroyTexture(Screen.texture);
	SDL_FreeSurface(Screen.font[0]);
	SDL_FreeSurface(Screen.font[1]);
	SDL_FreeSurface(Screen.font[2]);
	SDL_FreeSurface(Screen.font[3]);

	SDL_Quit();

	log_close();
}



void path_trim_to_directory(char* path)
{
	int i = (int)strlen(path);

	while (--i >= 0)
	{
		if ((path[i] == '\\') || (path[i] == '/'))
		{
			path[i + 1] = 0;
			break;
		}
	}
}



#define TEXT_SCREEN_WDT		16
#define TEXT_SCREEN_HGT		16

#define CHAR_WDT			8
#define CHAR_HGT			8

#define VIEWPORT_WDT		(TEXT_SCREEN_WDT*CHAR_WDT)
#define VIEWPORT_HGT		(TEXT_SCREEN_HGT*CHAR_HGT)

#define RGBC(r,g,b) (((g)<<16)|((b)<<8)|(r))

#define TARGET_SDL

#define PAD_LEFT	0x01
#define PAD_RIGHT	0x02
#define PAD_UP		0x04
#define PAD_DOWN	0x08
#define PAD_ACT		0x10
#define PAD_ESC		0x20
#define PAD_LFT		0x40
#define PAD_RGT		0x80


struct {
	uint8_t text[TEXT_SCREEN_WDT*TEXT_SCREEN_HGT];
	uint8_t attr[TEXT_SCREEN_WDT*TEXT_SCREEN_HGT];
	uint8_t color;

	unsigned int palette[16] = {
	RGBC(0x00, 0x00, 0x00),
	RGBC(0xA8, 0x00, 0x00),
	RGBC(0x00, 0xA8, 0x00),
	RGBC(0xA8, 0xA8, 0x00),
	RGBC(0x00, 0x00, 0xA8),
	RGBC(0xA8, 0x00, 0xA8),
	RGBC(0x00, 0x54, 0xA8),
	RGBC(0xA8, 0xA8, 0xA8),
	RGBC(0x54, 0x54, 0x54),
	RGBC(0xFC, 0x54, 0x54),
	RGBC(0x54, 0xFC, 0x54),
	RGBC(0xFC, 0xFC, 0x54),
	RGBC(0x54, 0x54, 0xFC),
	RGBC(0xFC, 0x54, 0xFC),
	RGBC(0x54, 0xFC, 0xFC),
	RGBC(0xff, 0xff, 0xff)
	};
} Text;



void screen_clear(void)
{
	memset(Text.text, 0x20, sizeof(Text.text));
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
	int off = y * TEXT_SCREEN_WDT + x;

	Text.text[off] = c;
	Text.attr[off] = Text.color;
}

uint8_t get_attr(signed char x, signed char y)
{
	int off = y * TEXT_SCREEN_WDT + x;

	return Text.attr[off];
}

void put_attr(signed char x, signed char y, uint8_t attr)
{
	int off = y * TEXT_SCREEN_WDT + x;

	Text.attr[off] = attr;
}

void put_str(signed char x, signed char y, const char* str)
{
	if (y < 0 || y >= TEXT_SCREEN_HGT) return;

	while (x < TEXT_SCREEN_WDT)
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
	SDL_FillRect(Screen.surface, &Screen.surface->clip_rect, 0);

	SDL_Surface* font = Screen.font[ebt_config_get_font()];

	int char_w = font->w / 16;
	int char_h = font->h / 8;

	int sy = 0;

	for (int i = 0; i < TEXT_SCREEN_HGT; ++i)
	{
		int ptr = i * TEXT_SCREEN_WDT;
		int sx = 0;

		for (int j = 0; j < TEXT_SCREEN_WDT; ++j)
		{
			uint8_t c = Text.text[ptr]&127;

			int tx = c % 16 * char_w;
			int ty = c / 16 * char_h;

			int ink = (Text.attr[ptr]) & 15;
			int paper = (Text.attr[ptr] >> 4) & 15;

			for (int k = 0; k < char_h; ++k)
			{
				Uint8* src = ((Uint8*)font->pixels + (ty*font->pitch) + (tx*font->format->BytesPerPixel));
				Uint32* dst = (Uint32*)((Uint8*)Screen.surface->pixels + ((sy + k)*Screen.surface->pitch) + (sx*Screen.surface->format->BytesPerPixel));

				for (int l = 0; l < char_w; ++l)
				{
					Uint8 col = *src++;

					*dst++ = Text.palette[(col ? ink : paper)];
				}

				++ty;
			}

			++ptr;

			sx += char_w;
		}

		sy += char_h;
	}
}



#include "ESPboy_EBT/ebt_input.h"
#include "ESPboy_EBT/ebt_main.h"
#include "ESPboy_EBT/ebt_file.h"
#include "ESPboy_EBT/ebt_load_save.h"
#include "ESPboy_EBT/ebt_edit_config.h"
#include "ESPboy_EBT/ebt_player.h"
#include "ESPboy_EBT/ebt_vumeter.h"
#include "ESPboy_EBT/ebt_dlg_confirm.h"
#include "ESPboy_EBT/ebt_dlg_file_browser.h"
#include "ESPboy_EBT/ebt_dlg_input_name.h"
#include "ESPboy_EBT/ebt_edit_info.h"
#include "ESPboy_EBT/ebt_edit_song.h"
#include "ESPboy_EBT/ebt_edit_order.h"
#include "ESPboy_EBT/ebt_edit_pattern.h"
#include "ESPboy_EBT/ebt_edit_instrument.h"



void load_palette_from_bmp(const char* filename, unsigned int* palette)
{
	SDL_Surface* surface = SDL_LoadBMP(filename);

	if (surface)
	{
		for (int i = 0; i < 16; ++i)
		{
			unsigned int color = (surface->format->palette->colors[i].r << 16) | (surface->format->palette->colors[i].g << 8) | (surface->format->palette->colors[i].b);

			palette[i] = color;
		}

		SDL_FreeSurface(surface);
	}
	else
	{
		printf("Warning: couldn't load palette from %s\n", filename);
	}
}



void save_screenshot(const char* filename)
{
	SDL_SaveBMP(Screen.surface, filename);
}




//this function borrowed from https://gist.github.com/noct/9884320

void set_window_icon_res(SDL_Window* window)
{
#ifdef WIN32
	const unsigned int mask_r = 0x00ff0000;
	const unsigned int mask_g = 0x0000ff00;
	const unsigned int mask_b = 0x000000ff;
	const unsigned int mask_a = 0xff000000;
	const int res_id = 1;
	const int size = 32;
	const int bpp = 32;

	HICON icon = (HICON)LoadImage(
		GetModuleHandle(NULL),
		MAKEINTRESOURCE(res_id),
		IMAGE_ICON,
		size, size,
		LR_SHARED
	);

	if (icon) {
		ICONINFO ici;

		if (GetIconInfo(icon, &ici)) {
			HDC dc = CreateCompatibleDC(NULL);

			if (dc) {
				SDL_Surface* surface = SDL_CreateRGBSurface(0, size, size, bpp, mask_r, mask_g, mask_b, mask_a);

				if (surface) {
					BITMAPINFO bmi;
					bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
					bmi.bmiHeader.biWidth = size;
					bmi.bmiHeader.biHeight = -size;
					bmi.bmiHeader.biPlanes = 1;
					bmi.bmiHeader.biBitCount = bpp;
					bmi.bmiHeader.biCompression = BI_RGB;
					bmi.bmiHeader.biSizeImage = 0;

					SelectObject(dc, ici.hbmColor);
					GetDIBits(dc, ici.hbmColor, 0, size, surface->pixels, &bmi, DIB_RGB_COLORS);
					SDL_SetWindowIcon(window, surface);
					SDL_FreeSurface(surface);
				}
				DeleteDC(dc);
			}
			DeleteObject(ici.hbmColor);
			DeleteObject(ici.hbmMask);
		}
		DestroyIcon(icon);
	}
#endif
}



void audio_callback(void* userdata, uint8_t* stream, int bytes)
{
	short *ptr = (short*)stream;

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


void audio_init(int rate, int channels, int buffer)
{
	SDL_AudioSpec *sndReq, *sndObt;

	sndReq = (SDL_AudioSpec*)malloc(sizeof(SDL_AudioSpec));
	sndObt = (SDL_AudioSpec*)malloc(sizeof(SDL_AudioSpec));

	sndReq->freq = rate;
	sndReq->format = AUDIO_S16SYS;
	sndReq->samples = buffer;
	sndReq->channels = channels;
	sndReq->callback = audio_callback;
	sndReq->userdata = NULL;

	if (SDL_OpenAudio(sndReq, sndObt) < 0) fail("Unable to set audio: %s\n", SDL_GetError());

	if (sndReq->freq != sndObt->freq || sndReq->format != sndObt->format || sndReq->channels != sndObt->channels)
	{
		fail("Unable to get requested audio buffer (%ihz 16bit %ich)\n", rate, channels);
	}

	free(sndReq);

	sndReq = sndObt;

	SDL_PauseAudio(0);
}


int main(int argc, char* argv[])
{
	SDL_Event event;
	bool quit;

	log_open("log.txt");

	atexit(cleanup);

	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)<0)
	{
		log_add("Error: Can't initialize SDL\n");
		exit(-1);
	}

	if (!video_init(VIEWPORT_WDT, VIEWPORT_HGT, VIEWPORT_SCALE, true))
	{
		log_add("Error: %s\n",SDL_GetError());
		exit(-1);
	}

	Screen.window = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Screen.width*Screen.scale, Screen.height*Screen.scale, SDL_WINDOW_SHOWN);
	Screen.renderer=SDL_CreateRenderer(Screen.window,-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
	Screen.texture=SDL_CreateTextureFromSurface(Screen.renderer, Screen.surface);

	if(!Screen.window)
	{
		log_add("Error: %s\n",SDL_GetError());
		exit(-1);
	}

	set_window_icon_res(Screen.window);

	memset(&Input,0,sizeof(Input));

	strncpy(dataDirectory, SDL_GetBasePath(), sizeof(dataDirectory)-1);
	path_trim_to_directory(dataDirectory);
	strncat(dataDirectory, "data\\", sizeof(dataDirectory)-1);

	ebt_init();

	audio_init(SAMPLE_RATE, 2, 4096);

	video_switch_mode();

	quit=false;

	Screen.font[0] = SDL_LoadBMP("font_custom.bmp");
	if (!Screen.font[0]) exit(1); 
	Screen.font[1] = SDL_LoadBMP("font_c64.bmp");
	if (!Screen.font[1]) exit(1);
	Screen.font[2] = SDL_LoadBMP("font_msx.bmp");
	if (!Screen.font[2]) exit(1);
	Screen.font[3] = SDL_LoadBMP("font_zx.bmp");
	if (!Screen.font[3]) exit(1);
	
	load_palette_from_bmp("palette_1.bmp", Text.palette);

	while(!quit)
	{
		const uint8_t* Keyboard=SDL_GetKeyboardState(NULL);

		int state = 0;

		state |= Keyboard[SDL_SCANCODE_LEFT] ? PAD_LEFT : 0;
		state |= Keyboard[SDL_SCANCODE_RIGHT] ? PAD_RIGHT : 0;
		state |= Keyboard[SDL_SCANCODE_UP] ? PAD_UP : 0;
		state |= Keyboard[SDL_SCANCODE_DOWN] ? PAD_DOWN : 0;
		state |= Keyboard[SDL_SCANCODE_Z] ? PAD_ACT : 0;
		state |= Keyboard[SDL_SCANCODE_X] ? PAD_ESC : 0;
		state |= Keyboard[SDL_SCANCODE_A] ? PAD_LFT : 0;
		state |= Keyboard[SDL_SCANCODE_S] ? PAD_RGT : 0;

		ebt_input_update(state);

		while(SDL_PollEvent(&event)!=0)
		{
			switch(event.type)
			{
			case SDL_KEYDOWN:
				{
					switch(event.key.keysym.sym)
					{
					case SDLK_F12:
						{
							char filename[MAX_PATH];
							char filepath[MAX_PATH];

							strncpy(filepath, SDL_GetBasePath(), sizeof(filepath));
							strncat(filepath, "screenshot", sizeof(filepath));

#ifdef WIN32
							_mkdir(filepath);
#else
							mkdir(filepath, S_IRWXU);
#endif

							time_t raw_time;
							struct tm *time_info;

							time(&raw_time);
							time_info = localtime(&raw_time);

							snprintf(filename, sizeof(filename), "%s/%2.2i%2.2i%2.2i_%2.2i%2.2i%2.2i.bmp", filepath, time_info->tm_mday, time_info->tm_mon, time_info->tm_year % 100, time_info->tm_hour, time_info->tm_min, time_info->tm_sec);

							save_screenshot(filename);
						}
						break;

					case SDLK_F10:
						{
							exit(0);
						}
						break;

					case SDLK_RETURN:
						{
							if(event.key.keysym.mod&(KMOD_RALT|KMOD_LALT))
							{
								Screen.windowed^=true;

								video_switch_mode();
							}
						}
						break;
					}
				}

				break;

			case SDL_KEYUP:
				{
				}
				break;

			case SDL_QUIT:
				exit(0);
				break;
			}
		}

		ebt_update();
		screen_update();

		video_render();

		SDL_Delay(1000 / FRAME_RATE);
	}

	exit(0);
}