enum {
	PTN_MENU_NUM = 0,
	PTN_MENU_LEN,
	PTN_MENU_LOOP,
	PTN_MENU_INSERT,
	PTN_MENU_DELETE,
	PTN_MENU_TRANSPOSE,
	PTN_MENU_EFF,
	PTN_MENU_ALL
};

enum {
	PTN_COL_NOTE_OCTAVE = 0,
	PTN_COL_INS,
	PTN_COL_EFF_TYPE,
	PTN_COL_EFF_PARAM
};

int8_t ptn_menu_cur = PTN_MENU_NUM;
uint8_t ptn_menu_active = FALSE;
int8_t ptn_menu_double_tap = 0;
int8_t ptn_ins_double_tap = 0;
uint8_t ptn_act_released = TRUE;




void ebt_edit_pattern_init(void)
{
	ptn_menu_double_tap = 0;
	ptn_menu_active = FALSE;
	ptn_ins_double_tap = 0;
	ptn_act_released = TRUE;
}



void ebt_edit_pattern_draw(void)
{
	char buf[16];

	pattern_struct* ps = &song->ptns[cur_ptn_num];

	//draw screen header

	ebt_put_header("PTN", COL_HEAD_PTN);

	//draw pattern settings
	
	uint8_t col_item_back = 0;

	snprintf(buf, sizeof(buf), " %2.2X", cur_ptn_num);

	set_font_color(COL_TEXT_DARK);
	set_back_color(COL_BACK);
	put_str(0, 2, "NUM");

	ebt_item_color((ptn_menu_cur == PTN_MENU_NUM) && ptn_menu_active);

	put_str(0, 3, buf);

	snprintf(buf, sizeof(buf), " %2.2X", song->ptns[cur_ptn_num].len);

	set_font_color(COL_TEXT_DARK);
	set_back_color(COL_BACK);
	put_str(0, 4, "LEN");

	ebt_item_color((ptn_menu_cur == PTN_MENU_LEN) && ptn_menu_active);

	put_str(0, 5, buf);

	set_font_color(COL_TEXT_DARK);
	set_back_color(COL_BACK);
	put_str(0, 6, "CYC");

	ebt_item_color((ptn_menu_cur == PTN_MENU_LOOP) && ptn_menu_active);

	put_str(0, 7, song->ptns[cur_ptn_num].loop ? "YES" : " NO");

	ebt_item_color((ptn_menu_cur == PTN_MENU_INSERT) && ptn_menu_active);

	put_str(0, 9, "INS");

	ebt_item_color((ptn_menu_cur == PTN_MENU_DELETE) && ptn_menu_active);

	put_str(0, 10, "DEL");

	ebt_item_color((ptn_menu_cur == PTN_MENU_TRANSPOSE) && ptn_menu_active);

	put_str(0, 11, "TRA");

	int sy = Text.height - 3;

	set_font_color(COL_TEXT_DARK);
	set_back_color(COL_BACK);
	put_str(0, sy + 0, "EFF");

	ebt_item_color((ptn_menu_cur == PTN_MENU_EFF) && ptn_menu_active);

	snprintf(buf, sizeof(buf), "0%i", cur_eff_column + 1);
	put_str(1, sy + 1, buf);

	//draw pattern body

	int row = cur_ptn_row - Text.height / 2;

	if ((row + Text.height) >= ps->len) row = ps->len - Text.height;

	if (row < 0) row = 0;

	int cur_col = !ptn_menu_active ? cur_ptn_col : -1;
	int cur_row = 0;

	const uint8_t col_cursor = 0x05;

	int16_t play_row = ebt_player_report_row_position();

	for (int i = 0; i < Text.height; ++i)
	{
		pattern_row_struct* rs = &ps->rows[row];

		uint8_t col_back = COL_BACK_ROW;

		if (!(row & 3)) col_back = COL_BACK_BEAT;

		if (row == cur_ptn_row)
		{
			cur_row = i;
			col_back = COL_HEAD_PTN;
		}

		set_font_color(COL_TEXT);
		set_back_color(col_back);

		put_char(4, i, hex_to_char(row >> 4));
		put_char(5, i, hex_to_char(row & 0x0f));

		put_str(6, i, " ..... ..."); 
		
		if (row == play_row)
		{
			put_char(6, i, 7);
		}

		//draw note name and octave

		int note = rs->note;

		if (note > 0)
		{
			if ((note & 15) < 12)
			{
				buf[0] = note_names[note & 15][0];
				buf[1] = note_names[note & 15][1];
				buf[2] = '0' + (note >> 4);
			}
			else
			{
				buf[0] = 'R';
				buf[1] = '-';
				buf[2] = '-';
			}
		}
		else
		{
			buf[0] = '.';
			buf[1] = '.';
			buf[2] = '.';
		}

		if ((cur_ptn_row == row) && (cur_col == PTN_COL_NOTE_OCTAVE))
		{
			ebt_item_color(1);
		}
		else
		{
			set_font_color(COL_TEXT);
			set_back_color(col_back);
		}

		buf[3] = 0;
		put_str(7, i, buf);

		//draw instrument number

		if (rs->ins)
		{
			buf[0] = hex_to_char(rs->ins >> 4);
			buf[1] = hex_to_char(rs->ins & 0x0f);
		}
		else
		{
			buf[0] = '.';
			buf[1] = '.';
		}

		if ((cur_ptn_row == row) && (cur_col == PTN_COL_INS))
		{
			ebt_item_color(1);
		}
		else
		{
			set_font_color(COL_TEXT);
			set_back_color(col_back);
		}

		buf[2] = 0;
		put_str(10, i, buf);

		//draw mark if there are extra hidden effects

		int effect_cnt = 0;

		for (int e = 0; e < MAX_EFFECTS_PER_ROW; ++e)
		{
			if (e == cur_eff_column) continue;
			if ((rs->effect[e].type > 0) || (rs->effect[e].param != 0)) ++effect_cnt;
		}

		if (effect_cnt > 0)
		{
			set_font_color(COL_TEXT);
			set_back_color(col_back);

			put_char(12, i, '+');
		}

		//draw effect type

		if (rs->effect[cur_eff_column].type > 0)
		{
			buf[0] = hex_to_char(rs->effect[cur_eff_column].type - 1);
		}
		else
		{
			buf[0] = '.';
		}

		if ((cur_ptn_row == row) && (cur_col == PTN_COL_EFF_TYPE))
		{
			ebt_item_color(1);
		}
		else
		{
			set_font_color(COL_TEXT);
			set_back_color(col_back);
		}

		buf[1] = 0;
		put_str(13, i, buf);

		//draw effect param

		if ((rs->effect[cur_eff_column].type > 0) || (rs->effect[cur_eff_column].param > 0))
		{
			buf[0] = hex_to_char(rs->effect[cur_eff_column].param >> 4);
			buf[1] = hex_to_char(rs->effect[cur_eff_column].param & 0x0f);
		}
		else
		{
			buf[0] = '.';
			buf[1] = '.';
		}

		if ((cur_ptn_row == row) && (cur_col == PTN_COL_EFF_PARAM))
		{
			ebt_item_color(1);
		}
		else
		{
			set_font_color(COL_TEXT);
			set_back_color(col_back);
		}

		buf[2] = 0;
		put_str(14, i, buf);

		++row;

		if (row >= song->ptns[cur_ptn_num].len) break;
	}

	//draw pattern hints

	if (config.hints)
	{
		set_font_color(COL_TEXT_DARK);
		set_back_color(COL_BACK);

		const char* hint_str = 0;

		switch (cur_ptn_col)
		{
		case PTN_COL_NOTE_OCTAVE:
			if ((ps->rows[cur_ptn_row].note & 0x0f) >= 12)
			{
				hint_str = "REST";
			}
			else
			{
				hint_str = "NOTE";
			}
			break;

		case PTN_COL_INS:
			if (ps->rows[cur_ptn_row].ins > 0)
			{
				memset(buf, 0, MAX_INSTRUMENT_NAME_LEN + 1);
				memcpy(buf, &song->ins[ps->rows[cur_ptn_row].ins].name, MAX_INSTRUMENT_NAME_LEN);
				hint_str = buf;
			}
			else
			{
				hint_str = "INS";
			}
			break;

		case PTN_COL_EFF_TYPE:
			switch (ps->rows[cur_ptn_row].effect[cur_eff_column].type - 1)
			{
			case EFF_ARP: hint_str = "ARP"; break;
			case EFF_SLIDE_UP: hint_str = "SLUP"; break;
			case EFF_SLIDE_DOWN: hint_str = "SLDW"; break;
			case EFF_PORTA: hint_str = "PORT"; break;
			case EFF_PHASE: hint_str = "PHA"; break;
			case EFF_PAN: hint_str = "PAN"; break;
			case EFF_WAVE: hint_str = "WAVE"; break;
			case EFF_VOLUME: hint_str = "VOL"; break;
			case EFF_EXTRA: hint_str = "EXT"; break;
			case EFF_SPEED: hint_str = "SPD"; break;
			default: "NONE";
			}
			if (ps->rows[cur_ptn_row].effect[cur_eff_column].type == 0) hint_str = "EFF";
			break;

		case PTN_COL_EFF_PARAM:
			switch (ps->rows[cur_ptn_row].effect[cur_eff_column].type - 1)
			{
			case EFF_ARP: hint_str = "XY"; break;
			case EFF_SLIDE_UP: hint_str = "XX"; break;
			case EFF_SLIDE_DOWN: hint_str = "XX"; break;
			case EFF_PORTA: hint_str = "XX"; break;
			case EFF_PHASE: hint_str = "XX"; break;
			case EFF_PAN: hint_str = "LR"; break;
			case EFF_WAVE: hint_str = "XX"; break;
			case EFF_VOLUME: hint_str = "0X"; break;
			case EFF_EXTRA: hint_str = "XY"; break;
			case EFF_SPEED: hint_str = "XY"; break;
			default: hint_str = "PRM";
			}
			break;
		}

		if (hint_str)
		{
			int8_t hy = Text.height - 1;

			if (cur_row > Text.height / 2) hy = 0;

			put_str(Text.width - (int8_t)strlen(hint_str), hy, hint_str);
		}
	}
}



