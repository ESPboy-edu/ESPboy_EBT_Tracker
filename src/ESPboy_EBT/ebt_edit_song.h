enum {
	SONG_ITEM_PLAY = 0,
	SONG_ITEM_LOAD,
	SONG_ITEM_SAVE,
	SONG_ITEM_SPEED_EVEN,
	SONG_ITEM_SPEED_ODD,
	SONG_ITEM_SPEED_INTERLEAVE,
	SONG_ITEM_CLEAR_SONG,
	SONG_ITEM_CLEAR_ALL,
	SONG_ITEMS_ALL
};

signed char cur_song = SONG_ITEM_PLAY;

char song_name[MAX_PATH] = { 0 };	//remember loaded and saved songs to keep in the save filename dialog



void ebt_edit_song_init(void)
{
}



void ebt_edit_song_draw(void)
{
	char buf[8];

	put_header("SONG", COL_HEAD_SONG);

	ebt_item_color(cur_song == SONG_ITEM_PLAY);

	put_str(2, 2, "PLAY");

	ebt_item_color(cur_song == SONG_ITEM_LOAD);

	put_str(2, 4, "LOAD");

	ebt_item_color(cur_song == SONG_ITEM_SAVE);

	put_str(2, 5, "SAVE");

	snprintf(buf, sizeof(buf), "%2.2X", song->speed_even);

	set_font_color(COL_TEXT_DARK);
	set_back_color(COL_BACK);
	put_str(2, 7, "SP.EVEN");

	ebt_item_color(cur_song == SONG_ITEM_SPEED_EVEN);
	put_str(10, 7, buf);

	snprintf(buf, sizeof(buf), "%2.2X", song->speed_odd);

	set_font_color(COL_TEXT_DARK);
	set_back_color(COL_BACK);
	put_str(2, 8, "SP.ODD");

	ebt_item_color(cur_song == SONG_ITEM_SPEED_ODD);
	put_str(10, 8, buf);

	snprintf(buf, sizeof(buf), "%2.2X", song->speed_interleave);

	set_font_color(COL_TEXT_DARK);
	set_back_color(COL_BACK);
	put_str(2, 9, "SP.INTR");

	ebt_item_color(cur_song == SONG_ITEM_SPEED_INTERLEAVE);
	put_str(10, 9, buf);

	ebt_item_color(cur_song == SONG_ITEM_CLEAR_SONG);

	put_str(2, 11, "CLR SONG");

	ebt_item_color(cur_song == SONG_ITEM_CLEAR_ALL);

	put_str(2, 12, "CLR ALL");
}



void ebt_edit_song_clear_song_cb(void)
{
	song_clear(false);

	osd_message_set("SONG CLEARED");
}



void ebt_edit_song_clear_all_cb(void)
{
	song_clear(TRUE);

	osd_message_set("ALL CLEARED");
}



void ebt_edit_song_load_cb_ok(void)
{
	char filename[MAX_PATH];

	strncpy(song_name, ebt_get_filename(), sizeof(song_name) - 1);
	strncpy(filename, ebt_get_filename(), sizeof(filename) - 1);
	strncat(filename, FILE_EXT_SONG, sizeof(filename) - 1);

	set_edit_mode(EDIT_MODE_SONG);

	if (ebt_song_load(filename))
	{
		osd_message_set("SONG LOAD OK");
	}
	else
	{
		osd_message_set("SONG LOAD FAIL");
	}
}



void ebt_edit_song_cb_cancel(void)
{
	set_edit_mode(EDIT_MODE_SONG);
}



void ebt_edit_song_make_name(char* filename, unsigned int filename_size)
{
	strncpy(song_name, ebt_get_name(), sizeof(song_name) - 1);
	strncpy(filename, ebt_get_name(), filename_size - 1);
	strncat(filename, FILE_EXT_SONG, filename_size - 1);
}



void ebt_edit_song_save_cb_ok_2(void)
{
	char filename[MAX_PATH];

	ebt_edit_song_make_name(filename, sizeof(filename));

	set_edit_mode(EDIT_MODE_SONG);

	if (ebt_song_save(filename))
	{
		osd_message_set("SONG SAVE OK");
	}
	else
	{
		osd_message_set("SONG SAVE FAIL");
	}
}



void ebt_edit_song_save_cb_ok(void)
{
	char filename[MAX_PATH];

	ebt_edit_song_make_name(filename, sizeof(filename));

	if (!ebt_file_is_exist(filename))
	{
		ebt_edit_song_save_cb_ok_2();
	}
	else
	{
		ebt_ask_confirm("OVERWRITE?", ebt_edit_song_save_cb_ok_2, ebt_edit_song_cb_cancel);
	}
}



void ebt_edit_song_load_sure_cb(void)
{
	ebt_ask_file("LOAD SONG", FILE_EXT_SONG, song_name, ebt_edit_song_load_cb_ok, ebt_edit_song_cb_cancel);
}



void ebt_edit_song_update(void)
{
	int pad_r = ebt_input_get_repeat();
	int pad_t = ebt_input_get_trigger();
	int pad = ebt_input_get_state();

	if (!(pad&PAD_ACT))
	{
		if (pad_r&PAD_UP)
		{
			--cur_song;

			if (cur_song < 0) cur_song = SONG_ITEMS_ALL - 1;
		}

		if (pad_r&PAD_DOWN)
		{
			++cur_song;

			if (cur_song >= SONG_ITEMS_ALL) cur_song = 0;
		}
	}
	else
	{
		if (pad_t&PAD_ACT)
		{
			switch (cur_song)
			{
			case SONG_ITEM_PLAY:
				ebt_toggle_song_play();
				break;

			case SONG_ITEM_LOAD:
				ebt_player_stop();
				ebt_ask_confirm("LOAD SURE?", ebt_edit_song_load_sure_cb, ebt_edit_song_cb_cancel);
				break;

			case SONG_ITEM_SAVE:
				ebt_player_stop();
				ebt_ask_name("SAVE SONG", song_name, ebt_edit_song_save_cb_ok, ebt_edit_song_cb_cancel);
				break;

			case SONG_ITEM_CLEAR_SONG:
				ebt_player_stop();
				ebt_ask_confirm("CLEAR SONG?", ebt_edit_song_clear_song_cb, ebt_edit_song_cb_cancel);
				break;

			case SONG_ITEM_CLEAR_ALL:
				ebt_player_stop();
				ebt_ask_confirm("CLEAR ALL?", ebt_edit_song_clear_all_cb, ebt_edit_song_cb_cancel);
				break;
			}
		}

		switch (cur_song)
		{
		case SONG_ITEM_SPEED_EVEN: ebt_change_param_u8(&song->speed_even, pad_r, DEFAULT_SONG_SPEED, 1, 255, 16); break;
		case SONG_ITEM_SPEED_ODD: ebt_change_param_u8(&song->speed_odd, pad_r, DEFAULT_SONG_SPEED, 1, 255, 16); break;
		case SONG_ITEM_SPEED_INTERLEAVE: ebt_change_param_u8(&song->speed_interleave, pad_r, 1, 1, 255, 16); break;
		}
	}
}
