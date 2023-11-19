signed char ebt_dlg_name_kb_x = 0;
signed char ebt_dlg_name_kb_y = 0;
signed char ebt_dlg_name_cur = 0;

#define EDIT_NAME_LEN	12

char ebt_dlg_input_str[EDIT_NAME_LEN + 1];

const char* ebt_dlg_name = NULL;

const char ebt_dlg_name_sym[40+1] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_ <e";

void(*ebt_edit_name_callback_ok)(void);
void(*ebt_edit_name_callback_cancel)(void);



void ebt_ask_name(const char* dlg_name, const char* default_name, void(*cb_ok)(void), void(*cb_cancel)(void))
{
	ebt_dlg_name = dlg_name;

	ebt_edit_name_callback_ok = cb_ok;
	ebt_edit_name_callback_cancel = cb_cancel;

	memset(ebt_dlg_input_str, 0x20, sizeof(ebt_dlg_input_str));
	ebt_dlg_input_str[sizeof(ebt_dlg_input_str) - 1] = 0;

	if (default_name)
	{
		int len = strlen(default_name);
		if (len >= sizeof(ebt_dlg_input_str) - 1) len = sizeof(ebt_dlg_input_str) - 1;
		memcpy(ebt_dlg_input_str, default_name, len);
	}

	set_edit_mode(EDIT_MODE_INPUT_NAME);
}



const char* ebt_get_name(void)
{
	int i = strlen(ebt_dlg_input_str) - 1;

	while (i >= 0 && ebt_dlg_input_str[i] == ' ')
	{
		ebt_dlg_input_str[i--] = 0;
	}

	return ebt_dlg_input_str;
}



void ebt_input_name_init(void)
{
	ebt_dlg_name_cur = sizeof(ebt_dlg_input_str) - 1;	//if the name is not empty, set cursor to the last character

	while (ebt_dlg_name_cur > 0)
	{
		if (ebt_dlg_input_str[ebt_dlg_name_cur] > ' ')
		{
			++ebt_dlg_name_cur;
			break;
		}
		--ebt_dlg_name_cur;
	}
	ebt_dlg_name_kb_x = 9;
	ebt_dlg_name_kb_y = 3;
}



void ebt_input_name_draw(void)
{
	ebt_put_header("NAME", COL_HEAD_NAME);

	if (ebt_dlg_name)
	{
		set_font_color(COL_TEXT_DARK);
		set_back_color(COL_BACK);
		put_str(Text.width - (signed char)strlen(ebt_dlg_name), 0, ebt_dlg_name);
	}

	set_font_color(COL_TEXT);
	set_back_color(COL_BACK_BEAT);

	int nx = 2;
	int ny = 6;

	put_str(nx, ny, ebt_dlg_input_str);
	put_attr(nx + ebt_dlg_name_cur, ny, 0x5f);

	set_font_color(COL_TEXT);
	set_back_color(COL_BACK);

	int kx = 3;
	int ky = 8;

	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 10; ++j)
		{
			ebt_item_color((j == ebt_dlg_name_kb_x) && (i == ebt_dlg_name_kb_y));
			put_char(kx + j, ky + i, ebt_dlg_name_sym[i * 10 + j]);
		}
	}
}



void ebt_input_name_add(char c)
{
	ebt_dlg_input_str[ebt_dlg_name_cur] = c;

	if (ebt_dlg_name_cur < (EDIT_NAME_LEN - 1)) ++ebt_dlg_name_cur;
}



void ebt_input_name_backspace(void)
{
	if (ebt_dlg_name_cur == (EDIT_NAME_LEN - 1))
	{
		if (ebt_dlg_input_str[ebt_dlg_name_cur] != ' ')
		{
			ebt_dlg_input_str[ebt_dlg_name_cur] = ' ';
			return;
		}
	}

	if (ebt_dlg_name_cur > 0) --ebt_dlg_name_cur;

	ebt_dlg_input_str[ebt_dlg_name_cur] = ' ';
}



void ebt_input_name_done(void)
{
	if (strlen(ebt_dlg_input_str) > 0)
	{
		if (ebt_edit_name_callback_ok) ebt_edit_name_callback_ok();
	}
}



void ebt_input_name_update(void)
{
	int pad_r = ebt_input_get_repeat();

	if (pad_r&PAD_UP)
	{
		--ebt_dlg_name_kb_y;

		if (ebt_dlg_name_kb_y < 0) ebt_dlg_name_kb_y = 3;
	}

	if (pad_r&PAD_DOWN)
	{
		++ebt_dlg_name_kb_y;

		if (ebt_dlg_name_kb_y > 3) ebt_dlg_name_kb_y = 0;
	}

	if (pad_r&PAD_LEFT)
	{
		--ebt_dlg_name_kb_x;

		if (ebt_dlg_name_kb_x < 0) ebt_dlg_name_kb_x = 9;
	}

	if (pad_r&PAD_RIGHT)
	{
		++ebt_dlg_name_kb_x;

		if (ebt_dlg_name_kb_x > 9) ebt_dlg_name_kb_x = 0;
	}

	if (pad_r&PAD_ACT)
	{
		char c = ebt_dlg_name_sym[ebt_dlg_name_kb_y * 10 + ebt_dlg_name_kb_x];

		if (c == 'e')
		{
			ebt_input_name_done();
			return;
		}

		if (c == '<')
		{
			ebt_input_name_backspace();
			return;
		}

		ebt_input_name_add(c);
	}

	if (pad_r&PAD_ESC)
	{
		if (ebt_edit_name_callback_cancel) ebt_edit_name_callback_cancel();
	}

	uint8_t kb_code = ebt_input_get_kb();

	if (kb_code != KB_NONE)
	{
		if (kb_code == KB_BACKSPACE)
		{
			ebt_input_name_backspace();
			return;
		}

		char c = ebt_input_get_kb_char();

		if(c) ebt_input_name_add(c);
	}
}