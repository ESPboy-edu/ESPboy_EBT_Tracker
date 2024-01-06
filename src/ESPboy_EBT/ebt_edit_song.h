enum {
  SONG_ITEM_NAME = 0,
  SONG_ITEM_AUTHOR,
  SONG_ITEM_LOAD,
  SONG_ITEM_SAVE,
  SONG_ITEM_SPEED_EVEN,
  SONG_ITEM_SPEED_ODD,
  SONG_ITEM_SPEED_INTERLEAVE,
  SONG_ITEM_CLEAR_SONG,
  SONG_ITEM_CLEAR_ALL,
  SONG_ITEM_SQUEEZE,
  SONG_ITEM_EXPORT,
  SONG_ITEM_PAN_CH1,
  SONG_ITEM_PAN_CH2,
  SONG_ITEM_PAN_CH3,
  SONG_ITEM_PAN_CH4
};

#define SONG_ITEM_PAGE_1_FROM	SONG_ITEM_NAME
#define SONG_ITEM_PAGE_1_TO		SONG_ITEM_CLEAR_ALL
#define SONG_ITEM_PAGE_2_FROM	SONG_ITEM_SQUEEZE
#define SONG_ITEM_PAGE_2_TO		SONG_ITEM_PAN_CH4

int8_t ebt_song_item[2] = { SONG_ITEM_PAGE_1_FROM,SONG_ITEM_PAGE_2_FROM };

const int8_t ebt_song_item_min[2] = { SONG_ITEM_PAGE_1_FROM,SONG_ITEM_PAGE_2_FROM };
const int8_t ebt_song_item_max[2] = { SONG_ITEM_PAGE_1_TO,SONG_ITEM_PAGE_2_TO };

uint8_t ebt_song_page = 0;

char ebt_song_last_name[MAX_PATH] = { 0 };	//remember loaded and saved song filenames to keep it in the save filename dialog



const char* ebt_song_get_last_name(void)
{
  return ebt_song_last_name;
}



void ebt_song_set_last_name(const char* str)
{
  strncpy(ebt_song_last_name, str, sizeof(ebt_song_last_name) - 1);

  for (unsigned int i = 0; i < strlen(ebt_song_last_name); ++i)
  {
    if (ebt_song_last_name[i] < 0x20)
    {
      ebt_song_last_name[i] = 0;
      break;
    }
  }
}



void ebt_edit_song_init(void)
{
}



void ebt_edit_song_draw(void)
{
  char buf[8];

  ebt_put_header("SONG", COL_HEAD_SONG);

  set_font_color(COL_TEXT_DARK);
  set_back_color(COL_BACK);

  snprintf(buf, sizeof(buf), "%i/2", ebt_song_page + 1);

  put_str((signed char)(Text.width - strlen(buf)), 0, buf);

  int sx = 1;
  int sy = 2;

  uint8_t item = ebt_song_item[ebt_song_page];

  if (ebt_song_page == 0)
  {
    ebt_item_color(item == SONG_ITEM_NAME);

    put_str(sx, sy, "NAME");

    set_font_color(COL_TEXT_DARK);
    set_back_color(COL_BACK);
    put_str(sx + 4, sy, ":");
    put_str(sx + 5, sy, song->name);

    ++sy;

    ebt_item_color(item == SONG_ITEM_AUTHOR);

    put_str(sx, sy, "AUTH");

    set_font_color(COL_TEXT_DARK);
    set_back_color(COL_BACK);
    put_str(sx + 4, sy, ":");
    put_str(sx + 5, sy, song->author);

    sy += 2;

    ebt_item_color(item == SONG_ITEM_LOAD);

    put_str(sx, sy, "LOAD");

    ++sy;

    ebt_item_color(item == SONG_ITEM_SAVE);

    put_str(sx, sy, "SAVE");

    sy += 2;

    snprintf(buf, sizeof(buf), "%2.2X", song->speed_even);

    set_font_color(COL_TEXT_DARK);
    set_back_color(COL_BACK);
    put_str(sx, sy, "SP.EVEN");

    ebt_item_color(item == SONG_ITEM_SPEED_EVEN);
    put_str(sx + 8, sy, buf);

    ++sy;

    snprintf(buf, sizeof(buf), "%2.2X", song->speed_odd);

    set_font_color(COL_TEXT_DARK);
    set_back_color(COL_BACK);
    put_str(sx, sy, "SP.ODD");

    ebt_item_color(item == SONG_ITEM_SPEED_ODD);
    put_str(sx + 8, sy, buf);

    ++sy;

    snprintf(buf, sizeof(buf), "%2.2X", song->speed_interleave);

    set_font_color(COL_TEXT_DARK);
    set_back_color(COL_BACK);
    put_str(sx, sy, "SP.INTR");

    ebt_item_color(item == SONG_ITEM_SPEED_INTERLEAVE);
    put_str(sx + 8, sy, buf);

    sy += 2;

    ebt_item_color(item == SONG_ITEM_CLEAR_SONG);

    put_str(sx, sy, "CLR SONG");

    ++sy;

    ebt_item_color(item == SONG_ITEM_CLEAR_ALL);

    put_str(sx, sy, "CLR ALL");

    ++sy;
  }
  else
  {
    ebt_item_color(item == SONG_ITEM_SQUEEZE);

    put_str(sx, sy, "SQUEEZE");

    sy += 2;

    ebt_item_color(item == SONG_ITEM_EXPORT);

    put_str(sx, sy, "EXPORT");

    sy += 2;

    for (int ch = 0; ch < MAX_CHANNELS; ++ch)
    {
      set_font_color(COL_TEXT_DARK);
      set_back_color(COL_BACK);

      snprintf(buf, sizeof(buf), "%sCH%i", (ch == 0) ? "PAN." : "    ", ch + 1);
      put_str(sx, sy, buf);

      ebt_item_color(item == (SONG_ITEM_PAN_CH1 + ch));

      snprintf(buf, sizeof(buf), "%i", abs(song->pan_default[ch]));
      put_str(sx + 8, sy, buf);
      if (song->pan_default[ch] < 0) put_str(sx + 7, sy, "-");

      ++sy;
    }
  }
}



