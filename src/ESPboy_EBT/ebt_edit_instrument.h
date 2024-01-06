enum {
	INS_ITEM_WAVE = 0,
	INS_ITEM_VOLUME,
	INS_ITEM_OFFSET,
	INS_ITEM_DETUNE,
	INS_ITEM_SLIDE,
	INS_ITEM_VIBRATO_DELAY,
	INS_ITEM_VIBRATO_SPEED,
	INS_ITEM_VIBRATO_DEPTH,
	INS_ITEM_CUT_TIME,
	INS_ITEM_FIXED_PITCH,
	INS_ITEM_BASE_NOTE,
	INS_ITEM_AUX_ID,
	INS_ITEM_AUX_MIX,
	INS_ITEM_NAME,
	INS_ITEM_LOAD,
	INS_ITEM_SAVE
};

#define INS_ITEM_PAGE_1_FROM	INS_ITEM_WAVE
#define INS_ITEM_PAGE_1_TO		INS_ITEM_AUX_MIX
#define INS_ITEM_PAGE_2_FROM	INS_ITEM_NAME
#define INS_ITEM_PAGE_2_TO		INS_ITEM_SAVE

int8_t ebt_ins_item[2] = { INS_ITEM_PAGE_1_FROM,INS_ITEM_PAGE_2_FROM };
const int8_t ebt_ins_item_min[2] = { INS_ITEM_PAGE_1_FROM,INS_ITEM_PAGE_2_FROM };
const int8_t ebt_ins_item_max[2] = { INS_ITEM_PAGE_1_TO,INS_ITEM_PAGE_2_TO };

uint8_t ebt_ins_page = 0;


int16_t ebt_ins_ptn_refs = -1;
int16_t ebt_ins_aux_refs = -1;

int8_t ins_menu_double_tap = 0;


void ebt_edit_instrument_init(void)
{
	ebt_ins_ptn_refs = -1;
	ebt_ins_aux_refs = -1;
	ins_menu_double_tap = 0;
}