void ebt_pattern_insert_row(void)
{
	for (int row = MAX_PATTERN_LEN - 1; row >= cur_ptn_row; --row)
	{
		memcpy(&song->ptns[cur_ptn_num].rows[row], &song->ptns[cur_ptn_num].rows[row - 1], sizeof(pattern_row_struct));
	}

	memset(&song->ptns[cur_ptn_num].rows[cur_ptn_row], 0, sizeof(pattern_row_struct));
}



void ebt_pattern_delete_row(void)
{
	for (int row = cur_ptn_row; row < MAX_PATTERN_LEN - 1; ++row)
	{
		memcpy(&song->ptns[cur_ptn_num].rows[row], &song->ptns[cur_ptn_num].rows[row + 1], sizeof(pattern_row_struct));
	}

	memset(&song->ptns[cur_ptn_num].rows[MAX_PATTERN_LEN - 1], 0, sizeof(pattern_row_struct));
}



void ebt_pattern_move_cur(int step, BOOL relative)
{
	if (relative)
	{
		cur_ptn_row += step;
	}
	else
	{
		cur_ptn_row = step;
	}

	while (cur_ptn_row < 0) cur_ptn_row += song->ptns[cur_ptn_num].len;
	while (cur_ptn_row >= song->ptns[cur_ptn_num].len) cur_ptn_row -= song->ptns[cur_ptn_num].len;
}



