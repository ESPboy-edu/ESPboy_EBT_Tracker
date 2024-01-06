#define H_FLAG_INS		0x01
#define H_FLAG_NOTE		0x02
#define H_FLAG_EFFECTS	0x04
#define H_FLAG_EFFECT_1	0x04
#define H_FLAG_EFFECT_2	0x08
#define H_FLAG_EFFECT_3	0x10
#define H_FLAG_LOOP		0x20
#define H_FLAG_END		0x40
#define H_FLAG_REPEAT	0x80


BOOL ebt_song_export(const char* filename)
{
	char buf[MAX_PATH + 32];
	char song_name[MAX_PATH];
	uint8_t in_use[MAX_PATTERNS > MAX_INSTRUMENTS ? MAX_PATTERNS : MAX_INSTRUMENTS];

	memset(in_use, 0, sizeof(in_use));

	if (!ebt_file_open(filename, TRUE)) return FALSE;

	strncpy(song_name, filename, sizeof(song_name) - 1);

	for (int i = strlen(song_name) - 1; i >= 0; --i)
	{
		if (song_name[i] == '.')
		{
			song_name[i] = 0;
			break;
		}
	}

	snprintf(buf, sizeof(buf), "const uint8_t prms_%s[] PROGMEM ={", song_name);
	ebt_file_put_line(buf);
	snprintf(buf, sizeof(buf), "0x%2.2x,", song->speed_even);
	ebt_file_put_line(buf);
	snprintf(buf, sizeof(buf), "0x%2.2x,", song->speed_odd);
	ebt_file_put_line(buf);
	snprintf(buf, sizeof(buf), "0x%2.2x,", song->speed_interleave);
	ebt_file_put_line(buf);
	snprintf(buf, sizeof(buf), "0x%2.2x,", song->order.loop_start);
	ebt_file_put_line(buf);
	snprintf(buf, sizeof(buf), "0x%2.2x,", song->order.loop_end);
	ebt_file_put_line(buf);

	for (int ch = 0; ch < MAX_CHANNELS; ++ch)
	{
		snprintf(buf, sizeof(buf), "0x%2.2x,", (uint8_t)song->pan_default[ch]);
		ebt_file_put_line(buf);
	}

	ebt_file_put_line("};\n\n");

	snprintf(buf, sizeof(buf), "const uint8_t ords_%s[] PROGMEM ={", song_name);
	ebt_file_put_line(buf);

	uint8_t prev_ptn[MAX_CHANNELS];
	int8_t prev_trans[MAX_CHANNELS];

	memset(prev_ptn, 0x00, sizeof(prev_ptn));
	memset(prev_trans, 0x00, sizeof(prev_trans));

	for (int pos = 0; pos < song->order.loop_end; ++pos)
	{
		uint8_t flags = 0;

		for (int ch = 0; ch < MAX_CHANNELS; ++ch)
		{
			order_pos_struct* ops = &song->order.pos[pos];

			if ((ops->ptn[ch] != prev_ptn[ch]) || (pos == song->order.loop_start))
			{
				prev_ptn[ch] = ops->ptn[ch];
				flags |= (0x01 << ch);
			}

			if ((ops->trans[ch] != prev_trans[ch]) || (pos == song->order.loop_start))
			{
				prev_trans[ch] = ops->trans[ch];
				flags |= (0x10 << ch);
			}
		}

		snprintf(buf, sizeof(buf), "0x%2.2x,", flags);

		for (int ch = 0; ch < MAX_CHANNELS; ++ch)
		{
			if (flags&(0x01 << ch)) snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "0x%2.2x,", song->order.pos[pos].ptn[ch]);
			if (flags&(0x10 << ch)) snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "0x%2.2x,", song->order.pos[pos].trans[ch]);
		}

		ebt_file_put_line(buf);
	}

	ebt_file_put_line("};\n\n");

	ebt_song_get_instruments_in_use(in_use, sizeof(in_use));

	for (int ins = 0; ins < MAX_INSTRUMENTS; ++ins)
	{
		if (!in_use[ins]) continue;

		instrument_struct* is = &song->ins[ins];

		char ins_name[MAX_INSTRUMENT_NAME_LEN + 1];
		memset(ins_name, 0, sizeof(ins_name));
		memcpy(ins_name, is->name, MAX_INSTRUMENT_NAME_LEN);

		snprintf(buf, sizeof(buf), "const uint8_t ins_%s_%i[] PROGMEM ={ //%s", song_name, ins, ins_name);
		ebt_file_put_line(buf);

		snprintf(buf, sizeof(buf), "0x%2.2x,", is->wave);
		ebt_file_put_line(buf);
		snprintf(buf, sizeof(buf), "0x%2.2x,", is->volume);
		ebt_file_put_line(buf);
		snprintf(buf, sizeof(buf), "0x%2.2x,", (uint8_t)is->offset);
		ebt_file_put_line(buf);
		snprintf(buf, sizeof(buf), "0x%2.2x,", (uint8_t)is->detune);
		ebt_file_put_line(buf);
		snprintf(buf, sizeof(buf), "0x%2.2x,", (uint8_t)is->slide);
		ebt_file_put_line(buf);
		snprintf(buf, sizeof(buf), "0x%2.2x,", is->mod_delay);
		ebt_file_put_line(buf);
		snprintf(buf, sizeof(buf), "0x%2.2x,", is->mod_speed);
		ebt_file_put_line(buf);
		snprintf(buf, sizeof(buf), "0x%2.2x,", is->mod_depth);
		ebt_file_put_line(buf);
		snprintf(buf, sizeof(buf), "0x%2.2x,", is->cut_time);
		ebt_file_put_line(buf);
		snprintf(buf, sizeof(buf), "0x%2.2x,", is->fixed_pitch);
		ebt_file_put_line(buf);
		snprintf(buf, sizeof(buf), "0x%2.2x,", is->base_note);
		ebt_file_put_line(buf);
		snprintf(buf, sizeof(buf), "0x%2.2x,", is->aux_id);
		ebt_file_put_line(buf);
		snprintf(buf, sizeof(buf), "0x%2.2x,", is->aux_mix);
		ebt_file_put_line(buf);

		ebt_file_put_line("};\n\n");
	}

	ebt_song_get_patterns_in_use(in_use, sizeof(in_use));

	for (int ptn = 0; ptn < MAX_PATTERNS; ++ptn)
	{
		if (!in_use[ptn]) continue;

		pattern_struct* ps = &song->ptns[ptn];

		snprintf(buf, sizeof(buf), "const uint8_t ptn_%s_%i[] PROGMEM ={", song_name, ptn);
		ebt_file_put_line(buf);

		pattern_row_struct prev_row;

		memset(&prev_row, 0xff, sizeof(prev_row));

		int repeat_cnt = 0;
		int prev_ins = -1;

		for (int row = 0; row < ps->len; ++row)
		{
			pattern_row_struct* rs = &ps->rows[row];

			if ((row > 0) && (row < (ps->len - 1)))
			{
				if (memcmp(&prev_row, rs, sizeof(prev_row)) == 0)
				{
					++repeat_cnt;
					continue;
				}
			}

			if (repeat_cnt > 0)
			{
				snprintf(buf, sizeof(buf), "0x%2.2x,", H_FLAG_REPEAT | (repeat_cnt & 0x7f));
				ebt_file_put_line(buf);
			}

			repeat_cnt = 0;

			buf[0] = 0;

			uint8_t flags = 0;

			if (row == (ps->len - 1))
			{
				flags |= H_FLAG_END;

				if (ps->loop) flags |= H_FLAG_LOOP;
			}

			if (rs->ins)
			{
				if (rs->ins != prev_ins)
				{
					prev_ins = rs->ins; 
					flags |= H_FLAG_INS;
				}
			}
			if (rs->note) flags |= H_FLAG_NOTE;
			
			for (int e = 0; e < MAX_EFFECTS_PER_ROW; ++e)
			{
				if (rs->effect[e].type || rs->effect[e].param)
				{
					flags |= (H_FLAG_EFFECTS << e);
				}
			}

			snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "0x%2.2x,", flags);

			if (flags&H_FLAG_INS)
			{
				snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "0x%2.2x,", rs->ins);
			}
			
			if (flags&H_FLAG_NOTE)
			{
				snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "0x%2.2x,", rs->note);
			}

			for (int e = 0; e < MAX_EFFECTS_PER_ROW; ++e)
			{
				if (flags&(H_FLAG_EFFECTS << e))
				{
					snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "0x%2.2x,0x%2.2x,", rs->effect[e].type, rs->effect[e].param);
				}
			}

			ebt_file_put_line(buf);

			memcpy(&prev_row, rs, sizeof(prev_row));
		}

		ebt_file_put_line("};\n\n");
	}

	ebt_song_get_instruments_in_use(in_use, sizeof(in_use));

	snprintf(buf, sizeof(buf), "const uint8_t* const inss_%s[] PROGMEM ={", song_name);
	ebt_file_put_line(buf);

	for (int ins = 0; ins < MAX_INSTRUMENTS; ++ins)
	{
		if (!in_use[ins]) continue;

		snprintf(buf, sizeof(buf), "ins_%s_%i,", song_name, ins);
		ebt_file_put_line(buf);
	}

	ebt_file_put_line("};\n\n");

	ebt_song_get_patterns_in_use(in_use, sizeof(in_use));

	snprintf(buf, sizeof(buf), "const uint8_t* const ptns_%s[] PROGMEM ={", song_name);
	ebt_file_put_line(buf);

	for (int ptn = 0; ptn < MAX_PATTERNS; ++ptn)
	{
		if (!in_use[ptn]) continue;

		snprintf(buf, sizeof(buf), "ptn_%s_%i,", song_name, ptn);
		ebt_file_put_line(buf);
	}

	ebt_file_put_line("};\n\n");

	snprintf(buf, sizeof(buf), "const void* song_%s[] PROGMEM ={", song_name);
	ebt_file_put_line(buf);
	snprintf(buf, sizeof(buf), "prms_%s,", song_name);
	ebt_file_put_line(buf);
	snprintf(buf, sizeof(buf), "ords_%s,", song_name);
	ebt_file_put_line(buf);
	snprintf(buf, sizeof(buf), "ptns_%s,", song_name);
	ebt_file_put_line(buf);
	snprintf(buf, sizeof(buf), "inss_%s,", song_name);
	ebt_file_put_line(buf);
	ebt_file_put_line("};\n\n");

	ebt_file_close();

	return TRUE;
}