void ebt_edit_instrument_draw(void)
{
	char buf[16];

	snprintf(buf, sizeof(buf), "INST%2.2X", cur_ins);

	ebt_put_header(buf, COL_HEAD_INS);

	set_font_color(COL_TEXT_DARK);
	set_back_color(COL_BACK);

	for (int i = 0; i < MAX_INSTRUMENT_NAME_LEN; ++i)
	{
		char c = song->ins[cur_ins].name[i];
		if (c >= ' ') put_char(Text.width - 4 - MAX_INSTRUMENT_NAME_LEN + i, 0, c);
	}

	snprintf(buf, sizeof(buf), "%i/2", ebt_ins_page + 1);

	put_str((signed char)(Text.width - strlen(buf)), 0, buf);

	const char* wave_names[SYNTH_WAVEFORMS_MAX] = {
		"P50%","P32%","P25%","P19%","P12%","P06%","SWPF","SWPS",
		"SWLO","SWHI","DSS-","DSS+","DSF-","VOW1","VOW2","VOW3",
		"VOW4","VOW5","VOW6","RSP1","RSP2","PHT1","PHT2","PHT3",
		"PHT4","PHT5","PHT6","PHT7","NSE1","NSE2","NSE3","NSE4",
		"ADSL","ADSH","ADSO","APH1","APH2","HARM","FPLS","FPLO",
		"SPLS","SPLO","SPLA","HPT1","HPT2","DRTY","PNS1","PNS2",
		"MTLC","WNS1","WNS2","WNS3","WNS4","LNS1","LNS2","LNS3",
		"LNS4","HNS1","HNS2","HNS3","HNS4","MOD1","MOD2","MOD3",
	};

	instrument_struct* is = &song->ins[cur_ins];

	int sy = 2;

	uint8_t item = ebt_ins_item[ebt_ins_page];

	if (ebt_ins_page == 0)
	{
		set_font_color(COL_TEXT_DARK);
		set_back_color(COL_BACK);
		put_str(1, sy, "WAVE");

		snprintf(buf, sizeof(buf), "%2.2X", is->wave);

		ebt_item_color(item == INS_ITEM_WAVE);
		put_str(9, sy, buf);

		set_back_color(COL_BACK);
		set_font_color(COL_TEXT_DARK);
		put_str(12, sy, wave_names[is->wave]);

		++sy;

		set_font_color(COL_TEXT_DARK);
		set_back_color(COL_BACK);
		put_str(1, sy, "VOLUME");

		snprintf(buf, sizeof(buf), "%u", is->volume);

		ebt_item_color(item == INS_ITEM_VOLUME);
		put_str(9, sy, buf);

		++sy;

		set_font_color(COL_TEXT_DARK);
		set_back_color(COL_BACK);
		put_str(1, sy, "OFFSET");

		snprintf(buf, sizeof(buf), "%2.2X", abs(is->offset));

		if (is->offset < 0) put_str(8, sy, "-");
		ebt_item_color(item == INS_ITEM_OFFSET);
		put_str(9, sy, buf);

		++sy;

		set_font_color(COL_TEXT_DARK);
		set_back_color(COL_BACK);
		put_str(1, sy, "DETUNE ");

		snprintf(buf, sizeof(buf), "%2.2X", abs(is->detune));

		if (is->detune < 0) put_str(8, sy, "-");
		ebt_item_color(item == INS_ITEM_DETUNE);
		put_str(9, sy, buf);

		++sy;

		set_font_color(COL_TEXT_DARK);
		set_back_color(COL_BACK);
		put_str(1, sy, "SLIDE");

		snprintf(buf, sizeof(buf), "%2.2X", abs(is->slide));

		if (is->slide < 0) put_str(8, sy, "-");
		ebt_item_color(item == INS_ITEM_SLIDE);
		put_str(9, sy, buf);

		++sy;

		set_font_color(COL_TEXT_DARK);
		set_back_color(COL_BACK);
		put_str(1, sy, "VIB.DL");

		snprintf(buf, sizeof(buf), "%2.2X", is->mod_delay);

		ebt_item_color(item == INS_ITEM_VIBRATO_DELAY);
		put_str(9, sy, buf);

		++sy;

		set_font_color(COL_TEXT_DARK);
		set_back_color(COL_BACK);
		put_str(1, sy, "VIB.SP");

		snprintf(buf, sizeof(buf), "%2.2X", is->mod_speed);

		ebt_item_color(item == INS_ITEM_VIBRATO_SPEED);
		put_str(9, sy, buf);

		++sy;

		set_font_color(COL_TEXT_DARK);
		set_back_color(COL_BACK);
		put_str(1, sy, "VIB.DP");

		snprintf(buf, sizeof(buf), "%2.2X", is->mod_depth);

		ebt_item_color(item == INS_ITEM_VIBRATO_DEPTH);
		put_str(9, sy, buf);

		++sy;

		set_font_color(COL_TEXT_DARK);
		set_back_color(COL_BACK);
		put_str(1, sy, "CUT");

		if (is->cut_time > 0)
		{
			snprintf(buf, sizeof(buf), "%2.2X", is->cut_time);
		}
		else
		{
			strcpy(buf, "OFF");
		}

		ebt_item_color(item == INS_ITEM_CUT_TIME);
		put_str(9, sy, buf);

		++sy;

		set_font_color(COL_TEXT_DARK);
		set_back_color(COL_BACK);
		put_str(1, sy, "FIX.PT");

		ebt_item_color(item == INS_ITEM_FIXED_PITCH);
		put_str(9, sy, is->fixed_pitch ? "ON" : "OFF");

		++sy;

		set_font_color(COL_TEXT_DARK);
		set_back_color(COL_BACK);
		put_str(1, sy, "NOTE");

		ebt_item_color(item == INS_ITEM_BASE_NOTE);

		snprintf(buf, sizeof(buf), "%s%i", note_names[is->base_note % 12], is->base_note / 12);
		put_str(9, sy, buf);

		++sy;

		set_font_color(COL_TEXT_DARK);
		set_back_color(COL_BACK);
		put_str(1, sy, "AUX.INS");

		if (is->aux_id != 0)
		{
			snprintf(buf, sizeof(buf), "%2.2X", abs(is->aux_id));
			if(is->aux_id<0) put_str(8, sy, "-");
		}
		else
		{
			strcpy(buf, "NONE");
		}

		ebt_item_color(item == INS_ITEM_AUX_ID);
		put_str(9, sy, buf);

		++sy;

		set_font_color(COL_TEXT_DARK);
		set_back_color(COL_BACK);
		put_str(1, sy, "AUX.MIX");

		const char* mix_str = "NONE";

		switch (is->aux_mix)
		{
		case SYNTH_MIX_ADD: mix_str = "ADD"; break;
		case SYNTH_MIX_SUB: mix_str = "SUB"; break;
		}

		ebt_item_color(item == INS_ITEM_AUX_MIX);
		put_str(9, sy, mix_str);
	}
	else
	{
		ebt_item_color(item == INS_ITEM_NAME);
		put_str(1, sy, "SET NAME");

		sy += 2;

		ebt_item_color(item == INS_ITEM_LOAD);
		put_str(1, sy, "LOAD");

		++sy;

		ebt_item_color(item == INS_ITEM_SAVE);
		put_str(1, sy, "SAVE");

		sy += 2;

		if (ebt_ins_ptn_refs < 0) ebt_ins_ptn_refs = ebt_song_count_instrument_ptn_refs(cur_ins);
		if (ebt_ins_aux_refs < 0) ebt_ins_aux_refs = ebt_song_count_instrument_aux_refs(cur_ins);

		set_font_color(COL_TEXT_DARK);
		set_back_color(COL_BACK);

		snprintf(buf, sizeof(buf), "PTN REFS: %i", ebt_ins_ptn_refs);
		put_str(1, sy, buf);

		++sy;

		set_font_color(COL_TEXT_DARK);
		set_back_color(COL_BACK);

		snprintf(buf, sizeof(buf), "AUX REFS: %i", ebt_ins_aux_refs);
		put_str(1, sy, buf);

		++sy;
	}
}



