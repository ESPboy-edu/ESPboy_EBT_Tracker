short int cur_file = 0;
short int cur_files_all = 0;

#define FILE_LIST_HGT		(MAX_TEXT_HGT-3)

char ebt_file_list_names[FILE_LIST_HGT][MAX_FILENAME_LEN];
uint32_t ebt_file_list_sizes[FILE_LIST_HGT];

const char* ebt_file_dlg_name = NULL;

void(*ebt_file_dlg_callback_ok)(void);
void(*ebt_file_dlg_callback_cancel)(void);

char ebt_file_filter[5] = { 0 };

char ebt_file_name[MAX_FILENAME_LEN] = { 0 };

uint16_t ebt_file_dlg_used_k = 0;
uint16_t ebt_file_dlg_all_k = 0;


enum {
	FILE_DLG_BROWSER,
	FILE_DLG_MANAGER
};

uint8_t ebt_file_dlg_mode = FILE_DLG_BROWSER;



int ebt_file_list_hgt(void)
{
	return Text.height - 3;
}



void ebt_file_browser_cat_read(void);
const char* ebt_get_filename(void);

void ebt_file_manager_cb_ok_2(void)
{
	ebt_file_delete(ebt_get_filename());

	osd_message_set("FILE DELETED");

	ebt_file_browser_cat_read();

	set_edit_mode(EDIT_MODE_FILE_BROWSER);
}



void ebt_file_manager_cb_cancel_2(void)
{
	set_edit_mode(EDIT_MODE_FILE_BROWSER);
}



void ebt_file_manager_cb_ok(void)
{
	ebt_ask_confirm("DELETE?", ebt_file_manager_cb_ok_2, ebt_file_manager_cb_cancel_2);
}



void ebt_file_manager_cb_cancel(void)
{
	set_edit_mode(EDIT_MODE_CONFIG);
}



void ebt_file_manager(void)
{
	ebt_file_dlg_mode = FILE_DLG_MANAGER;

	ebt_file_dlg_name = NULL;
	memset(ebt_file_filter, 0, sizeof(ebt_file_filter));
	memset(ebt_file_name, 0, sizeof(ebt_file_name));

	ebt_file_dlg_callback_ok = ebt_file_manager_cb_ok;
	ebt_file_dlg_callback_cancel = ebt_file_manager_cb_cancel;

	set_edit_mode(EDIT_MODE_FILE_BROWSER);
}



void ebt_ask_file(const char* dlg_name, const char* ext, const char* cur_name, void(*cb_ok)(void), void(*cb_cancel)(void))
{
	ebt_file_dlg_mode = FILE_DLG_BROWSER;
	ebt_file_dlg_name = dlg_name;

	memset(ebt_file_filter, 0, sizeof(ebt_file_filter));
	memcpy(ebt_file_filter, ext, sizeof(ebt_file_filter));
	memset(ebt_file_name, 0, sizeof(ebt_file_name));

	ebt_file_dlg_callback_ok = cb_ok;
	ebt_file_dlg_callback_cancel = cb_cancel;

	if (cur_name) strncpy(ebt_file_name, cur_name, sizeof(ebt_file_name));

	set_edit_mode(EDIT_MODE_FILE_BROWSER);
}



const char* ebt_get_filename(void)
{
	return ebt_file_name;
}



int ebt_file_browser_from_file(void)
{
	int from_file = cur_file - ebt_file_list_hgt() / 2;
	if (from_file > (cur_files_all - ebt_file_list_hgt())) from_file = (cur_files_all - ebt_file_list_hgt());
	if (from_file < 0) from_file = 0;

	return from_file;
}



#ifdef TARGET_SDL

void ebt_file_browser_update_list(int from_file)
{
	DIR* dp = opendir(dataDirectory);

	for (int i = 0; i < ebt_file_list_hgt(); ++i)
	{
		ebt_file_list_names[i][0] = 0;
		ebt_file_list_sizes[i] = 0;
	}

	ebt_file_dlg_used_k = 0;
	ebt_file_dlg_all_k = 9999;

	if (!dp)
	{
		log_add("Error: couldn't opendir %s\n", dataDirectory);
	}
	else
	{
		rewinddir(dp);

		int list_ptr = 0;

		cur_files_all = 0;

		while (1)
		{
			struct dirent* dirp = readdir(dp);

			if (!dirp) break;

			if (dirp->d_type != DT_REG) continue;
			if ((strcmp(dirp->d_name, ".") == 0) || (strcmp(dirp->d_name, "..") == 0)) continue;

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

			if ((ebt_file_dlg_mode == FILE_DLG_MANAGER) || (strcmp(ext, ebt_file_filter) == 0))
			{
				char filepath[MAX_PATH];

				strncpy(filepath, dataDirectory, sizeof(filepath) - 1);
				strncat(filepath, dirp->d_name, sizeof(filepath) - 1);

				FILE* f = fopen(filepath, "rb");
				uint32_t f_size = 0;

				if (f)
				{
					fseek(f, 0, SEEK_END);
					f_size = ftell(f);
					fclose(f);
				}

				if ((cur_files_all >= from_file) && (list_ptr < ebt_file_list_hgt()))
				{
					strncpy(ebt_file_list_names[list_ptr], name, sizeof(ebt_file_list_names[list_ptr]) - 1);
							
					if (ebt_file_dlg_mode == FILE_DLG_MANAGER)	//show extensions in the file manager mode
					{
						strncat(ebt_file_list_names[list_ptr], ext, sizeof(ebt_file_list_names[list_ptr]) - 1);
					}

					ebt_file_list_sizes[list_ptr] = f_size;

					if (strncmp(ebt_file_name, name, sizeof(ebt_file_name)) == 0)
					{
						cur_file = cur_files_all;
						ebt_file_name[0] = 0;
					}

					++list_ptr;
				}

				ebt_file_dlg_used_k += (f_size + 1023) / 1024;

				++cur_files_all;
			}
		}
	}

	closedir(dp);
}

