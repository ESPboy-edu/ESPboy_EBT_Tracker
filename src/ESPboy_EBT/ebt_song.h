void ebt_instrument_copy(void)
{
	memcpy(&clipboard_ins, &song->ins[cur_ins], sizeof(instrument_struct));

	clipboard_ins_empty = false;
}



void ebt_instrument_paste(void)
{
	if (!clipboard_ins_empty)
	{
		memcpy(&song->ins[cur_ins], &clipboard_ins, sizeof(instrument_struct));
	}
}



void ebt_pattern_copy(void)
{
	memcpy(&clipboard_pattern, &song->ptns[cur_ptn_num], sizeof(pattern_struct));

	clipboard_pattern_empty = false;
}



void ebt_pattern_paste(void)
{
	if (!clipboard_pattern_empty)
	{
		memcpy(&song->ptns[cur_ptn_num], &clipboard_pattern, sizeof(pattern_struct));
	}
}



void ebt_order_copy(void)
{
	if (!order_sel_active) return;

	int ptr = 0;

	for (int i = order_sel_from_pos; i <= order_sel_to_pos; ++i)
	{
		for (int j = order_sel_from_chn; j <= order_sel_to_chn; ++j)
		{
			if (!order_trans_mode)
			{
				clipboard_order[ptr++] = song->order.pos[i].ptn[j];
			}
			else
			{
				clipboard_order[ptr++] = song->order.pos[i].trans[j];
			}
		}
	}

	if (!order_trans_mode)
	{
		clipboard_order_type = ORDER_CLIPBOARD_PATTERNS;
	}
	else
	{
		clipboard_order_type = ORDER_CLIPBOARD_TRANS;
	}

	clipboard_order_w = order_sel_to_chn - order_sel_from_chn + 1;
	clipboard_order_h = order_sel_to_pos - order_sel_from_pos + 1;

	ebt_order_selection_reset();
}



void ebt_order_paste(void)
{
	if (clipboard_order_type == ORDER_CLIPBOARD_NONE) return;
	if ((clipboard_order_type == ORDER_CLIPBOARD_PATTERNS) && order_trans_mode) return;
	if ((clipboard_order_type == ORDER_CLIPBOARD_TRANS) && !order_trans_mode) return;

	int ptr = 0;

	for (int i = cur_ord_pos; i < (cur_ord_pos + clipboard_order_h); ++i)
	{
		for (int j = cur_ord_chn; j < (cur_ord_chn + clipboard_order_w); ++j)
		{
			if ((i >= 0) && (i < MAX_ORDER_LEN) && (j >= 0) && (j < MAX_CHANNELS))
			{
				if (!order_trans_mode)
				{
					song->order.pos[i].ptn[j] = clipboard_order[ptr++];
				}
				else
				{
					song->order.pos[i].trans[j] = clipboard_order[ptr++];
				}
			}
		}
	}

	ebt_order_selection_reset();
}



void ebt_song_clear_pattern(int ptn)
{
	memset(&song->ptns[ptn], 0, sizeof(pattern_struct));

	song->ptns[ptn].len = DEFAULT_PATTERN_LEN;
	song->ptns[ptn].loop = DEFAULT_PATTERN_LOOP;
}



void ebt_song_clear_instrument(int ins)
{
	memcpy(&song->ins[ins], ins_default, sizeof(instrument_struct));
}



BOOL ebt_song_is_pattern_clear(int ptn)
{
	if (song->ptns[ptn].len != DEFAULT_PATTERN_LEN) return FALSE;
	if (song->ptns[ptn].loop != DEFAULT_PATTERN_LOOP) return FALSE;

	for (int row = 0; row < MAX_PATTERN_LEN; ++row)
	{
		pattern_row_struct* rs = &song->ptns[ptn].rows[row];

		if (rs->note) return FALSE;
		if (rs->ins) return FALSE;

		for (int e = 0; e < MAX_EFFECTS_PER_ROW; ++e)
		{
			if (rs->effect[e].type) return FALSE;
			if (rs->effect[e].param) return FALSE;
		}
	}

	return TRUE;
}