void ebt_edit_pattern_update(void)
{
	uint8_t kb = ebt_input_get_kb();
	
	pattern_row_struct* ns = &song->ptns[cur_ptn_num].rows[cur_ptn_row];

	BOOL change = FALSE;

	if (kb == KB_TAB) ptn_menu_active ^= TRUE;
	if (kb == KB_UNDO) ebt_pattern_undo();

	if (!ptn_menu_active)
	{
		int8_t n = ebt_input_get_kb_hex();
		char c = ebt_input_get_kb_char();

		switch (cur_ptn_col)
		{
			case PTN_COL_NOTE_OCTAVE:
			{
				uint8_t prev_note = ns->note;

				if (c >= '1'&&c <= '9')
				{
					ns->note = (ns->note & 0x0f) | ((c - '0') << 4);
					change = TRUE;
				}

				uint8_t octave = cur_ptn_prev_note & 0xf0;

				int new_note = -1;
				switch (c)
				{
				case 'Z': new_note = octave + 0; break;
				case 'S': new_note = octave + 1; break;
				case 'X': new_note = octave + 2; break;
				case 'D': new_note = octave + 3; break;
				case 'C': new_note = octave + 4; break;
				case 'V': new_note = octave + 5; break;
				case 'G': new_note = octave + 6; break;
				case 'B': new_note = octave + 7; break;
				case 'H': new_note = octave + 8; break;
				case 'N': new_note = octave + 9; break;
				case 'J': new_note = octave + 10; break;
				case 'M': new_note = octave + 11; break;
				case 'A': new_note = octave + 15; break;
				}

				if (new_note >= 0)
				{
					ebt_pattern_undo_set();
					ns->note = new_note;
					change = TRUE;
					kb = 0;
				}

				if (ns->note != prev_note) cur_ptn_prev_note = ns->note;

				if (kb == KB_DELETE)
				{
					ebt_pattern_undo_set();
					ns->note = 0;
				}
			}
			break;

			case PTN_COL_INS:
			{
				uint8_t prev_ins = ns->ins;

				if (n >= 0) ns->ins = (ns->ins << 4) + n;
				
				if (ns->ins != prev_ins)
				{
					ebt_pattern_undo_set();
					cur_ptn_prev_ins = ns->ins;
					change = TRUE;
					kb = 0;
				}

				if (kb == KB_DELETE)
				{
					ebt_pattern_undo_set();
					ns->ins = 0;
				}
			}
			break;

			case PTN_COL_EFF_TYPE:
			{
				uint8_t prev_type = ns->effect[cur_eff_column].type;

				if (n >= 0)
				{
					ebt_pattern_undo_set();
					ns->effect[cur_eff_column].type = n;
					kb = 0;
				}
				
				if (ns->effect[cur_eff_column].type != prev_type)
				{
					cur_ptn_prev_effect = ns->effect[cur_eff_column].type;
					change = TRUE;
				}

				if (kb == KB_DELETE)
				{
					ebt_pattern_undo_set();
					ns->effect[cur_eff_column].type = 0;
				}
			}
			break;

			case PTN_COL_EFF_PARAM:
			{
				uint8_t prev_param = ns->effect[cur_eff_column].param;

				if (n >= 0)
				{
					ebt_pattern_undo_set();
					ns->effect[cur_eff_column].param = (ns->effect[cur_eff_column].param << 4) + n;
					kb = 0;
				}
				
				if (ns->effect[cur_eff_column].param != prev_param)
				{
					cur_ptn_prev_param = ns->effect[cur_eff_column].param;
					change = TRUE;
				}

				if (kb == KB_DELETE)
				{
					ebt_pattern_undo_set();
					ns->effect[cur_eff_column].param = 0;
				}
			}
			break;
		}

		switch (kb)
		{
		case KB_INSERT:
			ebt_pattern_undo_set();
			ebt_pattern_insert_row();
			break;
		case KB_BACKSPACE:
			ebt_pattern_undo_set(); 
			ebt_pattern_delete_row();
			break;
		case KB_PGUP: ebt_pattern_move_cur(-4, TRUE); break;
		case KB_PGDOWN: ebt_pattern_move_cur(4, TRUE); break;
		case KB_HOME: ebt_pattern_move_cur(0, FALSE); break;
		case KB_END: ebt_pattern_move_cur(MAX_PATTERN_LEN - 1, FALSE); break;
		}
	}

	int pad = ebt_input_get_state();
	int pad_t = ebt_input_get_trigger();
	int pad_r = ebt_input_get_repeat();

	if (cur_ptn_row >= song->ptns[cur_ptn_num].len)
	{
		cur_ptn_row = song->ptns[cur_ptn_num].len - 1;
	}

	if (ptn_menu_double_tap) --ptn_menu_double_tap;
	if (ptn_ins_double_tap) --ptn_ins_double_tap;

	if (ptn_menu_active)
	{
		if (!(pad & PAD_ACT))
		{
			if (pad_r & PAD_UP)
			{
				--ptn_menu_cur;

				if (ptn_menu_cur < 0) ptn_menu_cur = PTN_MENU_ALL - 1;
			}

			if (pad_r & PAD_DOWN)
			{
				++ptn_menu_cur;

				if (ptn_menu_cur >= PTN_MENU_ALL) ptn_menu_cur = 0;
			}
		}
		else
		{
			switch (ptn_menu_cur)
			{
			case PTN_MENU_INSERT:
				if (pad_t & PAD_ACT)
				{
					ebt_pattern_undo_set();
					ebt_pattern_insert_row();
				}
				break;

			case PTN_MENU_DELETE:
				if (pad_t & PAD_ACT)
				{
					ebt_pattern_undo_set();
					ebt_pattern_delete_row();
				}
				break;

			case PTN_MENU_TRANSPOSE:
				{
					int add = 0;
					if (pad_t&PAD_UP) add = 12;
					if (pad_t&PAD_DOWN) add = -12;
					if (pad_t&PAD_LEFT) add = -1;
					if (pad_t&PAD_RIGHT) add = 1;

					if (add != 0)
					{
						if (ebt_pattern_transpose(add, FALSE))
						{
							ebt_pattern_undo_set();
							ebt_pattern_transpose(add, TRUE);
						}
					}
				}
				break;
			}
		}

		switch (ptn_menu_cur)
		{
		case PTN_MENU_NUM:
		{
			uint8_t ptn = (uint8_t)cur_ptn_num;
			ebt_change_param_u8(&ptn, pad, pad_r, 1, 1, MAX_PATTERNS - 1, 16);
			cur_ptn_num = ptn;
		}
		break;

		case PTN_MENU_LEN:
			ebt_change_param_u8(&song->ptns[cur_ptn_num].len, pad, pad_r, DEFAULT_PATTERN_LEN, 1, MAX_PATTERN_LEN, 16);
			break;

		case PTN_MENU_LOOP:
			ebt_change_param_u8(&song->ptns[cur_ptn_num].loop, pad, pad_r, DEFAULT_PATTERN_LOOP, 0, 1, 1);
			break;

		case PTN_MENU_EFF:
			ebt_change_param_i8(&cur_eff_column, pad, pad_r, 0, 0, MAX_EFFECTS_PER_ROW - 1, 1, 0);
			break;
		}

		if (pad_t & PAD_ESC) ptn_menu_active = FALSE;

		return;
	}

	if (pad_t & PAD_ESC)
	{
		if (ptn_menu_double_tap > 0)
		{
			ptn_menu_active = TRUE;
			return;
		}

		ptn_menu_double_tap = DOUBLE_CLICK_TIMEOUT;
	}

	if (!(pad & PAD_ACT))
	{
		int step = 1;

		if (pad & PAD_ESC) step = 4;

		if (pad_r & PAD_UP) ebt_pattern_move_cur(-step, TRUE);
		if (pad_r & PAD_DOWN) ebt_pattern_move_cur(step, TRUE);

		if (!(pad&PAD_ESC))
		{
			if (pad_r & PAD_LEFT)
			{
				--cur_ptn_col;

				if (cur_ptn_col < 0) cur_ptn_col = 3;
			}

			if (pad_r & PAD_RIGHT)
			{
				++cur_ptn_col;

				if (cur_ptn_col >= 4) cur_ptn_col = 0;
			}
		}
		else
		{
			if (cur_ptn_col < 2)
			{
				if (pad_r&PAD_LEFT)
				{
					--cur_ptn_num;

					if (cur_ptn_num < 1) cur_ptn_num = MAX_PATTERNS - 1;
				}

				if (pad_r&PAD_RIGHT)
				{
					++cur_ptn_num;

					if (cur_ptn_num >= MAX_PATTERNS) cur_ptn_num = 1;
				}
			}
			else
			{
				if (pad_r&PAD_LEFT)
				{
					--cur_eff_column;

					if (cur_eff_column < 0) cur_eff_column = MAX_EFFECTS_PER_ROW - 1;
				}

				if (pad_r&PAD_RIGHT)
				{
					++cur_eff_column;

					if (cur_eff_column >= MAX_EFFECTS_PER_ROW) cur_eff_column = 0;
				}
			}
		}
	}

	switch (cur_ptn_col)
	{
	case PTN_COL_NOTE_OCTAVE:	//note and octave
	{
		if (pad & PAD_ACT)
		{
			int note_octave = ns->note;

			if (!note_octave)
			{
				if (pad_t & PAD_ACT)
				{
					note_octave = cur_ptn_prev_note;
				}
			}
			else
			{
				cur_ptn_prev_note = note_octave;

				int add = 0;

				if (pad_r & PAD_LEFT) add = -1;
				if (pad_r & PAD_RIGHT) add = 1;
				if (pad_r & PAD_UP) add = 16;
				if (pad_r & PAD_DOWN) add = -16;

				if (add != 0)
				{
					if (abs(add) < 16)
					{
						int note = note_octave & 15;

						note += add;

						if (note < 0) note = 12;
						if (note > 12) note = 0;

						note_octave = (note_octave & 0xf0) | note;
					}
					else
					{
						int octave = note_octave >> 4;

						octave += (add >> 4);

						if (octave < 1) octave = 1;
						if (octave > 9) octave = 9;

						note_octave = (note_octave & 0x0f) | (octave << 4);
					}
				}
			}

			if (pad & PAD_ESC)
			{
				note_octave = 0;
			}

			if (ns->note != note_octave)
			{
				change = TRUE;
				ebt_pattern_undo_set();
				ns->note = note_octave;
			}
		}
	}
	break;

	case PTN_COL_INS:	//instrument number
	{
		if (pad & PAD_ACT)
		{
			int ins = ns->ins;

			if (!ins)
			{
				if (pad_t & PAD_ACT)
				{
					ins = cur_ptn_prev_ins;
				}
			}
			else
			{
				if (pad_t&PAD_ACT)
				{
					if (ptn_ins_double_tap > 0)
					{
						if (ins > 0)
						{
							if (ins < MAX_INSTRUMENTS)
							{
								cur_ins = ins;

								set_edit_mode(EDIT_MODE_INSTRUMENT);
							}
							else
							{
								osd_message_set("NO INS");
							}
						}
					}

					ptn_ins_double_tap = DOUBLE_CLICK_TIMEOUT;
				}

				cur_ptn_prev_ins = ins;

				int add = 0;

				if (pad_r & PAD_LEFT) add = -1;
				if (pad_r & PAD_RIGHT) add = 1;
				if (pad_r & PAD_UP) add = 16;
				if (pad_r & PAD_DOWN) add = -16;

				if (add != 0)
				{
					ins += add;

					if (ins < 1) ins = 255;
					if (ins > 255) ins = 1;
				}
			}

			if (pad & PAD_ESC)
			{
				ins = 0;
			}

			if (ns->ins != ins)
			{
				ebt_pattern_undo_set();
				ns->ins = ins;
				change = TRUE;
			}

			if (ins > 0) cur_ins = ins;
		}
	}
	break;

	case PTN_COL_EFF_TYPE:	//effect type
	{
		if (pad & PAD_ACT)
		{
			int type = ns->effect[cur_eff_column].type;
			int param = ns->effect[cur_eff_column].param;

			if (!param && !type)
			{
				if (pad_t & PAD_ACT)
				{
					type = cur_ptn_prev_effect;

					if(cur_ptn_prev_param) param = cur_ptn_prev_param;
				}
			}
			else
			{
				cur_ptn_prev_effect = type;

				if(param) cur_ptn_prev_param = param;

				int add = 0;

				if (pad_r & PAD_LEFT) add = -1;
				if (pad_r & PAD_RIGHT) add = 1;
				if (pad_r & PAD_UP) add = 1;
				if (pad_r & PAD_DOWN) add = -1;

				if (add != 0)
				{
					type += add;

					if (type < 1) type = 16;	//0 no effect, 1..16 actual effects
					if (type > 16) type = 1;
				}
			}

			if (pad & PAD_ESC)
			{
				if (type > 0)
				{
					type = 0;
				}
				else
				{
					if (pad_t&(PAD_ACT | PAD_ESC)) param = 0;	//second press deletes the effect param as well
				}
			}

			if (ns->effect[cur_eff_column].type != type)
			{
				ebt_pattern_undo_set();
				ns->effect[cur_eff_column].type = type;
				change = TRUE;
			}
			if (ns->effect[cur_eff_column].param != param)
			{
				ebt_pattern_undo_set();
				ns->effect[cur_eff_column].param = param;
				change = TRUE;
			}
		}
	}
	break;
	
	case PTN_COL_EFF_PARAM:	//effect param
	{
		if (pad & PAD_ACT)
		{
			int type = ns->effect[cur_eff_column].type;
			int param = ns->effect[cur_eff_column].param;

			if (!param && !type)
			{
				if (pad_t & PAD_ACT)
				{
					if(cur_ptn_prev_effect) type = cur_ptn_prev_effect;
					param = cur_ptn_prev_param;
				}
			}
			else
			{
				if(type) cur_ptn_prev_effect = type;
				cur_ptn_prev_param = param;

				int add = 0;

				if (pad_r & PAD_LEFT) add = -1;
				if (pad_r & PAD_RIGHT) add = 1;
				if (pad_r & PAD_UP) add = 16;
				if (pad_r & PAD_DOWN) add = -16;

				if (add != 0)
				{
					param += add;

					if (param < 0) param = 255;
					if (param > 255) param = 0;
				}
			}

			if (pad & PAD_ESC)
			{
				if (param > 0)
				{
					param = 0;
				}
				else
				{
					if (pad_t&(PAD_ACT | PAD_ESC)) type = 0;	//second press deletes the effect type as well
				}
			}

			if (ns->effect[cur_eff_column].type != type)
			{
				ebt_pattern_undo_set();
				ns->effect[cur_eff_column].type = type;
				change = TRUE;
			}
			if (ns->effect[cur_eff_column].param != param)
			{
				ebt_pattern_undo_set();
				ns->effect[cur_eff_column].param = param;
				change = TRUE;
			}
		}
	}
	break;
	}

	BOOL play_row = FALSE;

	if (pad&PAD_ACT)
	{
		if (ptn_act_released)
		{
			ptn_act_released = FALSE;

			if ((config.ptn_sound_type == CFG_SOUND_TYPE_PRESS) || (config.ptn_sound_type == CFG_SOUND_TYPE_CHANGE)) play_row = TRUE;
		}
	}
	else
	{
		if (!ptn_act_released)
		{
			ptn_act_released = TRUE;

			if (config.ptn_sound_type == CFG_SOUND_TYPE_RELEASE) play_row = TRUE;
		}
	}

	if (change)
	{
		if (config.ptn_sound_type == CFG_SOUND_TYPE_CHANGE) play_row = TRUE;
	}

	if (play_row)
	{
		ebt_player_start_pattern_row((uint8_t)cur_ptn_num, (uint8_t)cur_ptn_row);
	}
}
