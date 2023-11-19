short int cur_file = 0;
short int cur_files_all = 0;

#define FILE_LIST_HGT		(MAX_TEXT_HGT-3)
#define MAX_FILENAME_LEN	32

char ebt_file_list_buf[FILE_LIST_HGT][MAX_FILENAME_LEN];

const char* ebt_file_dlg_name = NULL;

void(*ebt_file_dialog_callback_ok)(void);
void(*ebt_file_dialog_callback_cancel)(void);

char ebt_file_filter[5] = { 0 };

char ebt_file_name[MAX_FILENAME_LEN] = { 0 };



void ebt_ask_file(const char* dlg_name, const char* ext, const char* cur_name,void(*cb_ok)(void), void(*cb_cancel)(void))
{
  ebt_file_dlg_name = dlg_name;

  memset(ebt_file_filter, 0, sizeof(ebt_file_filter));
  memcpy(ebt_file_filter, ext, sizeof(ebt_file_filter));
  memset(ebt_file_name, 0, sizeof(ebt_file_name));

  ebt_file_dialog_callback_ok = cb_ok;
  ebt_file_dialog_callback_cancel = cb_cancel;

  if (cur_name) strncpy(ebt_file_name, cur_name, sizeof(ebt_file_name));

  set_edit_mode(EDIT_MODE_FILE_BROWSER);
}



const char* ebt_get_filename(void)
{
  return ebt_file_name;
}



int ebt_file_browser_from_file(void)
{
  int from_file = cur_file - FILE_LIST_HGT / 2;
  if (from_file >= (cur_files_all - FILE_LIST_HGT)) from_file = (cur_files_all - FILE_LIST_HGT);
  if (from_file < 0) from_file = 0;

  return from_file;
}



#ifdef TARGET_SDL

void ebt_file_browser_update_list(int from_file)
{
  DIR* dp = opendir(dataDirectory);

  for (int i = 0; i < (Text.height - 3); ++i) ebt_file_list_buf[i][0] = 0;

  if (dp)
  {
    rewinddir(dp);

    int file_cnt = 0;

    cur_files_all = 0;

    while (1)
    {
      struct dirent* dirp = readdir(dp);

      if (!dirp) break;

      if (dirp->d_type == DT_REG)
      {
        if ((strcmp(dirp->d_name, ".") != 0) && (strcmp(dirp->d_name, "..") != 0))
        {
          //split filename into name and extension

          char name[MAX_FILENAME_LEN];
          char ext[5];

          int name_ptr = 0;
          int ext_ptr = 0;
          int dot = FALSE;

          for (unsigned int i = 0; i < strlen(dirp->d_name); ++i)
          {
            char c = dirp->d_name[i];

            if (c == '.') dot = TRUE;

            if (!dot)
            {
              if (name_ptr < (sizeof(name) - 1)) name[name_ptr++] = c;
            }
            else
            {
              if (ext_ptr < (sizeof(ext) - 1)) ext[ext_ptr++] = c;
            }
          }

          name[name_ptr] = 0;
          ext[ext_ptr] = 0;

          //check if it matches the filter and add into the list

          if (strcmp(ext, ebt_file_filter) == 0)
          {
            if ((cur_files_all >= from_file) && (file_cnt < FILE_LIST_HGT))
            {
              strncpy(ebt_file_list_buf[file_cnt], name, sizeof(ebt_file_list_buf[file_cnt]) - 1);

			  if (strncmp(ebt_file_name, name, sizeof(ebt_file_name)) == 0)
			  {
				  cur_file = cur_files_all;
				  ebt_file_name[0] = 0;
			  }

              ++file_cnt;
            }

            ++cur_files_all;
          }
        }
      }
    }

    closedir(dp);
  }
}

#endif
#ifdef TARGET_ESPBOY

#define TEMP_FILE_CACHE_NAME  "files.txt"

void ebt_file_browser_cache_file_list(void)
{
  sound_output_shut();
  
  fs::File file = LittleFS.open(TEMP_FILE_CACHE_NAME, "w");

  fs::Dir dir = LittleFS.openDir("/");

  while (dir.next())
  {
    fs::File entry = dir.openFile("r");

    if (!entry) break;

    if (entry.isFile())
    {
      file.println(entry.name());
    }

    entry.close();
  }

  file.close();

  sound_output_init();
}

