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

#define SAMPLE_RATE			44100

#define FRAME_RATE			60



struct {
	SDL_Window* window;
	SDL_Surface* surface;
	SDL_Renderer* renderer;
	SDL_Texture* texture;
	SDL_Surface* font_8x8[4];
	SDL_Surface* font_15x16[4];

	int width;
	int height;
	int scale;

} Screen;

#define MAX_TEXT_WDT		32
#define MAX_TEXT_HGT		32

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
	uint8_t text[MAX_TEXT_WDT*MAX_TEXT_HGT];
	uint8_t attr[MAX_TEXT_WDT*MAX_TEXT_HGT];
	uint8_t width;
	uint8_t height; 
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



//this function borrowed from https://gist.github.com/noct/9884320

void sdl_set_window_icon_res(SDL_Window* window)
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



void sdl_video_render(void)
{
	SDL_Rect scaled;

	scaled.x = 0;
	scaled.y = 0;
	scaled.w = Screen.surface->w*Screen.scale;
	scaled.h = Screen.surface->h*Screen.scale;

	SDL_UpdateTexture(Screen.texture, NULL, Screen.surface->pixels, Screen.surface->pitch);

	SDL_RenderCopy(Screen.renderer, Screen.texture, NULL, &scaled);
	SDL_RenderPresent(Screen.renderer);
}



void sdl_get_video_config(int& width, int& height, int& scale, int& text_width, int& text_height);

bool sdl_video_init(void)
{
	int scale = 0;
	int width = 0;
	int height = 0;
	int text_width = 0;
	int text_height = 0;

	sdl_get_video_config(width, height, scale, text_width, text_height);

	if ((scale == 0) || (width == 0) || (height == 0) || (text_width == 0) || (text_height == 0)) return false;

	Screen.scale = scale;
	Screen.width = width;
	Screen.height = height;
	Text.width = text_width;
	Text.height = text_height;

	Screen.surface = SDL_CreateRGBSurface(0, Screen.width, Screen.height, 32, 0, 0, 0, 0);

	if (!Screen.surface) return false;

	Screen.window = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Screen.width*Screen.scale, Screen.height*Screen.scale, SDL_WINDOW_SHOWN);

	if (!Screen.window) return false;

	Screen.renderer = SDL_CreateRenderer(Screen.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (!Screen.renderer) return false;

	Screen.texture = SDL_CreateTextureFromSurface(Screen.renderer, Screen.surface);

	if (!Screen.texture) return false;

	sdl_set_window_icon_res(Screen.window);

	SDL_SetWindowFullscreen(Screen.window, 0);
	SDL_ShowCursor(SDL_ENABLE);

	sdl_video_render();

	return true;
}



void sdl_video_shut(void)
{
	if (Screen.surface)
	{
		SDL_FreeSurface(Screen.surface);
		Screen.surface = NULL;
	}

	if(Screen.renderer)
	{
		SDL_DestroyRenderer(Screen.renderer);
		Screen.renderer = NULL;
	}
	
	if (Screen.window)
	{
		SDL_DestroyWindow(Screen.window);
		Screen.window = NULL;
	}

	if (Screen.texture)
	{
		SDL_DestroyTexture(Screen.texture);
		Screen.texture = NULL;
	}
}



void sdl_video_change_size(void)
{
	sdl_video_shut();
	
	if (!sdl_video_init())
	{
		log_add("Error: %s\n", SDL_GetError());
		exit(-1);
	}
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

	Text.attr[off] = attr;
}

void put_str(signed char x, signed char y, const char* str)
{
	if (y < 0 || y >= Text.height) return;

	while (x < Text.width)
	{
		char c = *str++;

		if (!c) break;

		put_char(x, y, c);

		++x;
	}
}





#include "ESPboy_EBT/ebt_input.h"
#include "ESPboy_EBT/ebt_vars.h"
#include "ESPboy_EBT/ebt_osd.h"
#include "ESPboy_EBT/ebt_main.h"
#include "ESPboy_EBT/ebt_file.h"
#include "ESPboy_EBT/ebt_song.h"
#include "ESPboy_EBT/ebt_load_save.h"
#include "ESPboy_EBT/ebt_export.h"
#include "ESPboy_EBT/ebt_edit_config.h"
#include "ESPboy_EBT/ebt_synth.h"
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



