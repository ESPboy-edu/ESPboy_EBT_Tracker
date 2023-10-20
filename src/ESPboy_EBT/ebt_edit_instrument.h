enum {
	INS_ITEM_WAVE = 0,
	INS_ITEM_VOLUME,
	INS_ITEM_OCTAVE,
	INS_ITEM_DETUNE,
	INS_ITEM_SLIDE,
	INS_ITEM_VIBRATO_DELAY,
	INS_ITEM_VIBRATO_SPEED,
	INS_ITEM_VIBRATO_DEPTH,
	INS_ITEM_CUT_TIME,
	INS_ITEM_FIXED_PITCH,
	INS_ITEM_BASE_NOTE,
	INS_ITEM_LOAD,
	INS_ITEM_SAVE,
	INS_ITEMS_ALL
};

signed char ebt_ins_item = INS_ITEM_WAVE;



void ebt_edit_instrument_init(void)
{
}



void ebt_edit_instrument_draw(void)
{
	char buf[16];

	snprintf(buf, sizeof(buf), "INST%2.2X", cur_ins);

	put_header(buf, COL_HEAD_INS);

	const char* wave_names[SYNTH_WAVEFORMS_MAX] = {
		"P50%","P32%","P25%","P19%","P12%","P06%",
		"SWPF","SWPS","SWLO","SWHI",
		"DSS-","DSS+","DSF-",
		"VOW1","VOW2","VOW3","VOW4","VOW5","VOW6",
		"RSP1","RSP2",
		"PHT1","PHT2","PHT3","PHT4","PHT5","PHT6","PHT7",
		"NSE1","NSE2","NSE3","NSE4"
	};

	instrument_struct* is = &song->ins[cur_ins];

	set_font_color(COL_TEXT_DARK);
	set_back_color(COL_BACK);
	put_str(1, 2, "WAVE");

	snprintf(buf, sizeof(buf), "%2.2X", is->wave);

	ebt_item_color(ebt_ins_item == INS_ITEM_WAVE);
	put_str(9, 2, buf);

	set_back_color(COL_BACK);
	set_font_color(COL_TEXT_DARK);
	put_str(12, 2, wave_names[is->wave]);

	set_font_color(COL_TEXT_DARK);
	set_back_color(COL_BACK);
	put_str(1, 3, "VOLUME");

	snprintf(buf, sizeof(buf), "%u", is->volume);

	ebt_item_color(ebt_ins_item == INS_ITEM_VOLUME);
	put_str(9, 3, buf);

	set_font_color(COL_TEXT_DARK);
	set_back_color(COL_BACK);
	put_str(1, 4, "OCTAVE");

	snprintf(buf, sizeof(buf), "%2.2X", abs(is->octave));

	if (is->octave < 0) put_str(8, 4, "-");
	ebt_item_color(ebt_ins_item == INS_ITEM_OCTAVE);
	put_str(9, 4, buf);

	set_font_color(COL_TEXT_DARK);
	set_back_color(COL_BACK);
	put_str(1, 5, "DETUNE ");

	snprintf(buf, sizeof(buf), "%2.2X", abs(is->detune));

	if (is->detune < 0) put_str(8, 5, "-");
	ebt_item_color(ebt_ins_item == INS_ITEM_DETUNE);
	put_str(9, 5, buf);

	set_font_color(COL_TEXT_DARK);
	set_back_color(COL_BACK);
	put_str(1, 6, "SLIDE");

	snprintf(buf, sizeof(buf), "%2.2X", abs(is->slide));

	if (is->slide < 0) put_str(8, 6, "-");
	ebt_item_color(ebt_ins_item == INS_ITEM_SLIDE);
	put_str(9, 6, buf);

	set_font_color(COL_TEXT_DARK);
	set_back_color(COL_BACK);
	put_str(1, 7, "VIB.DL");

	snprintf(buf, sizeof(buf), "%2.2X", is->mod_delay);

	ebt_item_color(ebt_ins_item == INS_ITEM_VIBRATO_DELAY);
	put_str(9, 7, buf);

	set_font_color(COL_TEXT_DARK);
	set_back_color(COL_BACK);
	put_str(1, 8, "VIB.SP");

	snprintf(buf, sizeof(buf), "%2.2X", is->mod_speed);

	ebt_item_color(ebt_ins_item == INS_ITEM_VIBRATO_SPEED);
	put_str(9, 8, buf);

	set_font_color(COL_TEXT_DARK);
	set_back_color(COL_BACK);
	put_str(1, 9, "VIB.DP");

	snprintf(buf, sizeof(buf), "%2.2X", is->mod_depth);

	ebt_item_color(ebt_ins_item == INS_ITEM_VIBRATO_DEPTH);
	put_str(9, 9, buf);

	set_font_color(COL_TEXT_DARK);
	set_back_color(COL_BACK);
	put_str(1, 10, "CUT");

	if (is->cut_time > 0)
	{
		snprintf(buf, sizeof(buf), "%2.2X", is->cut_time);
	}
	else
	{
		strcpy(buf, "OFF");
	}

	ebt_item_color(ebt_ins_item == INS_ITEM_CUT_TIME);
	put_str(9, 10, buf);

	set_font_color(COL_TEXT_DARK);
	set_back_color(COL_BACK);
	put_str(1, 11, "FIX.PT");

	ebt_item_color(ebt_ins_item == INS_ITEM_FIXED_PITCH);
	put_str(9, 11, is->fixed_pitch ? "ON" : "OFF");

	set_font_color(COL_TEXT_DARK);
	set_back_color(COL_BACK);
	put_str(1, 12, "NOTE");

	ebt_item_color(ebt_ins_item == INS_ITEM_BASE_NOTE);

	snprintf(buf, sizeof(buf), "%s%i", note_names[is->base_note % 12], is->base_note / 12);
	put_str(9, 12, buf);

	ebt_item_color(ebt_ins_item == INS_ITEM_LOAD);
	put_str(1, 13, "LOAD");

	ebt_item_color(ebt_ins_item == INS_ITEM_SAVE);
	put_str(1, 14, "SAVE");
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



void ebt_edit_instrument_update(void)
{
	int pad_t = ebt_input_get_trigger();
	int pad_r = ebt_input_get_repeat();
	int pad = ebt_input_get_state();

	if (!(pad&PAD_ACT))
	{
		if (!(pad&PAD_ESC))
		{
			if (pad_r&PAD_UP)
			{
				--ebt_ins_item;

				if (ebt_ins_item < 0) ebt_ins_item = INS_ITEMS_ALL - 1;
			}

			if (pad_r&PAD_DOWN)
			{
				++ebt_ins_item;

				if (ebt_ins_item >= INS_ITEMS_ALL) ebt_ins_item = 0;
			}
		}
		else
		{
			if (pad_r&PAD_LEFT)
			{
				--cur_ins;

				if (cur_ins < 1) cur_ins = MAX_INSTRUMENTS - 1;
			}

			if (pad_r&PAD_RIGHT)
			{
				++cur_ins;

				if (cur_ins >= MAX_INSTRUMENTS) cur_ins = 1;
			}
		}
	}
	else
	{
		instrument_struct* is = &song->ins[cur_ins];

		switch (ebt_ins_item)
		{
		case INS_ITEM_WAVE: ebt_change_param_u8(&is->wave, pad_r, 0, 0, SYNTH_WAVEFORMS_MAX - 1, 16); break;
		case INS_ITEM_VOLUME: ebt_change_param_u8(&is->volume, pad_r, DEFAULT_VOLUME, 1, 4, 16); break;
		case INS_ITEM_OCTAVE: ebt_change_param_i8(&is->octave, pad_r, 0, -10, 10, 1, FALSE); break;
		case INS_ITEM_DETUNE: ebt_change_param_i8(&is->detune, pad_r, 0, -127, 127, 16, FALSE); break;
		case INS_ITEM_SLIDE: ebt_change_param_i8(&is->slide, pad_r, 0, -127, 127, 16, FALSE); break;
		case INS_ITEM_VIBRATO_DELAY: ebt_change_param_u8(&is->mod_delay, pad_r, 0, 0, 255, 16); break;
		case INS_ITEM_VIBRATO_SPEED: ebt_change_param_u8(&is->mod_speed, pad_r, 0, 0, 255, 16); break;
		case INS_ITEM_VIBRATO_DEPTH: ebt_change_param_u8(&is->mod_depth, pad_r, 0, 0, 255, 16); break;
		case INS_ITEM_CUT_TIME: ebt_change_param_u8(&is->cut_time, pad_r, 0, 0, 255, 16); break;
		case INS_ITEM_FIXED_PITCH: ebt_change_param_u8(&is->fixed_pitch, pad_r, 0, 0, 1, 1); break;
		case INS_ITEM_BASE_NOTE: ebt_change_param_u8(&is->base_note, pad_r, DEFAULT_BASE_NOTE, 12, 12 * 10 - 1, 12); break;
		}
	}

	if (pad_t&PAD_ACT)
	{
		switch (ebt_ins_item)
		{
		case INS_ITEM_LOAD: ebt_ask_confirm("LOAD SURE?", ebt_edit_instrument_load_sure_cb, ebt_edit_instrument_cb_cancel); break;
		case INS_ITEM_SAVE: ebt_ask_name("SAVE INS", NULL, ebt_edit_instrument_save_cb_ok, ebt_edit_instrument_cb_cancel); break;
		}
	}
}