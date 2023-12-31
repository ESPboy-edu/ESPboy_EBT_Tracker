struct config_struct {
	uint8_t output;
	uint8_t i2s_volume;	//1..9
	uint8_t cursor_blink;
	uint8_t screen_brightness;	//1..9
	uint8_t font;
	uint8_t ord_highlight;
	uint8_t ptn_sound_type;
	uint8_t ptn_sound_len;
	uint8_t swap_lft_rgt;
	uint8_t auto_load;
	uint8_t hints;
	uint8_t backup;
#ifdef TARGET_SDL
	uint8_t video_mode;
	uint8_t full_kb;
#endif
};

config_struct config;
config_struct config_prev;

#ifndef TARGET_SDL

enum {
	CFG_ITEM_OUTPUT = 0,
	CFG_ITEM_I2S_VOLUME,
	CFG_ITEM_CURSOR_BLINK,
	CFG_ITEM_BRIGHTNESS,
	CFG_ITEM_FONT,
	CFG_ITEM_ORD_HIGHLIGHT,
	CFG_ITEM_PTN_SOUND_TYPE,
	CFG_ITEM_PTN_SOUND_LEN,
	CFG_ITEM_SWAP_LFT_RGT,
	CFG_ITEM_AUTO_LOAD,
	CFG_ITEM_HINTS,
	CFG_ITEM_BACKUP,
	CFG_ITEM_FILE_MANAGER,
	CFG_ITEM_WIFI_AP,
	CFG_ITEMS_ALL
};

enum {
	CFG_OUTPUT_INT_SDM = 0,
	CFG_OUTPUT_EXT_SDM,
	CFG_OUTPUT_ALL_SDM,
	CFG_OUTPUT_EXT_PWM,
	CFG_OUTPUT_EXT_I2S,
	CFG_OUTPUTS_ALL
};

#else

enum {
	CFG_ITEM_CURSOR_BLINK = 0,
	CFG_ITEM_FONT,
	CFG_ITEM_VIDEO_MODE,
	CFG_ITEM_ORD_HIGHLIGHT,
	CFG_ITEM_PTN_SOUND_TYPE,
	CFG_ITEM_PTN_SOUND_LEN,
	CFG_ITEM_SWAP_LFT_RGT,
	CFG_ITEM_AUTO_LOAD,
	CFG_ITEM_HINTS,
	CFG_ITEM_BACKUP, 
	CFG_ITEM_FULL_KB,
	CFG_ITEM_FILE_MANAGER,
	CFG_ITEMS_ALL
};

enum {
	CFG_VIDEO_MODE_128_128 = 0,
	CFG_VIDEO_MODE_240_320,
	CFG_VIDEO_MODES_ALL
};

#endif

enum {
	CFG_CURSOR_BLINK_NONE = 0,
	CFG_CURSOR_BLINK_FAST,
	CFG_CURSOR_BLINK_MED,
	CFG_CURSOR_BLINK_SLOW,
	CFG_BLINKS_ALL
};

enum {
	CFG_FONT_CUSTOM = 0,
	CFG_FONT_C64,
	CFG_FONT_MSX,
	CFG_FONT_ZX,
	CFG_FONTS_ALL
};

enum {
	CFG_SOUND_TYPE_MUTE = 0,
	CFG_SOUND_TYPE_PRESS,
	CFG_SOUND_TYPE_CHANGE,
	CFG_SOUND_TYPE_RELEASE,
	CFG_SOUND_TYPES_ALL
};

signed char config_cur = 0;



void ebt_config_update_brightness(void)
{
#ifdef TARGET_ESPBOY
	const int max = 1100;
	const int min = 700;
	const int range = max - min;
	int value = min + range * (int)config.screen_brightness / 9;
	if (config.screen_brightness == 9) value = 4096;
	if (hasDAC) myESPboy.mcp.writeDAC(value, false);
#endif
}



void ebt_config_update_sound_device(void)
{
#ifdef TARGET_ESPBOY
	sound_output_shut();
	sound_output_init();
#endif
}



