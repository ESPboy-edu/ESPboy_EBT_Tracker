enum {
	ORDER_MENU_MODE=0,
	ORDER_MENU_INSERT,
	ORDER_MENU_DELETE,
	ORDER_MENU_SELECT,
	ORDER_MENU_MUTE_A,
	ORDER_MENU_MUTE_B,
	ORDER_MENU_MUTE_C,
	ORDER_MENU_MUTE_D,
	ORDER_MENU_ALL
};

signed char order_menu_double_tap = 0;
signed char order_set_loop_hold = 0;
uint8_t order_menu_active = FALSE;
signed char order_menu_cur = ORDER_MENU_MODE;
signed char order_ptn_double_tap = 0;


void ebt_order_selection_reset(void)
{
	order_sel_active = FALSE;
}



void ebt_edit_order_init(void)
{
	order_menu_active = FALSE;
	order_sel_active = FALSE;
	order_menu_double_tap = 0;
	order_ptn_double_tap = 0;
	order_set_loop_hold = 0;
}



uint8_t ebt_config_get_ord_highlight(void);

void ebt_edit_order_draw(void)
{
	char buf[16];

	//draw order header

	ebt_put_header("ORD", COL_HEAD_ORD);

	set_font_color(COL_TEXT_DARK);
	set_back_color(COL_BACK);

	put_str(0, 2, "MODE"); 
	
	ebt_item_color((order_menu_cur == ORDER_MENU_MODE) && order_menu_active);

	put_str(0, 3, !order_trans_mode ? "POS" : "TRAN");

	ebt_item_color((order_menu_cur == ORDER_MENU_INSERT) && order_menu_active);

	put_str(0, 5, "INS");

	ebt_item_color((order_menu_cur == ORDER_MENU_DELETE) && order_menu_active);

	put_str(0, 7, "DEL");

	ebt_item_color((order_menu_cur == ORDER_MENU_SELECT) && order_menu_active);

	put_str(0, 9, "SEL");

	int sy = Text.height - 5;
	
	set_font_color(COL_TEXT_DARK);
	set_back_color(COL_BACK);

	put_str(0, sy + 2, "ABCD");
	
	set_font_color(COL_TEXT);

	ebt_item_color((order_menu_cur == ORDER_MENU_MUTE_A) && order_menu_active);

	put_char(0, sy + 3, (mute_state&MUTE_CH1) ? '\3' : '\5');

	ebt_item_color((order_menu_cur == ORDER_MENU_MUTE_B) && order_menu_active);

	put_char(1, sy + 3, (mute_state&MUTE_CH2) ? '\3' : '\5');

	ebt_item_color((order_menu_cur == ORDER_MENU_MUTE_C) && order_menu_active);

	put_char(2, sy + 3, (mute_state&MUTE_CH3) ? '\3' : '\5');

	ebt_item_color((order_menu_cur == ORDER_MENU_MUTE_D) && order_menu_active);

	put_char(3, sy + 3, (mute_state&MUTE_CH4) ? '\3' : '\5');

	ebt_vumeter_draw(0, sy);

	//draw order body

	int ord_pos = cur_ord_pos - Text.height / 2;

	if ((ord_pos + Text.height) >= MAX_ORDER_LEN) ord_pos = MAX_ORDER_LEN - Text.height;

	if (ord_pos < 0) ord_pos = 0;

	const uint8_t col_cursor = 0x05;

	const uint8_t ptn_highlight = song->order.pos[cur_ord_pos].ptn[cur_ord_chn];
	const BOOL use_highlight = (ebt_config_get_ord_highlight() && !order_sel_active) ? TRUE : FALSE;

	int16_t play_pos = ebt_player_report_order_position();

	for (int row = 0; row < Text.height; ++row)
	{
		uint8_t col_back = COL_BACK_ROW;

		if (!(ord_pos & 3)) col_back = COL_BACK_BEAT;

		if (ord_pos == cur_ord_pos) col_back = COL_HEAD_ORD;

		set_font_color(COL_TEXT);
		set_back_color(col_back);

		snprintf(buf, sizeof(buf), "%2.2X ", ord_pos);

		put_str(5, row, buf);

		if (ord_pos == song->order.loop_start)
		{
			put_char(4, row, 20);
		}

		if (ord_pos == song->order.loop_end)
		{
			put_char(7, row, 21);
		}

		if (ord_pos == play_pos)
		{
			put_char(7, row, 7);
		}

		for (int ch = 0; ch < MAX_CHANNELS; ++ch)
		{
			if ((ord_pos == cur_ord_pos) && (ch == cur_ord_chn) && !order_menu_active)
			{
				if (!order_sel_active)
				{
					ebt_item_color_custom(1, COL_BACK_CUR_2, COL_BACK_CUR_1, col_back);
				}
				else
				{
					ebt_item_color_custom(1, COL_BACK_CUR_2, COL_BACK_SEL, COL_BACK_SEL);
				}
			}
			else
			{
				set_font_color(COL_TEXT);
				set_back_color(col_back);

				if (order_sel_active)
				{
					if (ord_pos >= order_sel_from_pos && ord_pos <= order_sel_to_pos)
					{
						if (ch >= order_sel_from_chn && ch <= order_sel_to_chn)
						{
							set_back_color(COL_BACK_SEL);
						}
					}
				}
			}

			if (!order_trans_mode)
			{
				int ptn = song->order.pos[ord_pos].ptn[ch];

				if(use_highlight)
				{
					if (ptn != ptn_highlight) set_font_color(COL_TEXT_DARK);
				}

				if (ptn)
				{
					buf[0]=hex_to_char(ptn>>4);
					buf[1]=hex_to_char(ptn&0x0f);
				}
				else
				{
					buf[0] = '.';
					buf[1] = '.';
				}
			}
			else
			{
				int trans = song->order.pos[ord_pos].trans[ch];

				if (trans < 0) snprintf(buf, sizeof(buf), "-%1.1X", abs(trans));
				if (trans > 0) snprintf(buf, sizeof(buf), "+%1.1X", abs(trans));
				if (trans == 0)
				{
					buf[0] = ' ';
					buf[1] = '.';
				}
			}

			put_str(8 + ch * 2, row, buf);
		}

		++ord_pos;
	}
}



