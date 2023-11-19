uint8_t ebt_get_frame(void)
{
	return ebt_frame_cnt;
}



uint8_t ebt_get_blink(void)
{
	return ebt_blink_cnt;
}



void ebt_fade_screen(void)
{
	for (int i = 0; i < Text.height; ++i)
	{
		for (int j = 0; j < Text.width; ++j)
		{
			uint8_t attr = get_attr(j, i);

			if ((attr & 0xf0) == (COL_BACK_ROW << 4)) attr &= 0x0f;

			if (attr & 0xf0) attr = COL_FADE << 4; else attr = COL_FADE;

			put_attr(j, i, attr);
		}
	}
}



void ebt_put_header(const char* text, uint8_t color)
{
	int x = strlen(text);

	set_font_color(COL_TEXT);
	set_back_color(color);
	put_str(0, 0, text);

	set_back_color(COL_BACK);
	set_font_color(color);
	put_char(x + 0, 0, 0x12);
	set_font_color(color);
	put_char(x + 1, 0, 0x13);
}



void ebt_navi_item_color(uint8_t active, uint8_t head_color)
{
	uint8_t back_color = head_color;

	if (active && !(ebt_get_blink() & ebt_config_get_blink_mask())) back_color = COL_BACK_CUR_2;

	set_font_color(COL_TEXT);
	set_back_color(back_color);
}



void osd_message_show(const char* msg)
{
	osd_message_text = msg;
	osd_message_time = 1;
}



void osd_message_set(const char* msg)
{
	osd_message_text = msg;
	osd_message_time = OSD_MSG_TIMEOUT;
}



void osd_message_set_custom(const char* msg, int timeout)
{
	osd_message_text = msg;
	osd_message_time = timeout;
}



void osd_message_clear(void)
{
	osd_message_text = NULL;
	osd_message_time = 0;
}



void osd_update(void)
{
	if (osd_message_text != NULL)
	{
		if (osd_message_time > 0)
		{
			int ox = Text.width / 2 - strlen(osd_message_text) / 2;
			int oy = Text.height / 2;

			ebt_fade_screen();

			set_font_color(COL_TEXT);
			set_back_color(COL_BACK_OSD);

			for (int i = 0; i < Text.width; ++i) put_char(i, oy, ' ');

			put_str(ox, oy, osd_message_text);

			--osd_message_time;
		}
		else
		{
			osd_message_text = NULL;
		}
	}
}