void ebt_config_set_default(void)
{
	memset(&config, 0, sizeof(config));

	config.cursor_blink = CFG_CURSOR_BLINK_MED;
	config.screen_brightness = 9;
	config.ord_highlight = 1;
	config.ptn_sound_type = CFG_SOUND_TYPE_CHANGE;
	config.i2s_volume = 9;
	config.auto_load = TRUE;
	config.hints = TRUE;
	config.backup = TRUE;
#ifdef TARGET_SDL
	config.full_kb = 1;
#endif

	memcpy(&config_prev, &config, sizeof(config_prev));

	ebt_config_update_brightness();
}



void ebt_config_load(void)
{
	if (!ebt_file_open(FILE_NAME_CFG, false)) return;

	while (1)
	{
		const char* line = ebt_file_get_line();

		if (!line) break;

		uint8_t param = ebt_parse_hex8(line + 2);

		if (ebt_parse_tag(line, "ou")) config.output = param;
		if (ebt_parse_tag(line, "bl")) config.cursor_blink = param;
		if (ebt_parse_tag(line, "sb")) config.screen_brightness = param;
		if (ebt_parse_tag(line, "fn")) config.font = param;
		if (ebt_parse_tag(line, "oh")) config.ord_highlight = param;
		if (ebt_parse_tag(line, "pt")) config.ptn_sound_type = param;
		if (ebt_parse_tag(line, "pl")) config.ptn_sound_len = param;
		if (ebt_parse_tag(line, "iv")) config.i2s_volume = param;
		if (ebt_parse_tag(line, "sw")) config.swap_lft_rgt = param;
		if (ebt_parse_tag(line, "al")) config.auto_load = param;
		if (ebt_parse_tag(line, "hi")) config.hints = param;
		if (ebt_parse_tag(line, "bu")) config.backup = param;
#ifdef TARGET_SDL
		if (ebt_parse_tag(line, "vm")) config.video_mode = param;
		if (ebt_parse_tag(line, "kb")) config.full_kb = param;
#endif
		if (ebt_parse_tag(line, "sn")) ebt_song_set_last_name(line + 2);
	}

	ebt_file_close();

	memcpy(&config_prev, &config, sizeof(config_prev));

	ebt_config_update_brightness();
}



void ebt_config_save(uint8_t force)
{
	if (!force && (memcmp(&config, &config_prev, sizeof(config)) == 0)) return; //no changes in the config, no need to save

	if (!ebt_file_open(FILE_NAME_CFG, TRUE)) return;

	ebt_file_put_line(ebt_make_hex8("ou", config.output));
	ebt_file_put_line(ebt_make_hex8("bl", config.cursor_blink));
	ebt_file_put_line(ebt_make_hex8("sb", config.screen_brightness));
	ebt_file_put_line(ebt_make_hex8("fn", config.font));
	ebt_file_put_line(ebt_make_hex8("oh", config.ord_highlight));
	ebt_file_put_line(ebt_make_hex8("pt", config.ptn_sound_type));
	ebt_file_put_line(ebt_make_hex8("pl", config.ptn_sound_len));
	ebt_file_put_line(ebt_make_hex8("iv", config.i2s_volume));
	ebt_file_put_line(ebt_make_hex8("sw", config.swap_lft_rgt));
	ebt_file_put_line(ebt_make_hex8("al", config.auto_load));
	ebt_file_put_line(ebt_make_hex8("hi", config.hints));
	ebt_file_put_line(ebt_make_hex8("bu", config.backup));
#ifdef TARGET_SDL
	ebt_file_put_line(ebt_make_hex8("vm", config.video_mode));
	ebt_file_put_line(ebt_make_hex8("kb", config.full_kb));
#endif

	char buf[MAX_PATH];
	snprintf(buf, sizeof(buf), "sn%s", ebt_song_get_last_name());
	ebt_file_put_line(buf);

	ebt_file_close();

	memcpy(&config_prev, &config, sizeof(config_prev));
}



void ebt_edit_config_init(void)
{
}



