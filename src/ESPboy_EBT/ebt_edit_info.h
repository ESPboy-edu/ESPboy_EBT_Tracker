const char* info_page_0[] = {
"@13~0fESPboy ~0etracker",
"@14~0e by shiru8bit",
"@26~05\3 \3~0f    \4  \5",
"@27~05\3~0f\x16~05\3~0f \5\5\5\4\4\4\5\5",
"@28~05\5\5\5~0f \5 \5\4 \4\5",
"@29~05 \4~0f  \5\5\5\4 \4\5",
"@2a~05\xff \xff~0f \5  \4 \4\5",
"@2b~05\1 \1~0f \5\5\5\4\4\4\5\5",
"~0e@2d",VERSION_STR,
"\n"
};

const char* info_page_1[] = {
"~3f@13LFT",
"@c3RGT",
"~0e@56Button",
"@57Layout",
"~3f@5aUp"
"~3f@caESC"
"~3f@1cLeft@7cRight",
"~3f@4eDown",
"~3f@ceACT",
"\n"
};

const char* info_page_2[] = {
"~0f@02ACT",
"~0e@04Press to select",
"@05or insert value",
"@07Hold+~0fUp~0e/~0fDown~0e",
"@08and ~0fLeft~0e/~0fRight~0e",
"@09to edit a value",
"@0bDouble tap to",
"@0cedit things"
"@0e~0fACT~0e+~0fESC~0e to clear",
"\n"
};

const char* info_page_3[] = {
"~0f@02ESC",
"~0e@04Press to cancel",
"@06Hold+~0fUp~0e/~0fDown~0e",
"@07to move quickly",
"@09Hold+~0fLeft~0e/~0fRight~0e",
"@0ato switch things",
"@0cDouble tap to"
"@0dactivate a menu",
"\n"
};

const char* info_page_4[] = {
"~0f@02LFT",
"~0e@04Hold to activate"
"@05the ~0fNAVI~0e screen"
"~0f@07RGT",
"~0e@09Controls play",
"@0aPress to hear",
"@0bfull song, order",
"@0cpos, a pattern,",
"@0dan instrument",
"\n"
};

const char* info_page_5[] = {
"@12~0fEffects",
"@14~0f0XY~0e arpeggio",
"@15~0f1XX~0e slide up",
"@16~0f2XX~0e slide down",
"@17~0f3XX~0e porta",
"@18~0f7XX~0e set phase",
"@19~0f8LR~0e pan control",
"@1a~0f9XX~0e set wave",
"@1b~0fC0V~0e set volume",
"@1c~0fEXY~0e options",
"@1d~0f 0X~0e arp speed",
"@1e~0fFXY~0e song speed",
"\n"
};

const char** const info_pages[] = {
	info_page_0,
	info_page_1,
	info_page_2,
	info_page_3,
	info_page_4,
	info_page_5,
};

signed char info_page_num = 0;

const signed char info_pages_all = sizeof(info_pages) / sizeof(const char*);



void ebt_edit_info_init(void)
{
}



void ebt_edit_info_draw(void)
{
	char buf[16];

	ebt_put_header("INFO", COL_HEAD_INFO);

	set_font_color(COL_TEXT_DARK);
	set_back_color(COL_BACK);

	snprintf(buf, sizeof(buf), "%i/%i", info_page_num + 1, info_pages_all);

	put_str((signed char)(Text.width - strlen(buf)), 0, buf);

	const char** str_array = info_pages[info_page_num];
	int id = 0;
	int done = FALSE;
	int tx = 0;
	int ty = 0;
	
	set_font_color(COL_TEXT);
	set_back_color(COL_BACK);

	while (!done)
	{
		const char* str = str_array[id];

		while (1)
		{
			char c = *str++;

			if (!c) break;
			if (c == '\n')
			{
				done = TRUE;
				break;
			}

			if ((uint8_t)c == 0xff) c = 0;
			if (c == '~')
			{
				set_back_color(ebt_parse_hex_char(*str++));
				set_font_color(ebt_parse_hex_char(*str++));
				continue;
			}

			if (c == '@')
			{
				tx = ebt_parse_hex_char(*str++);
				ty = ebt_parse_hex_char(*str++);
				continue;
			}

			if (tx >= 0 && tx < Text.width&&ty >= 0 && ty < Text.height)
			{
				put_char(tx, ty, c);
			}

			++tx;
		}

		++id;
	}
}



void ebt_edit_info_update(void)
{
	int pad_r = ebt_input_get_repeat();

	if (pad_r&(PAD_LEFT | PAD_UP))
	{
		--info_page_num;

		if (info_page_num < 0) info_page_num = info_pages_all - 1;
	}

	if (pad_r&(PAD_RIGHT | PAD_DOWN))
	{
		++info_page_num;

		if (info_page_num >= info_pages_all) info_page_num = 0;
	}
}