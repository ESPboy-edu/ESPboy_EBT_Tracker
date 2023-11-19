BOOL ebt_change_param_u8(uint8_t* pparam_u8, uint8_t pad, uint8_t pad_t, uint8_t def, uint8_t min, uint8_t max, uint8_t large_step)
{
	int add = 0;
	int param = *pparam_u8;

	int kb_hex = ebt_input_get_kb_hex();

	if (kb_hex >= 0)
	{
		param = ((param << 4) + kb_hex) & 0xff;
		pad = 0;
	}

	if (ebt_input_get_kb() == KB_BACKSPACE)
	{
		param = 0;
		pad = 0;
	}
	
	if (pad&PAD_ACT)
	{
		if (pad_t&PAD_ESC)
		{
			param = def;
		}
		else
		{
			if (pad_t&PAD_LEFT) add = -1;
			if (pad_t&PAD_RIGHT) add = 1;
			if (pad_t&PAD_UP) add = large_step;
			if (pad_t&PAD_DOWN) add = -large_step;

			param += add;
		}
	}

	if (param < min) param = max;
	if (param > max) param = min;

	if (*pparam_u8 != param)
	{
		*pparam_u8 = param;
		return TRUE;
	}

	return FALSE;
}



BOOL ebt_change_param_note(uint8_t* pparam_u8, uint8_t pad, uint8_t pad_t)
{
	int add = 0;
	int param = *pparam_u8;

	char c = ebt_input_get_kb_char();

	if (c >= '1'&&c <= '9')
	{
		param = (c - '0') * 12 + (param % 12);
	}
	
	switch (c)
	{
	case 'Z': param = (param - param % 12) + 0; break;
	case 'S': param = (param - param % 12) + 1; break;
	case 'X': param = (param - param % 12) + 2; break;
	case 'D': param = (param - param % 12) + 3; break;
	case 'C': param = (param - param % 12) + 4; break;
	case 'V': param = (param - param % 12) + 5; break;
	case 'G': param = (param - param % 12) + 6; break;
	case 'B': param = (param - param % 12) + 7; break;
	case 'H': param = (param - param % 12) + 8; break;
	case 'N': param = (param - param % 12) + 9; break;
	case 'J': param = (param - param % 12) + 10; break;
	case 'M': param = (param - param % 12) + 11; break;
	}

	if (pad&PAD_ACT)
	{
		if (pad_t&PAD_ESC)
		{
			param = 3*12;
		}
		else
		{
			if (pad_t&PAD_LEFT) add = -1;
			if (pad_t&PAD_RIGHT) add = 1;
			if (pad_t&PAD_UP) add = 12;
			if (pad_t&PAD_DOWN) add = -12;

			param += add;
		}
	}

	const int min = 12;
	const int max = 12 * 10 - 1;

	if (param < min) param = max;
	if (param > max) param = min;

	if (*pparam_u8 != param)
	{
		*pparam_u8 = param;
		return TRUE;
	}

	return FALSE;
}



BOOL ebt_change_param_i8(signed char* pparam_i8, uint8_t pad, uint8_t pad_t, int8_t def, int8_t min, int8_t max, int8_t large_step, uint8_t no_zero)
{
	int add = 0;
	int param = *pparam_i8;

	int kb_hex = ebt_input_get_kb_hex();

	if (kb_hex >= 0)
	{
		param = ((param << 4) + kb_hex) & 0xff;
		pad = 0;

		if (no_zero && (param == 0)) param = 1;
	}

	if (ebt_input_get_kb() == KB_MINUS)
	{
		param = -param;
		pad = 0;
	}

	if (!no_zero)
	{
		if (ebt_input_get_kb() == KB_BACKSPACE)
		{
			param = 0;
			pad = 0;
		}
	}

	if (pad&PAD_ACT)
	{
		if (pad_t&PAD_ESC)
		{
			param = def;
		}
		else
		{
			if (pad_t&PAD_LEFT) add = -1;
			if (pad_t&PAD_RIGHT) add = 1;
			if (pad_t&PAD_UP) add = large_step;
			if (pad_t&PAD_DOWN) add = -large_step;

			param += add;

			if (param == 0 && no_zero) param += add;
		}
	}

	if (param < min) param = max;
	if (param > max) param = min;

	if (*pparam_i8 != param)
	{
		*pparam_i8 = param;
		return TRUE;
	}

	return FALSE;
}