void screen_update(void)
{
	SDL_FillRect(Screen.surface, &Screen.surface->clip_rect, 0);

	int font_id = ebt_config_get_font();

	SDL_Surface* font = NULL;
	
	if (config.video_mode == CFG_VIDEO_MODE_240_320)
	{
		font = Screen.font_15x16[font_id];
	}
	else
	{
		font = Screen.font_8x8[font_id];
	}

	int char_w = font->w / 16;
	int char_h = font->h / 8;

	int sy = 0;

	for (int i = 0; i < Text.height; ++i)
	{
		int ptr = i * Text.width;
		int sx = 0;

		for (int j = 0; j < Text.width; ++j)
		{
			uint8_t c = Text.text[ptr] & 127;

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



void sdl_get_video_config(int& width, int& height, int& scale, int& text_width, int& text_height)
{
	switch (config.video_mode)
	{
	case CFG_VIDEO_MODE_128_128:
		scale = 5;
		width = 128;
		height = 128;
		text_width = 128 / 8;
		text_height = 128 / 8;
		break;

	case CFG_VIDEO_MODE_240_320:
		scale = 3;
		width = 240;
		height = 320;
		text_width = 240 / 15;
		text_height = 320 / 16;
		break;
	}
}


void sdl_load_palette_from_bmp(const char* filename, unsigned int* palette)
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



void sdl_save_screenshot(const char* filename)
{
	SDL_SaveBMP(Screen.surface, filename);
}



void sdl_audio_callback(void* userdata, uint8_t* stream, int bytes)
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



uint8_t sdl_to_ebt_keycode(SDL_Keycode sym)
{
	switch (sym)
	{
	case SDLK_F1: return KB_F1;
	case SDLK_F2: return KB_F2;
	case SDLK_F3: return KB_F3;
	case SDLK_F4: return KB_F4;
	case SDLK_F5: return KB_F5;
	case SDLK_F6: return KB_F6;
	case SDLK_F7: return KB_F7;
	case SDLK_F8: return KB_F8;
	case SDLK_F9: return KB_F9;
	case SDLK_F10: return KB_F10;
	case SDLK_1: return KB_1;
	case SDLK_2: return KB_2;
	case SDLK_3: return KB_3;
	case SDLK_4: return KB_4;
	case SDLK_5: return KB_5;
	case SDLK_6: return KB_6;
	case SDLK_7: return KB_7;
	case SDLK_8: return KB_8;
	case SDLK_9: return KB_9;
	case SDLK_0: return KB_0;

	case SDLK_a: return KB_A;
	case SDLK_b: return KB_B;
	case SDLK_c: return KB_C;
	case SDLK_d: return KB_D;
	case SDLK_e: return KB_E;
	case SDLK_f: return KB_F;
	case SDLK_g: return KB_G;
	case SDLK_h: return KB_H;
	case SDLK_i: return KB_I;
	case SDLK_j: return KB_J;
	case SDLK_k: return KB_K;
	case SDLK_l: return KB_L;
	case SDLK_m: return KB_M;
	case SDLK_n: return KB_N;
	case SDLK_o: return KB_O;
	case SDLK_p: return KB_P;
	case SDLK_q: return KB_Q;
	case SDLK_r: return KB_R;
	case SDLK_s: return KB_S;
	case SDLK_t: return KB_T;
	case SDLK_u: return KB_U;
	case SDLK_v: return KB_V;
	case SDLK_w: return KB_W;
	case SDLK_x: return KB_X;
	case SDLK_y: return KB_Y;
	case SDLK_z: return KB_Z;
	case SDLK_MINUS: return KB_MINUS;
	case SDLK_BACKSPACE: return KB_BACKSPACE;
	case SDLK_INSERT: return KB_INSERT;
	case SDLK_DELETE: return KB_DELETE;
	case SDLK_HOME: return KB_HOME;
	case SDLK_END: return KB_END;
	case SDLK_PAGEUP: return KB_PGUP;
	case SDLK_PAGEDOWN: return KB_PGDOWN;
	case SDLK_TAB: return KB_TAB;
	}

	return KB_NONE;
}



void export_command_line(const char* src_filename)
{
	if (ebt_song_load(src_filename))
	{
		ebt_song_squeeze();

		char out_filename[MAX_PATH];

		strncpy(out_filename, src_filename, sizeof(out_filename));

		for (int i = strlen(out_filename) - 1; i >= 0; --i)
		{
			if (out_filename[i] == '.')
			{
				out_filename[i] = 0;
				strncat(out_filename, ".h", sizeof(out_filename));
				ebt_song_export(out_filename);
				break;
			}
		}
	}
}



void sdl_cleanup(void)
{
	SDL_PauseAudio(1);

	sdl_video_shut();

	for (int i = 0; i < (sizeof(Screen.font_8x8) / sizeof(SDL_Surface*)); ++i)
	{
		SDL_FreeSurface(Screen.font_8x8[i]);
		Screen.font_8x8[i] = NULL;
	}

	for (int i = 0; i < (sizeof(Screen.font_15x16) / sizeof(SDL_Surface*)); ++i)
	{
		SDL_FreeSurface(Screen.font_15x16[i]);
		Screen.font_15x16[i] = NULL;
	}

	SDL_Quit();
	ebt_shut();
	log_close();
}



int main(int argc, char* argv[])
{
	strncpy(dataDirectory, SDL_GetBasePath(), sizeof(dataDirectory) - 1);
	path_trim_to_directory(dataDirectory);
	strncat(dataDirectory, "data\\", sizeof(dataDirectory) - 1);

	log_open("log.txt");

	atexit(sdl_cleanup);

	memset(&Screen, 0, sizeof(Screen));
	memset(&Input, 0, sizeof(Input));

	ebt_init();

	for (int argn = 1; argn < (argc - 1); ++argn)
	{
		if (_stricmp(argv[argn], "-e") == 0)
		{
			export_command_line(argv[argn + 1]);
			exit(0);
		}
	}

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
	{
		log_add("Error: Can't initialize SDL\n");
		exit(-1);
	}

	if (!sdl_video_init())
	{
		log_add("Error: %s\n",SDL_GetError());
		exit(-1);
	}

	if (!sdl_audio_init(SAMPLE_RATE, 2, 2048))
	{
		log_add("Error: %s\n", SDL_GetError());
		exit(-1);
	}

	Screen.font_8x8[0] = SDL_LoadBMP("fonts/custom_8x8.bmp");
	if (!Screen.font_8x8[0]) exit(1);
	Screen.font_8x8[1] = SDL_LoadBMP("fonts/c64_8x8.bmp");
	if (!Screen.font_8x8[1]) exit(1);
	Screen.font_8x8[2] = SDL_LoadBMP("fonts/msx_8x8.bmp");
	if (!Screen.font_8x8[2]) exit(1);
	Screen.font_8x8[3] = SDL_LoadBMP("fonts/zx_8x8.bmp");
	if (!Screen.font_8x8[3]) exit(1);

	Screen.font_15x16[0] = SDL_LoadBMP("fonts/custom_15x16.bmp");
	if (!Screen.font_15x16[0]) exit(1);
	Screen.font_15x16[1] = SDL_LoadBMP("fonts/c64_15x16.bmp");
	if (!Screen.font_15x16[1]) exit(1);
	Screen.font_15x16[2] = SDL_LoadBMP("fonts/msx_15x16.bmp");
	if (!Screen.font_15x16[2]) exit(1);
	Screen.font_15x16[3] = SDL_LoadBMP("fonts/zx_15x16.bmp");
	if (!Screen.font_15x16[3]) exit(1);

	sdl_load_palette_from_bmp("fonts/palette_1.bmp", Text.palette);

	bool quit = false;

	while(!quit)
	{
		const uint8_t* Keyboard=SDL_GetKeyboardState(NULL);

		int state = 0;

		state |= Keyboard[SDL_SCANCODE_LEFT] ? PAD_LEFT : 0;
		state |= Keyboard[SDL_SCANCODE_RIGHT] ? PAD_RIGHT : 0;
		state |= Keyboard[SDL_SCANCODE_UP] ? PAD_UP : 0;
		state |= Keyboard[SDL_SCANCODE_DOWN] ? PAD_DOWN : 0;
		state |= Keyboard[SDL_SCANCODE_RETURN] ? PAD_ACT : 0;
		state |= Keyboard[SDL_SCANCODE_LSHIFT] ? PAD_ACT : 0;
		state |= Keyboard[SDL_SCANCODE_RSHIFT] ? PAD_ACT : 0;
		state |= Keyboard[SDL_SCANCODE_SPACE] ? PAD_ESC : 0;
		state |= Keyboard[SDL_SCANCODE_LCTRL] ? PAD_ESC : 0;
		state |= Keyboard[SDL_SCANCODE_RCTRL] ? PAD_ESC : 0;
		state |= Keyboard[SDL_SCANCODE_ESCAPE] ? PAD_ESC : 0;
		state |= Keyboard[SDL_SCANCODE_LALT] ? PAD_LFT : 0;
		state |= Keyboard[SDL_SCANCODE_RALT] ? PAD_LFT : 0;
		state |= Keyboard[SDL_SCANCODE_F5] ? PAD_RGT : 0;
		state |= Keyboard[SDL_SCANCODE_GRAVE] ? PAD_RGT : 0;

		ebt_input_update_pad(state);
		ebt_input_update_kb(KB_NONE);

		SDL_Event event;

		while(SDL_PollEvent(&event)!=0)
		{
			switch(event.type)
			{
			case SDL_KEYDOWN:
				{
					ebt_input_update_kb(sdl_to_ebt_keycode(event.key.keysym.sym));

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

							sdl_save_screenshot(filename);
						}
						break;

					case SDLK_F10:
						{
							exit(0);
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

		sdl_video_render();

		SDL_Delay(1000 / FRAME_RATE);
	}

	exit(0);
}