void ebt_song_clear(BOOL clear_instruments)
{
	mute_state = 0;

	order_trans_mode = FALSE;

	cur_ptn_row = 0;
	cur_ptn_col = 0;
	cur_ptn_num = 1;
	cur_ord_pos = 0;
	cur_ord_chn = 0;
	cur_ord_prev_ptn = 1;
	cur_ord_prev_trans = 1;
	cur_ptn_prev_note = 3 << 4;	//C-3
	cur_ord_same_pos = 0;
	cur_ptn_prev_ins = 1;
	cur_ptn_prev_effect = 1;
	cur_ptn_prev_param = 1;
	cur_ins = 1;
	cur_eff_column = 0;

	song->speed_even = DEFAULT_SONG_SPEED;
	song->speed_odd = DEFAULT_SONG_SPEED;
	song->speed_interleave = 1;

	memset(&song->order, 0, sizeof(order_struct));
	memset(&song->name, 0, sizeof(song->name));
	memset(&song->author, 0, sizeof(song->author));
	memset(&song->pan_default, 0, sizeof(song->pan_default));

	song->order.loop_start = MAX_ORDER_LEN - 1;
	song->order.loop_end = MAX_ORDER_LEN - 1;

	for (int ptn = 0; ptn < MAX_PATTERNS; ++ptn)
	{
		ebt_song_clear_pattern(ptn);
	}

	ins_default = &song->ins[0];

	memset(ins_default, 0, sizeof(instrument_struct)); //also used in the song save function

	ins_default->volume = DEFAULT_VOLUME;
	ins_default->base_note = DEFAULT_BASE_NOTE;

	if (clear_instruments)
	{
		for (int ins = 1; ins < MAX_INSTRUMENTS; ++ins)
		{
			ebt_song_clear_instrument(ins);
		}
	}

	osd_message_clear();
}



int ebt_song_get_instruments_in_use(uint8_t* in_use, int in_use_size)
{
	memset(in_use, FALSE, in_use_size);

	int ins_count = 0;

	for (int ptn = 0; ptn < MAX_PATTERNS; ++ptn)
	{
		pattern_struct* ps = &song->ptns[ptn];

		for (int row = 0; row < MAX_PATTERN_LEN; ++row)
		{
			int ins = ps->rows[row].ins;

			if (ins < in_use_size)
			{
				if (!in_use[ins])
				{
					in_use[ins] = TRUE;
					++ins_count;
				}
			}
		}
	}

	return ins_count;
}



int ebt_song_get_patterns_in_use(uint8_t* in_use, int in_use_size)
{
	memset(in_use, FALSE, in_use_size);

	int ptn_count = 0;

	for (int pos = 0; pos < MAX_ORDER_LEN; ++pos)
	{
		for (int ch = 0; ch < MAX_CHANNELS; ++ch)
		{
			int ptn = song->order.pos[pos].ptn[ch];

			if (ptn < in_use_size)
			{
				if (!in_use[ptn])
				{
					in_use[ptn] = TRUE;
					++ptn_count;
				}
			}
		}
	}

	return ptn_count;
}



void ebt_order_replace_pattern(int ptn_prev, int ptn_new)
{
	for (int pos = 0; pos < MAX_ORDER_LEN; ++pos)
	{
		for (int ch = 0; ch < MAX_CHANNELS; ++ch)
		{
			if (song->order.pos[pos].ptn[ch] == ptn_prev)
			{
				song->order.pos[pos].ptn[ch] = ptn_new;
			}
		}
	}
}



void ebt_patterns_replace_instrument(int ins_prev, int ins_new)
{
	for (int ptn = 0; ptn < MAX_PATTERNS; ++ptn)
	{
		pattern_struct* ps = &song->ptns[ptn];

		for (int row = 0; row < MAX_PATTERN_LEN; ++row)
		{
			if (ps->rows[row].ins == ins_prev)
			{
				ps->rows[row].ins = ins_new;
			}
		}
	}
}