void ebt_item_color_custom(BOOL active, uint8_t col_active_1, uint8_t col_active_2, uint8_t col_inactive)
{
	if (active)
	{
		set_font_color(COL_TEXT);
		set_back_color(!(ebt_get_blink() & ebt_config_get_blink_mask() && !navi_active) ? col_active_1 : col_active_2);
	}
	else
	{
		set_font_color(COL_TEXT);
		set_back_color(col_inactive);
	}
}



void ebt_item_color(BOOL active)
{
	ebt_item_color_custom(active, COL_BACK_CUR_2, COL_BACK_CUR_1, COL_BACK);
}



char hex_to_char(uint8_t n)
{
	return (n < 10) ? ('0' + n) : 'A' + n - 10;
}



void set_edit_mode(int new_mode)
{
	if (edit_mode == new_mode) return;

	edit_mode = new_mode;

	switch (edit_mode)
	{
	case EDIT_MODE_INFO: ebt_edit_info_init(); break;
	case EDIT_MODE_SONG: ebt_edit_song_init(); break;
	case EDIT_MODE_ORDER: ebt_edit_order_init(); break;
	case EDIT_MODE_PATTERN: ebt_edit_pattern_init(); break;
	case EDIT_MODE_INSTRUMENT: ebt_edit_instrument_init(); break;
	case EDIT_MODE_CONFIG: ebt_edit_config_init(); break;
	case EDIT_MODE_FILE_BROWSER: ebt_file_browser_init(); break;
	case EDIT_MODE_INPUT_NAME: ebt_input_name_init(); break;
	}

	ebt_player_stop();

	osd_message_clear();
}



void ebt_init(void)
{
	ebt_frame_cnt = 0;

	ebt_config_set_default();
	ebt_config_load();

	song = new song_struct;	//needs to be allocated rather than static in the ESP8266 environment to allow free RAM for OTA and file web server

	memset(&clipboard_ins, 0, sizeof(clipboard_ins));
	clipboard_ins_empty = true;

	memset(&clipboard_pattern, 0, sizeof(clipboard_pattern));
	clipboard_pattern_empty = true;

	memset(clipboard_order, 0, sizeof(clipboard_order));
	clipboard_order_w = 0;
	clipboard_order_h = 0;
	clipboard_order_type = ORDER_CLIPBOARD_NONE;

	ebt_input_init();
	ebt_player_init(SAMPLE_RATE);

	ebt_song_clear(TRUE);

	set_edit_mode(EDIT_MODE_INFO);

	ebt_active = TRUE;
}



BOOL ebt_is_active(void)
{
	return ebt_active;
}



void ebt_stop(void)
{
	ebt_active = FALSE;
}



void ebt_shut(void)
{
	ebt_active = FALSE;

	if (song)
	{
		delete song;
		song = NULL;
	}
}