#endif
#ifdef TARGET_ESPBOY

#define TEMP_FILE_CACHE_NAME  "files.txt"

void ebt_file_browser_cache_file_list(void)
{
	sound_output_shut();

	fs::File fout = LittleFS.open(TEMP_FILE_CACHE_NAME, "w");

	fs::Dir dir = LittleFS.openDir("/");

	while (dir.next())
	{
		fs::File f = dir.openFile("r");

		if (!f) break;

		if (f.isFile())
		{
			fout.println(f.name());
			fout.println(f.size());
		}

		f.close();
	}

	fout.close();

	FSInfo fs_info;

	LittleFS.info(fs_info);

	ebt_file_dlg_used_k = (fs_info.usedBytes + 1023) / 1024;
	ebt_file_dlg_all_k = (fs_info.totalBytes + 1023) / 1024;

	sound_output_init();
}

void ebt_file_browser_update_list(int from_file)
{
	sound_output_shut();

	for (int i = 0; i < ebt_file_list_hgt(); ++i)
	{
		ebt_file_list_names[i][0] = 0;
		ebt_file_list_sizes[i] = 0;
	}

	int list_ptr = 0;

	cur_files_all = 0;

  fs::File file = LittleFS.open(TEMP_FILE_CACHE_NAME, "r");

	while (file.available())
	{
		String f_name = file.readStringUntil('\n');

		if (f_name.isEmpty()) break;

		String f_size = file.readStringUntil('\n');

		const char* f_name_c = f_name.c_str();

		//split filename into name and extension

		char name[MAX_FILENAME_LEN];
		char ext[5];

		int name_ptr = 0;
		int ext_ptr = 0;
		int dot = FALSE;

		for (unsigned int i = 0; i < strlen(f_name_c); ++i)
		{
			char c = f_name_c[i];

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

		if ((ebt_file_dlg_mode == FILE_DLG_MANAGER) || (strcmp(ext, ebt_file_filter) == 0))
		{
			if ((cur_files_all >= from_file) && (list_ptr < ebt_file_list_hgt()))
			{
				strncpy(ebt_file_list_names[list_ptr], name, sizeof(ebt_file_list_names[list_ptr]) - 1);
				
				if (ebt_file_dlg_mode == FILE_DLG_MANAGER)	//show extensions in the file manager mode
				{
					strncat(ebt_file_list_names[list_ptr], ext, sizeof(ebt_file_list_names[list_ptr]) - 1);
				}

				ebt_file_list_sizes[list_ptr] = atoi(f_size.c_str());

				if (strncmp(ebt_file_name, name, sizeof(ebt_file_name)) == 0)
				{
					cur_file = cur_files_all;
					ebt_file_name[0] = 0;
				}

				++list_ptr;
			}

			++cur_files_all;
		}
	}

	file.close();

	sound_output_init();
}

#endif


void ebt_file_browser_cat_read(void)
{

#ifdef TARGET_ESPBOY
	ebt_file_browser_cache_file_list();
#endif

	ebt_file_browser_update_list(ebt_file_browser_from_file());
	ebt_file_browser_update_list(ebt_file_browser_from_file()); //intentional
}



void ebt_file_browser_init(void)
{
	cur_file = 0;

	ebt_file_dlg_used_k = 0;
	ebt_file_dlg_all_k = 0;

	ebt_file_browser_cat_read();
}



void ebt_file_browser_draw(void)
{
	char buf[16];

	if (ebt_file_dlg_mode == FILE_DLG_MANAGER)
	{
		ebt_put_header("FMAN", COL_HEAD_NAME);

		set_font_color(COL_TEXT_DARK);
		set_back_color(COL_BACK);

		snprintf(buf, sizeof(buf), "%i/%iK", ebt_file_dlg_used_k, ebt_file_dlg_all_k);

		put_str(Text.width - (int8_t)strlen(buf), 0, buf);
	}
	else
	{
		ebt_put_header("FILE", COL_HEAD_FILE);
	}

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

	if (from_file < (cur_files_all - ebt_file_list_hgt()))
	{
		put_char(0, 2 + ebt_file_list_hgt() - 1, 0x11);
	}

	if (cur_files_all > 0)
	{
		snprintf(buf, sizeof(buf), "%i/%i", (cur_file + 1), cur_files_all);

		put_str((signed char)(Text.width - strlen(buf)), Text.height - 1, buf);

		if (ebt_file_dlg_mode == FILE_DLG_MANAGER)
		{
			int file_off = cur_file - from_file;

			if ((file_off >= 0) && (file_off < ebt_file_list_hgt()))
			{
				if (ebt_file_list_sizes[file_off] > 0)
				{
					snprintf(buf, sizeof(buf), "%i bytes", ebt_file_list_sizes[file_off]);
					put_str(0, Text.height - 1, buf);
				}
			}
		}
	}

	for (int i = 0; i < ebt_file_list_hgt(); ++i)
	{
		ebt_item_color(from_file == cur_file);

		put_str(1, 2 + i, ebt_file_list_names[i]);

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
			strncpy(ebt_file_name, ebt_file_list_names[cur_file - file_from], sizeof(ebt_file_name) - 1);

			if (ebt_file_dlg_callback_ok) ebt_file_dlg_callback_ok();
		}
	}

	if (pad_r & PAD_ESC)
	{
		memset(ebt_file_name, 0, sizeof(ebt_file_name));

		if (ebt_file_dlg_callback_cancel) ebt_file_dlg_callback_cancel();
	}
}