void ebt_edit_instrument_load_cb_ok(void)
{
	char filename[MAX_PATH];

	strncpy(filename, ebt_get_filename(), sizeof(filename) - 1);
	strncat(filename, FILE_EXT_INSTRUMENT, sizeof(filename) - 1);

	set_edit_mode(EDIT_MODE_INSTRUMENT);

	if (ebt_instrument_load(cur_ins, filename))
	{
		osd_message_set("INS LOAD OK");
	}
	else
	{
		osd_message_set("INS LOAD FAIL");
	}
}



void ebt_edit_instrument_cb_cancel(void)
{
	set_edit_mode(EDIT_MODE_INSTRUMENT);
}



void ebt_edit_instrument_make_name(char* filename, unsigned int filename_size)
{
	strncpy(filename, ebt_get_name(), filename_size - 1);
	strncat(filename, FILE_EXT_INSTRUMENT, filename_size - 1);
}




void ebt_edit_instrument_save_cb_ok_2(void)
{
	char filename[MAX_PATH];

	ebt_edit_instrument_make_name(filename, sizeof(filename));

	set_edit_mode(EDIT_MODE_INSTRUMENT);

	if (ebt_instrument_save(cur_ins, filename))
	{
		osd_message_set("INS SAVE OK");
	}
	else
	{
		osd_message_set("INS SAVE FAIL");
	}
}