void ebt_update(void)
{
	++ebt_frame_cnt;
	++ebt_blink_cnt;

	int pad = ebt_input_get_state();
	int pad_t = ebt_input_get_trigger();

	if (pad_t) ebt_blink_cnt = 0;	//blink resets on new presses to make cursor more vibisle

	if (!(pad&PAD_LFT))
	{
		if (pad&~PAD_RGT) ebt_blink_cnt = 0;
	}

	screen_clear();

	switch (edit_mode)
	{
	case EDIT_MODE_INFO: ebt_edit_info_draw(); break;
	case EDIT_MODE_SONG: ebt_edit_song_draw(); break;
	case EDIT_MODE_ORDER: ebt_edit_order_draw(); break;
	case EDIT_MODE_PATTERN: ebt_edit_pattern_draw(); break;
	case EDIT_MODE_INSTRUMENT: ebt_edit_instrument_draw(); break;
	case EDIT_MODE_CONFIG: ebt_edit_config_draw(); break;
	case EDIT_MODE_FILE_BROWSER: ebt_file_browser_draw(); break;
	case EDIT_MODE_INPUT_NAME: ebt_input_name_draw(); break;
	}

	osd_update();

	if (ebt_confirm_update()) return;

	switch (edit_mode)
	{
	case EDIT_MODE_SONG:
	case EDIT_MODE_INFO:
	{
		if (pad_t&PAD_RGT)
		{
			if (!ebt_player_is_active())	//play and stop
			{
				ebt_player_start_song();
			}
			else
			{
				ebt_player_stop();
			}

			return;
		}

		if (pad&PAD_RGT)
		{
			if (ebt_player_is_active())
			{
				osd_message_set_custom("PLAY SONG",6);
			}
			else
			{
				osd_message_set_custom("STOP",6);
			}

			return;
		}
	}
	break;

	case EDIT_MODE_ORDER:
	{
		if (pad_t&PAD_RGT)
		{
			if (!ebt_player_is_active())	//play and stop
			{
				ebt_player_start_order((uint8_t)cur_ord_pos);
			}
			else
			{
				ebt_player_stop();
			}

			return;
		}

		if (pad&PAD_RGT)
		{
			if (ebt_player_is_active())
			{
				osd_message_set_custom("PLAY ORD", 6);
			}
			else
			{
				osd_message_set_custom("STOP", 6);
			}

			return;
		}
	}
	break;

	case EDIT_MODE_PATTERN:
	{
		if (!(pad&PAD_RGT))
		{
			if (ebt_player_is_active() && ebt_player_is_pattern_test_mode())
			{
				ebt_player_stop();
			}
		}

		if (pad_t&PAD_RGT)
		{
			ebt_player_start_pattern_test((uint8_t)cur_ptn_num, (uint8_t)cur_ptn_row);
		}

		if (pad&PAD_RGT)
		{
			osd_message_show("PLAY PTN");
			return;
		}
	}
	break;

	case EDIT_MODE_INSTRUMENT:
	{
		if (!(pad&PAD_RGT)) ebt_player_stop();

		if (pad_t&PAD_RGT)
		{
			ebt_player_start_instrument_test((uint8_t)cur_ins);
		}

		if (pad&PAD_RGT)
		{
			osd_message_show("PLAY INS");
			return;
		}
	}
	break;
	}

	if (pad_t) osd_message_clear();

	int navi = TRUE;

	if (edit_mode == EDIT_MODE_FILE_BROWSER) navi = FALSE;
	if (edit_mode == EDIT_MODE_INPUT_NAME) navi = FALSE;

	if (navi)
	{
		switch (ebt_input_get_kb())
		{
		case KB_F1: set_edit_mode(EDIT_MODE_INFO); break;
		case KB_F2: set_edit_mode(EDIT_MODE_ORDER); break;
		case KB_F3: set_edit_mode(EDIT_MODE_PATTERN); break;
		case KB_F4: set_edit_mode(EDIT_MODE_INSTRUMENT); break;
		case KB_F9: set_edit_mode(EDIT_MODE_SONG); break;
		case KB_F10: set_edit_mode(EDIT_MODE_CONFIG); break;
		}
	}

	if ((pad&PAD_LFT) && navi)
	{
		navi_active = TRUE;

		ebt_fade_screen();

		set_font_color(COL_TEXT_DARK);
		set_back_color(COL_BACK);

		put_str(6, 1, "NAVI");

		set_font_color(COL_TEXT);

		ebt_navi_item_color((edit_mode == EDIT_MODE_INFO), COL_HEAD_INFO);
		put_str(6, 3, "INFO");

		ebt_navi_item_color((edit_mode == EDIT_MODE_SONG), COL_HEAD_SONG);

		put_str(6, 5, "SONG");

		ebt_navi_item_color((edit_mode == EDIT_MODE_ORDER), COL_HEAD_ORD);

		put_str(2, 7, "ORD");

		ebt_navi_item_color((edit_mode == EDIT_MODE_PATTERN), COL_HEAD_PTN);

		put_str(11, 7, "PTN");

		ebt_navi_item_color((edit_mode == EDIT_MODE_INSTRUMENT), COL_HEAD_INS);

		put_str(6, 9, "INST");

		ebt_navi_item_color((edit_mode == EDIT_MODE_CONFIG), COL_HEAD_CONF);

		put_str(6, 11, "CONF");

		//mode-specific copy/paste functions

		switch (edit_mode)
		{
		case EDIT_MODE_INSTRUMENT:
		{
			set_back_color(COL_HEAD_FILE);
			put_str(1, 14, "ACT");
			set_back_color(COL_HEAD_NAME);
			put_str(9, 14, "ESC");
			set_back_color(COL_BACK);
			put_str(4, 14, "CPY");
			put_str(12, 14, "PST");

			if (pad_t&PAD_ACT) ebt_instrument_copy();
			if (pad_t&PAD_ESC) ebt_instrument_paste();
		}
		break;

		case EDIT_MODE_PATTERN:
		{
			set_back_color(COL_HEAD_FILE);
			put_str(1, 14, "ACT");
			set_back_color(COL_HEAD_NAME);
			put_str(9, 14, "ESC");
			set_back_color(COL_BACK);
			put_str(4, 14, "CPY");
			put_str(12, 14, "PST");

			if (pad_t&PAD_ACT) ebt_pattern_copy();
			if (pad_t&PAD_ESC) ebt_pattern_paste();
		}
		break;

		case EDIT_MODE_ORDER:
		{
			set_back_color(COL_HEAD_FILE);
			put_str(1, 14, "ACT");
			set_back_color(COL_HEAD_NAME);
			put_str(9, 14, "ESC");
			set_back_color(COL_BACK);
			put_str(4, 14, "CPY");
			put_str(12, 14, "PST");

			if (pad_t&PAD_ACT) ebt_order_copy();
			if (pad_t&PAD_ESC) ebt_order_paste();
		}
		break;
		}

		//in the UDLR order

		const uint8_t mode_matrix[] = {
			EDIT_MODE_CONFIG		,EDIT_MODE_SONG			,EDIT_MODE_ORDER		,EDIT_MODE_PATTERN,	//EDIT_MODE_TITLE
			EDIT_MODE_INFO			,EDIT_MODE_INSTRUMENT	,EDIT_MODE_ORDER		,EDIT_MODE_PATTERN,	//EDIT_MODE_SONG		
			EDIT_MODE_SONG			,EDIT_MODE_INSTRUMENT	,EDIT_MODE_ORDER		,EDIT_MODE_PATTERN,	//EDIT_MODE_ORDER
			EDIT_MODE_SONG			,EDIT_MODE_INSTRUMENT	,EDIT_MODE_ORDER		,EDIT_MODE_PATTERN,	//EDIT_MODE_PATTERN
			EDIT_MODE_SONG			,EDIT_MODE_CONFIG		,EDIT_MODE_ORDER		,EDIT_MODE_PATTERN,	//EDIT_MODE_INSTRUMENT
			EDIT_MODE_INSTRUMENT	,EDIT_MODE_INFO			,EDIT_MODE_ORDER		,EDIT_MODE_PATTERN,	//EDIT_MODE_CONFIG
			EDIT_MODE_FILE_BROWSER	,EDIT_MODE_FILE_BROWSER	,EDIT_MODE_FILE_BROWSER	,EDIT_MODE_FILE_BROWSER, //EDIT_MODE_FILE_BROWSER
			EDIT_MODE_INPUT_NAME			,EDIT_MODE_INPUT_NAME			,EDIT_MODE_INPUT_NAME			,EDIT_MODE_INPUT_NAME, //EDIT_MODE_NAME
		};

		int off = -1;

		if (pad_t&PAD_UP) off = 0;
		if (pad_t&PAD_DOWN) off = 1;
		if (pad_t&PAD_LEFT) off = 2;
		if (pad_t&PAD_RIGHT) off = 3;

		if (off >= 0) set_edit_mode(mode_matrix[edit_mode * 4 + off]);

		return;
	}
	else
	{
		navi_active = FALSE;
	}

	switch (edit_mode)
	{
	case EDIT_MODE_INFO: ebt_edit_info_update(); break;
	case EDIT_MODE_SONG: ebt_edit_song_update(); break;
	case EDIT_MODE_ORDER: ebt_edit_order_update(); break;
	case EDIT_MODE_PATTERN: ebt_edit_pattern_update(); break;
	case EDIT_MODE_INSTRUMENT: ebt_edit_instrument_update(); break;
	case EDIT_MODE_CONFIG: ebt_edit_config_update(); break;
	case EDIT_MODE_FILE_BROWSER: ebt_file_browser_update(); break;
	case EDIT_MODE_INPUT_NAME: ebt_input_name_update(); break;
	}
}