uint8_t ebt_config_get_blink_mask(void)
{
	switch (config.cursor_blink)
	{
	case CFG_CURSOR_BLINK_FAST: return 4;
	case CFG_CURSOR_BLINK_MED: return 8;
	case CFG_CURSOR_BLINK_SLOW: return 16;
	}

	return 0;
}



uint8_t ebt_config_get_font(void)
{
	return config.font;
}



uint8_t ebt_config_get_ord_highlight(void)
{
	return config.ord_highlight;
}



BOOL ebt_config_get_swap_lft_rgt(void)
{
	return config.swap_lft_rgt;
}



BOOL ebt_config_get_full_kb(void)
{
#ifdef TARGET_SDL
	return config.full_kb;
#else
	return 0;
#endif
}



BOOL ebt_config_get_auto_load(void)
{
	return config.auto_load;
}



BOOL ebt_config_get_backup(void)
{
	return config.backup;
}



void ebt_edit_config_draw(void)
{
	char buf[16];

	ebt_put_header("CONF", COL_HEAD_CONF);

	int sy = 2;

#ifndef TARGET_SDL
	const char* str_output = "INT";

	switch (config.output)
	{
	case CFG_OUTPUT_EXT_SDM: str_output = "EXT SDM"; break;
	case CFG_OUTPUT_ALL_SDM: str_output = "INT+EXT"; break;
	case CFG_OUTPUT_EXT_PWM: str_output = "EXT PWM"; break;
	case CFG_OUTPUT_EXT_I2S: str_output = "EXT I2S"; break;
	}

	set_font_color(COL_TEXT_DARK);
	set_back_color(COL_BACK);
	put_str(1, sy, "OUTPUT");

	ebt_item_color(config_cur == CFG_ITEM_OUTPUT);

	put_str(8, sy, str_output);

	++sy;

	if (config.output == CFG_OUTPUT_EXT_I2S)
	{
		snprintf(buf, sizeof(buf), "%i", config.i2s_volume);

		set_font_color(COL_TEXT_DARK);
		set_back_color(COL_BACK);
		put_str(1, sy, "VOLUME");

		ebt_item_color(config_cur == CFG_ITEM_I2S_VOLUME);

		put_str(8, sy, buf);

		++sy;
	}

#endif

	const char* str_blink = "NONE";

	switch (config.cursor_blink)
	{
	case CFG_CURSOR_BLINK_FAST: str_blink = "FAST"; break;
	case CFG_CURSOR_BLINK_MED: str_blink = "MED"; break;
	case CFG_CURSOR_BLINK_SLOW: str_blink = "SLOW"; break;
	}

	set_font_color(COL_TEXT_DARK);
	set_back_color(COL_BACK);
	put_str(1, sy, "BLINK");

	ebt_item_color(config_cur == CFG_ITEM_CURSOR_BLINK);

	put_str(8, sy, str_blink);

	++sy;

#ifndef TARGET_SDL

	snprintf(buf, sizeof(buf), "%i", config.screen_brightness);

	set_font_color(COL_TEXT_DARK);
	set_back_color(COL_BACK);
	put_str(1, sy, "BRIGHT");

	ebt_item_color(config_cur == CFG_ITEM_BRIGHTNESS);

	put_str(8, sy, buf);

	++sy;

#endif

	const char* str_font = "CUSTOM";

	switch (config.font)
	{
	case CFG_FONT_C64: str_font = "C64"; break;
	case CFG_FONT_MSX: str_font = "MSX"; break;
	case CFG_FONT_ZX: str_font = "ZX"; break;
	}

	set_font_color(COL_TEXT_DARK);
	set_back_color(COL_BACK);
	put_str(1, sy, "FONT");

	ebt_item_color(config_cur == CFG_ITEM_FONT);

	put_str(8, sy, str_font);

	++sy;

#ifdef TARGET_SDL

	set_font_color(COL_TEXT_DARK);
	set_back_color(COL_BACK);
	put_str(1, sy, "VMODE");

	ebt_item_color(config_cur == CFG_ITEM_VIDEO_MODE);

	const char* str_vmode = "128128";

	switch (config.video_mode)
	{
	case CFG_VIDEO_MODE_240_320: str_vmode = "240320"; break;
	}

	put_str(8, sy, str_vmode);

	++sy;

#endif

	set_font_color(COL_TEXT_DARK);
	set_back_color(COL_BACK);
	put_str(1, sy, "ORD.HL");

	ebt_item_color(config_cur == CFG_ITEM_ORD_HIGHLIGHT);

	put_str(8, sy, config.ord_highlight ? "ON" : "OFF");

	++sy;

	const char* ptn_sound_str = "MUTE";

	switch (config.ptn_sound_type)
	{
	case CFG_SOUND_TYPE_PRESS: ptn_sound_str = "PRESS"; break;
	case CFG_SOUND_TYPE_CHANGE: ptn_sound_str = "CHANGE"; break;
	case CFG_SOUND_TYPE_RELEASE: ptn_sound_str = "RELEASE"; break;
	}

	set_font_color(COL_TEXT_DARK);
	set_back_color(COL_BACK);
	put_str(1, sy, "PTNSND");

	ebt_item_color(config_cur == CFG_ITEM_PTN_SOUND_TYPE);

	put_str(8, sy, ptn_sound_str);

	++sy;

	if (config.ptn_sound_len == 0)
	{
		strncpy(buf, "SPEED", sizeof(buf));
	}
	else
	{
		snprintf(buf, sizeof(buf), "%2.2X", config.ptn_sound_len);
	}

	set_font_color(COL_TEXT_DARK);
	set_back_color(COL_BACK);
	put_str(1, sy, "SNDLEN");

	ebt_item_color(config_cur == CFG_ITEM_PTN_SOUND_LEN);

	put_str(8, sy, buf);

	++sy;

	set_font_color(COL_TEXT_DARK);
	set_back_color(COL_BACK);
	put_str(1, sy, "SWP LR");
	
	ebt_item_color(config_cur == CFG_ITEM_SWAP_LFT_RGT);

	put_str(8, sy, config.swap_lft_rgt ? "YES" : "NO");

	++sy;

	set_font_color(COL_TEXT_DARK);
	set_back_color(COL_BACK);
	put_str(1, sy, "AUTOLD");

	ebt_item_color(config_cur == CFG_ITEM_AUTO_LOAD);

	put_str(8, sy, config.auto_load ? "YES" : "NO");

	++sy;

	set_font_color(COL_TEXT_DARK);
	set_back_color(COL_BACK);
	put_str(1, sy, "HINTS");

	ebt_item_color(config_cur == CFG_ITEM_HINTS);

	put_str(8, sy, config.hints ? "YES" : "NO");

	++sy;

	set_font_color(COL_TEXT_DARK);
	set_back_color(COL_BACK);
	put_str(1, sy, "BACKUP");

	ebt_item_color(config_cur == CFG_ITEM_BACKUP);

	put_str(8, sy, config.backup ? "YES" : "NO");

#ifdef TARGET_SDL

	++sy;

	set_font_color(COL_TEXT_DARK);
	set_back_color(COL_BACK);
	put_str(1, sy, "FULLKB");

	ebt_item_color(config_cur == CFG_ITEM_FULL_KB);

	put_str(8, sy, config.full_kb ? "YES" : "NO");

#endif

	++sy;

	ebt_item_color(config_cur == CFG_ITEM_FILE_MANAGER);

	put_str(1, sy, "FMAN");
	
#ifndef TARGET_SDL

	++sy;

	ebt_item_color(config_cur == CFG_ITEM_WIFI_AP);

	put_str(1, sy, "WIFI AP");

#endif
}