void ebt_edit_instrument_save_cb_ok(void)
{
	char filename[MAX_PATH];

	ebt_edit_instrument_make_name(filename, sizeof(filename));

	if (!ebt_file_is_exist(filename))
	{
		ebt_edit_instrument_save_cb_ok_2();
	}
	else
	{
		ebt_ask_confirm("OVERWRITE?", ebt_edit_instrument_save_cb_ok_2, ebt_edit_instrument_cb_cancel);
	}
}



void ebt_edit_instrument_load_sure_cb(void)
{
	ebt_ask_file("LOAD INS", FILE_EXT_INSTRUMENT, NULL, ebt_edit_instrument_load_cb_ok, ebt_edit_instrument_cb_cancel);
}



void ebt_edit_instrument_change(int step, BOOL relative)
{
	if (relative)
	{
		cur_ins += step;
	}
	else
	{
		cur_ins = step;
	}

	if (cur_ins < 1) cur_ins = MAX_INSTRUMENTS - 1;
	if (cur_ins >= MAX_INSTRUMENTS) cur_ins = 1;

	ebt_ins_ptn_refs = -1;
	ebt_ins_aux_refs = -1;
}



void ebt_edit_instrument_name_cb_ok(void)
{
	ebt_instrument_undo_set();

	const char* name = ebt_get_name();

	for (int i = 0; i < MAX_INSTRUMENT_NAME_LEN; ++i)
	{
		song->ins[cur_ins].name[i] = name[i];
	}

	set_edit_mode(EDIT_MODE_INSTRUMENT);
}



BOOL ebt_change_param_u8_ins(uint8_t* pparam_u8, uint8_t pad, uint8_t pad_t, uint8_t def, uint8_t min, uint8_t max, uint8_t large_step)
{
	return ebt_change_param_u8_undo(pparam_u8, pad, pad_t, def, min, max, large_step, ebt_instrument_undo_set);
}



BOOL ebt_change_param_note_ins(uint8_t* pparam_u8, uint8_t pad, uint8_t pad_t)
{
	return ebt_change_param_note_undo(pparam_u8, pad, pad_t, ebt_instrument_undo_set);
}



BOOL ebt_change_param_i8_ins(signed char* pparam_i8, uint8_t pad, uint8_t pad_t, int8_t def, int8_t min, int8_t max, int8_t large_step, uint8_t no_zero)
{
	return ebt_change_param_i8_undo(pparam_i8, pad, pad_t, def, min, max, large_step, no_zero, ebt_instrument_undo_set);
}