void ebt_edit_song_clear_song_cb(void)
{
	ebt_song_backup();
  ebt_song_clear(false);
  ebt_song_set_last_name("");
  ebt_config_save(TRUE);
 
  osd_message_set("SONG CLEARED");
}



void ebt_edit_song_clear_all_cb(void)
{
	ebt_song_backup();
  ebt_song_clear(TRUE);
  ebt_song_set_last_name("");
  ebt_config_save(TRUE);
 
  osd_message_set("ALL CLEARED");
}



void ebt_edit_song_squeeze_cb(void)
{
  ebt_song_squeeze();

  osd_message_set("SQUEEZED");
}



void ebt_edit_song_load_cb_ok(void)
{
  char filename[MAX_PATH];

  strncpy(ebt_song_last_name, ebt_get_filename(), sizeof(ebt_song_last_name) - 1);
  strncpy(filename, ebt_get_filename(), sizeof(filename) - 1);
  strncat(filename, FILE_EXT_SONG, sizeof(filename) - 1);

  set_edit_mode(EDIT_MODE_SONG);

  ebt_song_backup();
  
  if (ebt_song_load(filename))
  {
    ebt_config_save(TRUE);
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



void ebt_edit_song_make_name(char* filename, const char* ext, unsigned int filename_size)
{
  strncpy(ebt_song_last_name, ebt_get_name(), sizeof(ebt_song_last_name) - 1);
  strncpy(filename, ebt_get_name(), filename_size - 1);
  strncat(filename, ext, filename_size - 1);
}



void ebt_edit_song_save_cb_ok_2(void)
{
  char filename[MAX_PATH];

  ebt_edit_song_make_name(filename, FILE_EXT_SONG, sizeof(filename));

  set_edit_mode(EDIT_MODE_SONG);

  ebt_song_rename(filename);

  if (ebt_song_save(filename))
  {
    ebt_config_save(TRUE);
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

  ebt_edit_song_make_name(filename, FILE_EXT_SONG, sizeof(filename));

  if (!ebt_file_is_exist(filename))
  {
    ebt_edit_song_save_cb_ok_2();
  }
  else
  {
    ebt_ask_confirm("OVERWRITE?", ebt_edit_song_save_cb_ok_2, ebt_edit_song_cb_cancel);
  }
}



void ebt_edit_song_export_cb_ok_2(void)
{
  char filename[MAX_PATH];

  ebt_edit_song_make_name(filename, FILE_EXT_HEADER, sizeof(filename));

  set_edit_mode(EDIT_MODE_SONG);

  if (ebt_song_export(filename))
  {
    osd_message_set("EXPORT OK");
  }
  else
  {
    osd_message_set("EXPORT FAIL");
  }
}



void ebt_edit_song_export_cb_ok(void)
{
  char filename[MAX_PATH];

  ebt_edit_song_make_name(filename, FILE_EXT_HEADER, sizeof(filename));

  if (!ebt_file_is_exist(filename))
  {
    ebt_edit_song_export_cb_ok_2();
  }
  else
  {
    ebt_ask_confirm("OVERWRITE?", ebt_edit_song_export_cb_ok_2, ebt_edit_song_cb_cancel);
  }
}



void ebt_edit_song_load_sure_cb(void)
{
  ebt_ask_file("LOAD SONG", FILE_EXT_SONG, ebt_song_last_name, ebt_edit_song_load_cb_ok, ebt_edit_song_cb_cancel);
}



void ebt_edit_song_name_cb_ok(void)
{
  memset(song->name, 0, sizeof(song->name));
  strncpy(song->name, ebt_get_name(), sizeof(song->name) - 1);
  set_edit_mode(EDIT_MODE_SONG);
}



void ebt_edit_song_author_cb_ok(void)
{
  memset(song->author, 0, sizeof(song->author));
  strncpy(song->author, ebt_get_name(), sizeof(song->author) - 1);
  set_edit_mode(EDIT_MODE_SONG);
}



void ebt_edit_song_update(void)
{
  int pad_r = ebt_input_get_repeat();
  int pad_t = ebt_input_get_trigger();
  int pad = ebt_input_get_state();

  if (!(pad & PAD_ACT))
  {
    if (pad_r & PAD_UP)
    {
      --ebt_song_item[ebt_song_page];

	  if (ebt_song_item[ebt_song_page] < ebt_song_item_min[ebt_song_page]) ebt_song_item[ebt_song_page] = ebt_song_item_max[ebt_song_page];
    }

    if (pad_r & PAD_DOWN)
    {
      ++ebt_song_item[ebt_song_page];

	  if (ebt_song_item[ebt_song_page] > ebt_song_item_max[ebt_song_page]) ebt_song_item[ebt_song_page] = ebt_song_item_min[ebt_song_page];
    }

    if (pad_r & (PAD_LEFT | PAD_RIGHT))
    {
		ebt_song_page ^= 1;
    }
  }
  else
  {
    if (pad_t & PAD_ACT)
    {
      switch (ebt_song_item[ebt_song_page])
      {
        case SONG_ITEM_NAME:
          ebt_player_stop();
		  ebt_ask_name("SONG NAME", song->name, ebt_edit_song_name_cb_ok, ebt_edit_song_cb_cancel, MAX_SONG_INFO_LEN, TRUE);
          break;

        case SONG_ITEM_AUTHOR:
          ebt_player_stop();
		  ebt_ask_name("SONG AUTHOR", song->author, ebt_edit_song_author_cb_ok, ebt_edit_song_cb_cancel, MAX_SONG_INFO_LEN, TRUE);
          break;

        case SONG_ITEM_LOAD:
          ebt_player_stop();
          ebt_ask_confirm("LOAD SURE?", ebt_edit_song_load_sure_cb, ebt_edit_song_cb_cancel);
          break;

        case SONG_ITEM_SAVE:
          ebt_player_stop();
		  ebt_ask_name("SAVE SONG", ebt_song_last_name, ebt_edit_song_save_cb_ok, ebt_edit_song_cb_cancel, 0, FALSE);
          break;

        case SONG_ITEM_CLEAR_SONG:
          ebt_player_stop();
          ebt_ask_confirm("CLEAR SONG?", ebt_edit_song_clear_song_cb, ebt_edit_song_cb_cancel);
          break;

        case SONG_ITEM_CLEAR_ALL:
          ebt_player_stop();
          ebt_ask_confirm("CLEAR ALL?", ebt_edit_song_clear_all_cb, ebt_edit_song_cb_cancel);
          break;

        case SONG_ITEM_SQUEEZE:
          ebt_player_stop();
          ebt_ask_confirm("SURE?", ebt_edit_song_squeeze_cb, ebt_edit_song_cb_cancel);
          break;

        case SONG_ITEM_EXPORT:
          ebt_player_stop();
		  ebt_ask_name("EXPORT SONG", ebt_song_last_name, ebt_edit_song_export_cb_ok, ebt_edit_song_cb_cancel, 0, FALSE);
          break;
      }
    }
  }

  switch (ebt_song_item[ebt_song_page])
  {
    case SONG_ITEM_SPEED_EVEN: ebt_change_param_u8(&song->speed_even, pad, pad_r, DEFAULT_SONG_SPEED, 1, 255, 16); break;
    case SONG_ITEM_SPEED_ODD: ebt_change_param_u8(&song->speed_odd, pad, pad_r, DEFAULT_SONG_SPEED, 1, 255, 16); break;
    case SONG_ITEM_SPEED_INTERLEAVE: ebt_change_param_u8(&song->speed_interleave, pad, pad_r, 1, 1, 255, 16); break;
    case SONG_ITEM_PAN_CH1: ebt_change_param_i8(&song->pan_default[0], pad, pad_r, 0, -4, 4, 1, FALSE); break;
    case SONG_ITEM_PAN_CH2: ebt_change_param_i8(&song->pan_default[1], pad, pad_r, 0, -4, 4, 1, FALSE); break;
    case SONG_ITEM_PAN_CH3: ebt_change_param_i8(&song->pan_default[2], pad, pad_r, 0, -4, 4, 1, FALSE); break;
    case SONG_ITEM_PAN_CH4: ebt_change_param_i8(&song->pan_default[3], pad, pad_r, 0, -4, 4, 1, FALSE); break;
  }
}