void ebt_edit_config_wifi_ap_ok(void)
{
	ebt_song_backup();
	ebt_stop();
}



void ebt_edit_config_move_cur(int dx)
{
	config_cur += dx;

#ifndef TARGET_SDL
	if (config.output != CFG_OUTPUT_EXT_I2S)
	{
		if (config_cur == CFG_ITEM_I2S_VOLUME) config_cur += dx;
	}
#endif

	if (config_cur < 0) config_cur = CFG_ITEMS_ALL - 1;
	if (config_cur >= CFG_ITEMS_ALL) config_cur = 0;
}



void ebt_edit_config_update(void)
{
	int pad_t = ebt_input_get_trigger();
	int pad_r = ebt_input_get_repeat();
	int pad = ebt_input_get_state();

	if (!(pad & PAD_ACT))
	{
		if (pad_r & PAD_UP) ebt_edit_config_move_cur(-1);
		if (pad_r & PAD_DOWN) ebt_edit_config_move_cur(1);

		ebt_config_save(FALSE);	//does not save if there was no changes, so it safe to call it every frame
	}
	else
	{
		switch (config_cur)
		{
#ifndef TARGET_SDL
		case CFG_ITEM_WIFI_AP:
			if (pad_t & PAD_ACT)
			{
				ebt_ask_confirm("SURE?", ebt_edit_config_wifi_ap_ok, NULL);
			}
			break;
#endif
		case CFG_ITEM_FILE_MANAGER:
			ebt_file_manager();
			break;
		}
	}

	switch (config_cur)
	{
#ifndef TARGET_SDL
	case CFG_ITEM_OUTPUT:
		if (ebt_change_param_u8(&config.output, pad, pad_r, 0, 0, CFG_OUTPUTS_ALL - 1, 1))
		{
			ebt_config_update_sound_device();
		}
		break;
	case CFG_ITEM_I2S_VOLUME: ebt_change_param_u8(&config.i2s_volume, pad, pad_r, 9, 1, 9, 1); break;
	case CFG_ITEM_BRIGHTNESS:
		if (ebt_change_param_u8(&config.screen_brightness, pad, pad_r, 9, 1, 9, 1))
		{
			ebt_config_update_brightness();
		}
		break;
#endif
#ifdef TARGET_SDL
	case CFG_ITEM_VIDEO_MODE:
		if (ebt_change_param_u8(&config.video_mode, pad, pad_r, 0, 0, CFG_VIDEO_MODES_ALL - 1, 1))
		{
			sdl_video_change_size();
		}
		break;
	case CFG_ITEM_FULL_KB:
		ebt_change_param_u8(&config.full_kb, pad, pad_r, 0, 0, 1, 1);
		break;
#endif
	case CFG_ITEM_CURSOR_BLINK: ebt_change_param_u8(&config.cursor_blink, pad, pad_r, CFG_CURSOR_BLINK_MED, 0, CFG_BLINKS_ALL - 1, 1); break;
	case CFG_ITEM_FONT: ebt_change_param_u8(&config.font, pad, pad_r, 0, 0, CFG_FONTS_ALL - 1, 1); break;
	case CFG_ITEM_ORD_HIGHLIGHT: ebt_change_param_u8(&config.ord_highlight, pad, pad_r, 0, 0, 1, 1); break;
	case CFG_ITEM_PTN_SOUND_TYPE: ebt_change_param_u8(&config.ptn_sound_type, pad, pad_r, 0, 0, CFG_SOUND_TYPES_ALL - 1, 1); break;
	case CFG_ITEM_PTN_SOUND_LEN: ebt_change_param_u8(&config.ptn_sound_len, pad, pad_r, 0, 0, 0x20, 1); break;
	case CFG_ITEM_SWAP_LFT_RGT: ebt_change_param_u8(&config.swap_lft_rgt, pad, pad_r, 0, 0, 1, 1); break;
	case CFG_ITEM_AUTO_LOAD: ebt_change_param_u8(&config.auto_load, pad, pad_r, 0, 0, 1, 1); break;
	case CFG_ITEM_HINTS: ebt_change_param_u8(&config.hints, pad, pad_r, 0, 0, 1, 1); break;
	case CFG_ITEM_BACKUP: ebt_change_param_u8(&config.backup, pad, pad_r, 0, 0, 1, 1); break;
	}
}