void ebt_edit_instrument_update(void)
{
	uint8_t kb = ebt_input_get_kb();

	if (kb == KB_UNDO) ebt_instrument_undo(); 
	
	int pad_t = ebt_input_get_trigger();
	int pad_r = ebt_input_get_repeat();
	int pad = ebt_input_get_state();

	if (!(pad&PAD_ACT))
	{
		if (!(pad&PAD_ESC))
		{
			if (pad_r&PAD_UP)
			{
				--ebt_ins_item[ebt_ins_page];

				if (ebt_ins_item[ebt_ins_page] < ebt_ins_item_min[ebt_ins_page]) ebt_ins_item[ebt_ins_page] = ebt_ins_item_max[ebt_ins_page];
			}

			if (pad_r&PAD_DOWN)
			{
				++ebt_ins_item[ebt_ins_page];

				if (ebt_ins_item[ebt_ins_page] > ebt_ins_item_max[ebt_ins_page]) ebt_ins_item[ebt_ins_page] = ebt_ins_item_min[ebt_ins_page];
			}

			if (pad_r & (PAD_LEFT | PAD_RIGHT))
			{
				ebt_ins_page ^= 1;
			}
		}
		else
		{
			if (pad_r&PAD_LEFT) ebt_edit_instrument_change(-1, TRUE);
			if (pad_r&PAD_RIGHT) ebt_edit_instrument_change(1, TRUE);
		}
	}

	instrument_struct* is = &song->ins[cur_ins];

	switch (ebt_ins_item[ebt_ins_page])
	{
	case INS_ITEM_WAVE: ebt_change_param_u8_ins(&is->wave, pad, pad_r, 0, 0, SYNTH_WAVEFORMS_MAX - 1, 16); break;
	case INS_ITEM_VOLUME: ebt_change_param_u8_ins(&is->volume, pad, pad_r, DEFAULT_VOLUME, 1, 4, 16); break;
	case INS_ITEM_OFFSET: ebt_change_param_i8_ins(&is->offset, pad, pad_r, 0, -127, 127, 1, FALSE); break;
	case INS_ITEM_DETUNE: ebt_change_param_i8_ins(&is->detune, pad, pad_r, 0, -127, 127, 16, FALSE); break;
	case INS_ITEM_SLIDE: ebt_change_param_i8_ins(&is->slide, pad, pad_r, 0, -127, 127, 16, FALSE); break;
	case INS_ITEM_VIBRATO_DELAY: ebt_change_param_u8_ins(&is->mod_delay, pad, pad_r, 0, 0, 255, 16); break;
	case INS_ITEM_VIBRATO_SPEED: ebt_change_param_u8_ins(&is->mod_speed, pad, pad_r, 0, 0, 255, 16); break;
	case INS_ITEM_VIBRATO_DEPTH: ebt_change_param_u8_ins(&is->mod_depth, pad, pad_r, 0, 0, 255, 16); break;
	case INS_ITEM_CUT_TIME: ebt_change_param_u8_ins(&is->cut_time, pad, pad_r, 0, 0, 255, 16); break;
	case INS_ITEM_FIXED_PITCH: ebt_change_param_u8_ins(&is->fixed_pitch, pad, pad_r, 0, 0, 1, 1); break;
	case INS_ITEM_BASE_NOTE: ebt_change_param_note_ins(&is->base_note, pad, pad_r); break;
	case INS_ITEM_AUX_ID:
		if (ebt_change_param_i8_ins(&is->aux_id, pad, pad_r, 0, -127, 127, 16, FALSE))
		{
			ebt_ins_ptn_refs = -1;
			ebt_ins_aux_refs = -1;
		}
		break;
	case INS_ITEM_AUX_MIX: ebt_change_param_u8_ins(&is->aux_mix, pad, pad_r, 0, 0, SYNTH_MIX_NONE - 1, 1); break;
	}

	if (pad_t&PAD_ACT)
	{
		switch (ebt_ins_item[ebt_ins_page])
		{
		case INS_ITEM_LOAD: ebt_ask_confirm("LOAD SURE?", ebt_edit_instrument_load_sure_cb, ebt_edit_instrument_cb_cancel); break;
		case INS_ITEM_SAVE: ebt_ask_name("SAVE INS", NULL, ebt_edit_instrument_save_cb_ok, ebt_edit_instrument_cb_cancel, 0, FALSE); break;
		case INS_ITEM_NAME:
			{
				char default_name[MAX_INSTRUMENT_NAME_LEN + 1];
				memset(default_name, 0, sizeof(default_name));
				memcpy(default_name, song->ins[cur_ins].name, MAX_INSTRUMENT_NAME_LEN);
				ebt_ask_name("INS NAME", default_name, ebt_edit_instrument_name_cb_ok, ebt_edit_instrument_cb_cancel, MAX_INSTRUMENT_NAME_LEN, TRUE);
			}
			break;
		}
	}

	if (ptn_menu_double_tap) --ptn_menu_double_tap;

	if (pad_t & PAD_ACT)
	{
		if (ptn_menu_double_tap > 0)
		{
			if (ebt_ins_item[ebt_ins_page] == INS_ITEM_AUX_ID)
			{
				if (song->ins[cur_ins].aux_id != 0)
				{
					ebt_edit_instrument_change(cur_ins + song->ins[cur_ins].aux_id, FALSE);
				}
			}
		}

		ptn_menu_double_tap = DOUBLE_CLICK_TIMEOUT;
	}
}