void ebt_song_squeeze(void)
{
	uint8_t in_use[MAX_PATTERNS > MAX_INSTRUMENTS ? MAX_PATTERNS : MAX_INSTRUMENTS];

	//1. clear patterns that are not in the order list

	ebt_song_get_patterns_in_use(in_use, sizeof(in_use));

	for (int ptn = 0; ptn < MAX_PATTERNS; ++ptn)
	{
		if (!in_use[ptn]) ebt_song_clear_pattern(ptn);
	}

	//2. join matching patterns and replace their numbers in the order list, one by one (slow)

	for (int ptn = 1; ptn < MAX_PATTERNS; ++ptn)
	{
		if (!in_use[ptn]) continue;

		for (int pptn = ptn + 1; pptn < MAX_PATTERNS; ++pptn)
		{
			if (!in_use[pptn]) continue;

			if (memcmp(&song->ptns[ptn], &song->ptns[pptn], sizeof(pattern_struct)) == 0)
			{
				ebt_order_replace_pattern(pptn, ptn);
				ebt_song_clear_pattern(pptn);

				in_use[pptn] = FALSE;
			}
		}
	}

	//3. renumber patterns in the ascending order without gaps between the numbers

	ebt_song_get_patterns_in_use(in_use, sizeof(in_use));

	for (int ptn = 1; ptn < MAX_PATTERNS; ++ptn)
	{
		if (!in_use[ptn])
		{
			int new_ptn = -1;

			for (int pptn = ptn + 1; pptn < MAX_PATTERNS; ++pptn)
			{
				if (in_use[pptn])
				{
					new_ptn = pptn;
					break;
				}
			}

			if (new_ptn < 0) break;

			memcpy(&song->ptns[ptn], &song->ptns[new_ptn], sizeof(pattern_struct));
			ebt_song_clear_pattern(new_ptn);

			ebt_order_replace_pattern(new_ptn, ptn);

			in_use[new_ptn] = FALSE;
		}
	}

	//4. set unused instruments to a default one

	ebt_song_get_instruments_in_use(in_use, sizeof(in_use));

	for (int ins = 0; ins < MAX_INSTRUMENTS; ++ins)
	{
		if (!in_use[ins]) ebt_song_clear_instrument(ins);
	}

	//5. join matching instruments and replace their numbers in the pattern data, one by one (slow)

	for (int ins = 1; ins < MAX_INSTRUMENTS; ++ins)
	{
		if (!in_use[ins]) continue;

		for (int iins = ins + 1; iins < MAX_INSTRUMENTS; ++iins)
		{
			if (!in_use[iins]) continue;

			if (memcmp(&song->ins[ins], &song->ins[iins], sizeof(instrument_struct)) == 0)
			{
				ebt_patterns_replace_instrument(iins, ins);
				ebt_song_clear_instrument(iins);

				in_use[iins] = FALSE;
			}
		}
	}

	//6. renumber instruments in the ascending order without gaps between the numbers

	ebt_song_get_instruments_in_use(in_use, sizeof(in_use));

	for (int ins = 1; ins < MAX_INSTRUMENTS; ++ins)
	{
		if (!in_use[ins])
		{
			int new_ins = -1;

			for (int iins = ins + 1; iins < MAX_INSTRUMENTS; ++iins)
			{
				if (in_use[iins])
				{
					new_ins = iins;
					break;
				}
			}

			if (new_ins < 0) break;

			memcpy(&song->ins[ins], &song->ins[new_ins], sizeof(instrument_struct));

			ebt_song_clear_instrument(new_ins);

			ebt_patterns_replace_instrument(new_ins, ins);

			in_use[new_ins] = FALSE;
		}
	}

	//7. ????
	//8. PROFIT!!!
}