void ebt_file_browser_update_list(int from_file)
{
  sound_output_shut();

  fs::File file = LittleFS.open(TEMP_FILE_CACHE_NAME, "r");

  for (int i = 0; i < (Text.height - 3); ++i) ebt_file_list_buf[i][0] = 0;

  int file_cnt = 0;

  cur_files_all = 0;

  while (file.available())
  {
    String entry = file.readStringUntil('\n');

    if (entry.isEmpty()) break;

    const char* entry_name = entry.c_str();

    //split filename into name and extension

    char name[MAX_FILENAME_LEN];
    char ext[5];

    int name_ptr = 0;
    int ext_ptr = 0;
    int dot = FALSE;

    for (unsigned int i = 0; i < strlen(entry_name); ++i)
    {
      char c = entry_name[i];

      if (c == '.') dot = TRUE;

      if (!dot)
      {
        if (name_ptr < (sizeof(name) - 1)) name[name_ptr++] = c;
      }
      else
      {
        if (ext_ptr < (sizeof(ext) - 1)) ext[ext_ptr++] = c;
      }
    }

    name[name_ptr] = 0;
    ext[ext_ptr] = 0;

    //check if it matches the filter and add into the list

    if (strcmp(ext, ebt_file_filter) == 0)
    {
      if ((cur_files_all >= from_file) && (file_cnt < FILE_LIST_HGT))
      {
        strncpy(ebt_file_list_buf[file_cnt], name, sizeof(ebt_file_list_buf[file_cnt]) - 1);

		if (strncmp(ebt_file_name, name, sizeof(ebt_file_name)) == 0)
		{
			cur_file = cur_files_all;
			ebt_file_name[0] = 0;
		}

        ++file_cnt;
      }

      ++cur_files_all;
    }
  }

  file.close();

  sound_output_init();
}

#endif



void ebt_file_browser_init(void)
{
  cur_file = 0;

#ifdef TARGET_ESPBOY
  ebt_file_browser_cache_file_list();
#endif

  ebt_file_browser_update_list(ebt_file_browser_from_file());
  ebt_file_browser_update_list(ebt_file_browser_from_file()); //intentional
}



void ebt_file_browser_draw(void)
{
  char buf[16];

  ebt_put_header("FILE", COL_HEAD_FILE);

  if (ebt_file_dlg_name)
  {
    set_font_color(COL_TEXT_DARK);
    set_back_color(COL_BACK);
	put_str(Text.width - (signed char)strlen(ebt_file_dlg_name), 0, ebt_file_dlg_name);
  }

  int from_file = ebt_file_browser_from_file();

  set_font_color(COL_TEXT_DARK);
  set_back_color(COL_BACK);

  if (from_file > 0)
  {
    put_char(0, 2, 0x10);
  }

  if (from_file < (cur_files_all - FILE_LIST_HGT))
  {
    put_char(0, 2 + FILE_LIST_HGT - 1, 0x11);
  }

  if (cur_files_all > 0)
  {
    snprintf(buf, sizeof(buf), "%i/%i", (cur_file + 1), cur_files_all);

    put_str((signed char)(Text.width - strlen(buf)), Text.height - 1, buf);
  }

  for (int i = 0; i < (Text.height - 3); ++i)
  {
    ebt_item_color(from_file == cur_file);

    put_str(1, 2 + i, ebt_file_list_buf[i]);

    ++from_file;
  }

  if (cur_files_all == 0)
  {
    set_font_color(COL_TEXT_DARK);
    set_back_color(COL_BACK);

    put_str(4, Text.height / 2, "NO FILES");
  }
}



void ebt_file_browser_update(void)
{
  if (cur_file >= cur_files_all) cur_file = cur_files_all - 1;
  if (cur_file < 0) cur_file = 0;

  int pad_r = ebt_input_get_repeat();

  int add = 0;

  if (pad_r & PAD_UP) add = -1;
  if (pad_r & PAD_DOWN) add = 1;
  if (pad_r & PAD_LEFT) add = 8;
  if (pad_r & PAD_RIGHT) add = 8;

  if (add != 0)
  {
    cur_file += add;

    if (cur_file >= cur_files_all) cur_file = 0;
    if (cur_file < 0) cur_file = cur_files_all - 1;

    ebt_file_browser_update_list(ebt_file_browser_from_file());
  }

  if (pad_r & PAD_ACT)
  {
    if (cur_files_all > 0)
    {
      int file_from = ebt_file_browser_from_file();
      ebt_file_browser_update_list(file_from);
      strncpy(ebt_file_name, ebt_file_list_buf[cur_file - file_from], sizeof(ebt_file_name) - 1);

      if (ebt_file_dialog_callback_ok) ebt_file_dialog_callback_ok();
    }
  }

  if (pad_r & PAD_ESC)
  {
    memset(ebt_file_name, 0, sizeof(ebt_file_name));

    if (ebt_file_dialog_callback_cancel) ebt_file_dialog_callback_cancel();
  }
}
