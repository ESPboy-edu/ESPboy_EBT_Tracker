uint8_t ebt_confirm_cur = 1;
const char* ebt_confirm_msg = NULL;
void(*ebt_confirm_callback_yes)(void);
void(*ebt_confirm_callback_no)(void);

void ebt_fade_screen(void);



void ebt_ask_confirm(const char* msg, void(*cb_yes)(void), void(*cb_no)(void))
{
	ebt_confirm_cur = 1;
	ebt_confirm_msg = msg;
	ebt_confirm_callback_yes = cb_yes;
	ebt_confirm_callback_no = cb_no;
}



void ebt_confirm_draw(void)
{
	ebt_fade_screen();

	set_font_color(COL_TEXT);
	set_back_color(COL_BACK_WARN);

	int cx = TEXT_SCREEN_WDT / 2;
	int cy = TEXT_SCREEN_HGT / 2;

	for (int i = 0; i < TEXT_SCREEN_WDT; ++i) put_char(i, cy - 1, ' ');

	put_str((signed char)(cx - strlen(ebt_confirm_msg) / 2), cy - 1, ebt_confirm_msg);

	ebt_item_color(ebt_confirm_cur == 0);

	put_str(cx - 4, cy + 1, "YES");

	ebt_item_color(ebt_confirm_cur == 1);

	put_str(cx + 2, cy + 1, "NO");
}



void ebt_confirm_callback(void)
{
	if (ebt_confirm_cur == 0)
	{
		if (ebt_confirm_callback_yes) ebt_confirm_callback_yes();
	}
	else
	{
		if (ebt_confirm_callback_no) ebt_confirm_callback_no();
	}
}



BOOL ebt_confirm_update(void)
{
	if (!ebt_confirm_msg) return FALSE;

	ebt_confirm_draw();

	int pad_t = ebt_input_get_trigger();

	if (pad_t&PAD_LEFT)
	{
		ebt_confirm_cur = 0;
	}

	if (pad_t&PAD_RIGHT)
	{
		ebt_confirm_cur = 1;
	}

	if (pad_t&PAD_ACT)
	{
		ebt_confirm_msg = NULL;
		ebt_confirm_callback();
	}

	if (pad_t&PAD_ESC)
	{
		ebt_confirm_cur = 1;
		ebt_confirm_msg = NULL;
		ebt_confirm_callback();
	}

	return TRUE;
}