void ebt_order_insert_pos(void)
{
	for (int pos = MAX_ORDER_LEN - 1; pos >= cur_ord_pos; --pos)
	{
		memcpy(&song->order.pos[pos], &song->order.pos[pos - 1], sizeof(order_pos_struct));
	}

	memset(&song->order.pos[cur_ord_pos], 0, sizeof(order_pos_struct));
}



void ebt_order_delete_pos(void)
{
	for (int pos = cur_ord_pos; pos < MAX_ORDER_LEN - 1; ++pos)
	{
		memcpy(&song->order.pos[pos], &song->order.pos[pos + 1], sizeof(order_pos_struct));
	}

	memset(&song->order.pos[MAX_ORDER_LEN - 1], 0, sizeof(order_pos_struct));
}



uint8_t ebt_find_first_unused_pattern(void)
{
	uint8_t use[MAX_PATTERNS];

	memset(use, FALSE, sizeof(use));

	for (int pos = 0; pos < MAX_ORDER_LEN; ++pos)
	{
		for (int ch = 0; ch < MAX_CHANNELS; ++ch)
		{
			use[song->order.pos[pos].ptn[ch]] = TRUE;
		}
	}

	int ptn = 0;

	for (int i = 0; i < MAX_PATTERNS; ++i)
	{
		if (!use[i])
		{
			ptn = i;
			break;
		}
	}

	return ptn;
}



void ebt_order_move_cur(int step)
{
	cur_ord_pos += step;

	if (cur_ord_pos < 0) cur_ord_pos = 0;
	if (cur_ord_pos >= MAX_ORDER_LEN) cur_ord_pos = MAX_ORDER_LEN - 1;

	cur_ord_same_pos = 0;
}



void ebt_edit_order_update(void)
{
	ebt_vumeter_update();

	uint8_t kb = ebt_input_get_kb();

	if (kb == KB_TAB)
	{
		order_menu_active ^= TRUE;
	}

	//check controls

	int pad = ebt_input_get_state();
	int pad_t = ebt_input_get_trigger();
	int pad_r = ebt_input_get_repeat();

	if (order_menu_double_tap) --order_menu_double_tap;
	if (order_ptn_double_tap) --order_ptn_double_tap;

	if (!order_menu_active)
	{
		int8_t n = ebt_input_get_kb_hex();

		order_pos_struct* ops = &song->order.pos[cur_ord_pos];

		switch (kb)
		{
		case KB_BACKSPACE:
			if (!order_trans_mode)
			{
				ops->ptn[cur_ord_chn] = 0;
			}
			else
			{
				ops->trans[cur_ord_chn] = 0;
			}
			return;

		case KB_INSERT: ebt_order_insert_pos(); return;
		case KB_DELETE: ebt_order_delete_pos(); return;
		case KB_PGUP: ebt_order_move_cur(-16); break;
		case KB_PGDOWN: ebt_order_move_cur(16); break;
		case KB_HOME: ebt_order_move_cur(-MAX_ORDER_LEN); break;
		case KB_END: ebt_order_move_cur(MAX_ORDER_LEN); break;
		}

		if (!order_trans_mode)
		{
			if (n >= 0)
			{
				ops->ptn[cur_ord_chn] = ((ops->ptn[cur_ord_chn]) << 4) + n;
				return;
			}
		}
		else
		{
			if (n >= 0)
			{
				BOOL neg = ops->trans[cur_ord_chn] < 0 ? TRUE : FALSE;
				ops->trans[cur_ord_chn] = n;
				if (neg) ops->trans[cur_ord_chn] = -ops->trans[cur_ord_chn];
				return;
			}

			if (kb == KB_MINUS)
			{
				ops->trans[cur_ord_chn] = -ops->trans[cur_ord_chn];
				return;
			}
		}
	}

	if (order_menu_active)
	{
		if (!(pad&PAD_ACT))
		{
			if (pad_r&PAD_UP)
			{
				--order_menu_cur;
			}

			if (pad_r&PAD_DOWN)
			{
				++order_menu_cur;	
			}

			if (pad_r&PAD_LEFT)
			{
				if (order_menu_cur < ORDER_MENU_MUTE_A)
				{
					order_menu_cur = ORDER_MENU_MUTE_D;
				}
				else
				{
					--order_menu_cur;
				}
			}

			if (pad_r&PAD_RIGHT)
			{
				if (order_menu_cur < ORDER_MENU_MUTE_A)
				{
					order_menu_cur = ORDER_MENU_MUTE_A;
				}
				else
				{
					++order_menu_cur;
				}
			}

			if (order_menu_cur < 0) order_menu_cur = ORDER_MENU_ALL - 1;
			if (order_menu_cur >= ORDER_MENU_ALL) order_menu_cur = 0;
		}
		else
		{
			switch (order_menu_cur)
			{
			case ORDER_MENU_MODE:
				{
					if (pad_r&PAD_ACT) order_trans_mode ^= TRUE;
				}
			break;

			case ORDER_MENU_INSERT:
				{
					if (pad_t&PAD_ACT) ebt_order_insert_pos();
				}
				break;

			case ORDER_MENU_DELETE:
				{
					if (pad_t&PAD_ACT) ebt_order_delete_pos();
				}
				break;

			case ORDER_MENU_SELECT:
				{
					if (pad_t&PAD_ACT)
					{
						if (!order_sel_active)
						{
							order_sel_start_pos = cur_ord_pos;
							order_sel_start_chn = cur_ord_chn;
							order_sel_from_pos = order_sel_start_pos;
							order_sel_from_chn = order_sel_start_chn;
							order_sel_to_pos = order_sel_start_pos;
							order_sel_to_chn = order_sel_start_chn;
							order_sel_active = TRUE;
						}
						else
						{
							order_sel_active = FALSE;
						}

						order_menu_active = FALSE;
					}
				}
				break;

			case ORDER_MENU_MUTE_A: if (pad_r&PAD_ACT) mute_state ^= MUTE_CH1; break;
			case ORDER_MENU_MUTE_B: if (pad_r&PAD_ACT) mute_state ^= MUTE_CH2; break;
			case ORDER_MENU_MUTE_C: if (pad_r&PAD_ACT) mute_state ^= MUTE_CH3; break;
			case ORDER_MENU_MUTE_D: if (pad_r&PAD_ACT) mute_state ^= MUTE_CH4; break;
			}
		}

		if (pad_t&PAD_ESC) order_menu_active = FALSE;

		return;
	}

	if (!(pad&PAD_ACT))
	{
		int step = 1;

		if (pad&PAD_ESC) step = 16;

		if (pad_r&PAD_UP) ebt_order_move_cur(-step);
		if (pad_r&PAD_DOWN) ebt_order_move_cur(step);

		if (!(pad&PAD_ESC))
		{
			if (pad_r&PAD_LEFT)
			{
				--cur_ord_chn;

				if (cur_ord_chn < 0) cur_ord_chn = MAX_CHANNELS - 1;

				cur_ord_same_pos = 0;
			}

			if (pad_r&PAD_RIGHT)
			{
				++cur_ord_chn;

				if (cur_ord_chn >= MAX_CHANNELS) cur_ord_chn = 0;

				cur_ord_same_pos = 0;
			}
		}
		else
		{
			if (pad_t&(PAD_LEFT | PAD_RIGHT)) order_trans_mode ^= TRUE;

			if (pad&PAD_LEFT)
			{
				if (order_set_loop_hold < DOUBLE_CLICK_TIMEOUT)
				{
					++order_set_loop_hold;
				}
				else
				{
					song->order.loop_start = cur_ord_pos;
				}
			}
			else
			if (pad&PAD_RIGHT)
			{
				if (order_set_loop_hold < DOUBLE_CLICK_TIMEOUT)
				{
					++order_set_loop_hold;
				}
				else
				{
					song->order.loop_end = cur_ord_pos;
				}
			}
			else
			{
				order_set_loop_hold = 0;
			}
		}
	}

	if (pad&PAD_ACT)
	{
		if (!order_trans_mode)
		{
			int ptn = song->order.pos[cur_ord_pos].ptn[cur_ord_chn];

			if (pad_t&PAD_ACT)
			{
				if (order_ptn_double_tap > 0)
				{
					if (ptn > 0)
					{
						if (ptn < MAX_PATTERNS)
						{
							cur_ptn_num = ptn;
							set_edit_mode(EDIT_MODE_PATTERN);
							return;
						}
						else
						{
							osd_message_set("NO PATTERN");
						}
					}
				}

				order_ptn_double_tap = DOUBLE_CLICK_TIMEOUT;
			}

			if (!ptn)
			{
				if (pad_t&PAD_ACT)
				{
					ptn = cur_ord_prev_ptn;
					cur_ord_same_pos = 1;
				}
			}
			else
			{
				cur_ord_prev_ptn = ptn;

				int add = 0;

				if (cur_ord_same_pos)
				{
					if (pad_t&PAD_ACT)
					{
						add = 0;
						ptn = ebt_find_first_unused_pattern();
						cur_ord_same_pos = 0;
					}
				}

				if (pad_r&PAD_LEFT) add = -1;
				if (pad_r&PAD_RIGHT) add = 1;
				if (pad_r&PAD_UP) add = 16;
				if (pad_r&PAD_DOWN) add = -16;

				ptn += add;

				if (ptn < 1) ptn = 1;
				if (ptn > 255) ptn = 255;
			}

			if (pad&PAD_ESC)
			{
				ptn = 0;
			}

			song->order.pos[cur_ord_pos].ptn[cur_ord_chn] = ptn;
			
			if (ptn) cur_ptn_num = ptn;
		}
		else
		{
			signed char trans = song->order.pos[cur_ord_pos].trans[cur_ord_chn];

			if (!trans) trans = cur_ord_prev_trans;

			ebt_change_param_i8(&trans, pad, pad_r, 0, -15, 15, 1, TRUE);

			if (pad&PAD_ESC)
			{
				trans = 0;
			}

			if (trans) cur_ord_prev_trans = trans;

			song->order.pos[cur_ord_pos].trans[cur_ord_chn] = trans;
		}
	}

	if (pad_t&PAD_ESC)
	{
		if (order_menu_double_tap > 0)
		{
			order_menu_active = TRUE;
			return;
		}

		order_menu_double_tap = DOUBLE_CLICK_TIMEOUT;
	}

	if (order_sel_active)
	{
		if (cur_ord_pos < order_sel_start_pos)
		{
			order_sel_from_pos = cur_ord_pos;
			order_sel_to_pos = order_sel_start_pos;
		}
		else
		{
			order_sel_from_pos = order_sel_start_pos;
			order_sel_to_pos = cur_ord_pos;
		}

		if (cur_ord_chn < order_sel_start_chn)
		{
			order_sel_from_chn = cur_ord_chn;
			order_sel_to_chn = order_sel_start_chn;
		}
		else
		{
			order_sel_from_chn = order_sel_start_chn;
			order_sel_to_chn = cur_ord_chn;
		}
	}
}
