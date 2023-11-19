#define SIGNATURE_SONG			"ebtsongv1"
#define SIGNATURE_INSTRUMENT	"ebtinsv1"



int ebt_parse_hex_char(char c)
{
	if (c >= '0'&&c <= '9') return c - '0';
	if (c >= 'a'&&c <= 'f') return c - 'a' + 10;
	if (c >= 'A'&&c <= 'F') return c - 'A' + 10;
	return 0;
}



uint8_t ebt_parse_tag(const char* line, const char* tag)
{
	return (memcmp(line, tag, strlen(tag)) == 0) ? TRUE : FALSE;
}



uint8_t ebt_parse_hex4(const char* line)
{
	return ebt_parse_hex_char(line[0]);
}



uint8_t ebt_parse_hex8(const char* line)
{
	return (ebt_parse_hex_char(line[0]) << 4) | ebt_parse_hex_char(line[1]);
}



void ebt_parse_str(const char* line, char* str, int str_size)
{
	int ptr = 0;

	while (1)
	{
		if (ptr >= str_size) break;

		char c = *line++;

		if (c < ' ') break;

		str[ptr++] = c;
	}
}



const char* ebt_make_hex8(const char* tag, uint8_t value)
{
	static char buf[8];

	snprintf(buf, sizeof(buf), "%s%2.2X", tag, value);

	return buf;
}



void ebt_instrument_parse(int ins)
{
	instrument_struct* is = &song->ins[ins];

	while (1)
	{
		const char* line = ebt_file_get_line();

		if (!line) break;
		if (ebt_parse_tag(line, "ie")) break;

		uint8_t param = ebt_parse_hex8(line + 2);

		if (ebt_parse_tag(line, "wa")) is->wave = param;
		if (ebt_parse_tag(line, "vo")) is->volume = param;
		if (ebt_parse_tag(line, "oc")) is->octave = (signed char)param;
		if (ebt_parse_tag(line, "dt")) is->detune = (signed char)param;
		if (ebt_parse_tag(line, "sl")) is->slide = (signed char)param;
		if (ebt_parse_tag(line, "md")) is->mod_delay = param;
		if (ebt_parse_tag(line, "ms")) is->mod_speed = param;
		if (ebt_parse_tag(line, "me")) is->mod_depth = param;
		if (ebt_parse_tag(line, "ct")) is->cut_time = param;
		if (ebt_parse_tag(line, "fp")) is->fixed_pitch = param;
		if (ebt_parse_tag(line, "bn")) is->base_note = param;
	}
}



BOOL ebt_instrument_load(int ins, const char* filename)
{
	if (!ebt_file_open(filename, FALSE)) return FALSE;

	const char* sig = ebt_file_get_line();

	if (!sig || (memcmp(sig, SIGNATURE_INSTRUMENT, strlen(SIGNATURE_INSTRUMENT)) != 0))
	{
		ebt_file_close();
		return FALSE;
	}

	ebt_instrument_parse(ins);

	ebt_file_close();

	return TRUE;
}



void ebt_instrument_put(int ins)
{
	instrument_struct* is = &song->ins[ins];

	ebt_file_put_line(ebt_make_hex8("wa", is->wave));
	ebt_file_put_line(ebt_make_hex8("vo", is->volume));
	ebt_file_put_line(ebt_make_hex8("oc", is->octave));
	ebt_file_put_line(ebt_make_hex8("dt", is->detune));
	ebt_file_put_line(ebt_make_hex8("sl", is->slide));
	ebt_file_put_line(ebt_make_hex8("md", is->mod_delay));
	ebt_file_put_line(ebt_make_hex8("ms", is->mod_speed));
	ebt_file_put_line(ebt_make_hex8("me", is->mod_depth));
	ebt_file_put_line(ebt_make_hex8("ct", is->cut_time));
	ebt_file_put_line(ebt_make_hex8("fp", is->fixed_pitch));
	ebt_file_put_line(ebt_make_hex8("bn", is->base_note));
	ebt_file_put_line("ie");
}



BOOL ebt_instrument_save(int ins, const char* filename)
{
	if (!ebt_file_open(filename, TRUE)) return FALSE;

	ebt_file_put_line(SIGNATURE_INSTRUMENT);

	ebt_instrument_put(ins);

	ebt_file_close();

	return TRUE;
}



BOOL ebt_song_load(const char* filename)
{
	if (!ebt_file_open(filename, FALSE)) return FALSE;

	const char* sig = ebt_file_get_line();

	if (!sig || (memcmp(sig, SIGNATURE_SONG, strlen(SIGNATURE_SONG)) != 0))
	{
		ebt_file_close();
		return FALSE;
	}

	ebt_song_clear(TRUE);

	while (1)
	{
		const char* chunk = ebt_file_get_line();

		if (!chunk) break;

		//parse main song properties

		if (ebt_parse_tag(chunk, "mn"))
		{
			while (1)
			{
				const char* line = ebt_file_get_line();

				if (!line) break;
				if (ebt_parse_tag(line, "me")) break;

				uint8_t param = ebt_parse_hex8(line + 2);

				if (ebt_parse_tag(line, "se")) song->speed_even = param;
				if (ebt_parse_tag(line, "so")) song->speed_odd = param;
				if (ebt_parse_tag(line, "si")) song->speed_interleave = param;
				if (ebt_parse_tag(line, "sn")) ebt_parse_str(line + 2, song->name, sizeof(song->name));
				if (ebt_parse_tag(line, "sa")) ebt_parse_str(line + 2, song->author, sizeof(song->author));
				if (ebt_parse_tag(line, "dp"))
				{
					for (int ch = 0; ch < MAX_CHANNELS; ++ch)
					{
						song->pan_default[ch] = (int8_t)ebt_parse_hex8(line + 2 + ch * 2);
					}
				}
			}
		}

		//parse order

		if (ebt_parse_tag(chunk, "or"))
		{
			while (1)
			{
				const char* line = ebt_file_get_line();

				if (!line) break;
				if (ebt_parse_tag(line, "oe")) break;

				if (ebt_parse_tag(line, "ls"))
				{
					song->order.loop_start = ebt_parse_hex8(line + 2);
					continue;
				}

				if (ebt_parse_tag(line, "le"))
				{
					song->order.loop_end = ebt_parse_hex8(line + 2);
					continue;
				}

				uint8_t pos = ebt_parse_hex8(line);

				for (int ch = 0; ch < MAX_CHANNELS; ++ch)
				{
					song->order.pos[pos].ptn[ch] = ebt_parse_hex8(line + 2 + ch * 4);
					song->order.pos[pos].trans[ch] = ebt_parse_hex8(line + 4 + ch * 4);
				}
			}
		}

		//parse pattern

		if (ebt_parse_tag(chunk, "pt"))
		{
			int ptn = ebt_parse_hex8(chunk + 2);

			pattern_struct* ps = &song->ptns[ptn];

			while (1)
			{
				const char* line = ebt_file_get_line();

				if (!line) break;
				if (ebt_parse_tag(line, "pe")) break;

				if (ebt_parse_tag(line, "ln"))
				{
					ps->len = ebt_parse_hex8(line + 2);
					continue;
				}

				if (ebt_parse_tag(line, "lp"))
				{
					ps->loop = ebt_parse_hex8(line + 2);
					continue;
				}

				uint8_t row = ebt_parse_hex8(line);

				pattern_row_struct* rs = &ps->rows[row];

				rs->note = ebt_parse_hex8(line + 2);
				rs->ins = ebt_parse_hex8(line + 4);

				for (int e = 0; e < MAX_EFFECTS_PER_ROW; ++e)
				{
					int ptr = 6 + e * 3;

					if (line[ptr] < ' ') break;

					if (line[ptr] == '.')
					{
						rs->effect[e].type = 0;
					}
					else
					{
						rs->effect[e].type = ebt_parse_hex4(line + ptr + 0) + 1;	
					}

					rs->effect[e].param = ebt_parse_hex8(line + ptr + 1);
				}
			}
		}

		//parse instrument

		if (ebt_parse_tag(chunk, "in"))
		{
			int ins = ebt_parse_hex8(chunk + 2);

			if (ins >= 0 && ins < MAX_INSTRUMENTS)
			{
				ebt_instrument_parse(ins);
			}
		}
	}

	ebt_file_close();

	return TRUE;
}



BOOL ebt_song_save(const char* filename)
{
	char buf[32];
	uint8_t in_use[MAX_INSTRUMENTS];

	memset(in_use, 0, sizeof(in_use));

	if (!ebt_file_open(filename, TRUE)) return FALSE;

	ebt_file_put_line(SIGNATURE_SONG);

	//main song properties

	ebt_file_put_line("mn");
	ebt_file_put_line(ebt_make_hex8("se", song->speed_even));
	ebt_file_put_line(ebt_make_hex8("so", song->speed_odd));
	ebt_file_put_line(ebt_make_hex8("si", song->speed_interleave));
	snprintf(buf, sizeof(buf), "sn%s", song->name);
	ebt_file_put_line(buf);
	snprintf(buf, sizeof(buf), "sa%s", song->author);
	ebt_file_put_line(buf);

	snprintf(buf, sizeof(buf), "dp");
	for (int ch = 0; ch < MAX_CHANNELS; ++ch)
	{
		char bufp[3];
		snprintf(bufp, sizeof(bufp), "%2.2X", (uint8_t)song->pan_default[ch]);
		strncat(buf, bufp, sizeof(buf) - 1);
	}
	ebt_file_put_line(buf);

	ebt_file_put_line("me");

	//order list section
	//only non-empty rows saved

	ebt_file_put_line("or");
	ebt_file_put_line(ebt_make_hex8("ls", (uint8_t)song->order.loop_start));
	ebt_file_put_line(ebt_make_hex8("le", (uint8_t)song->order.loop_end));

	for (int pos = 0; pos < MAX_ORDER_LEN; ++pos)
	{
		order_pos_struct* os = &song->order.pos[pos];

		int empty = TRUE;

		for (int ch = 0; ch < MAX_CHANNELS; ++ch)
		{
			if (os->ptn[ch] || os->trans[ch])
			{
				empty = FALSE;
				break;
			}
		}

		if (!empty)
		{
			snprintf(buf, sizeof(buf), "%2.2X", pos);

			for (int ch = 0; ch < MAX_CHANNELS; ++ch)
			{
				char bufp[5];

				snprintf(bufp, sizeof(bufp), "%2.2X%2.2X", os->ptn[ch], (uint8_t)os->trans[ch]);

				strncat(buf, bufp, sizeof(buf) - 1);
			}

			ebt_file_put_line(buf);
		}
	}

	ebt_file_put_line("oe");

	//patterns section
	//only save patterns that are not empty, and only save rows that are not empty
	//pattern 0 is never used

	for (int ptn = 1; ptn < MAX_PATTERNS; ++ptn)
	{
		pattern_struct* ps = &song->ptns[ptn];

		int cnt = 0;

		if (ps->len != DEFAULT_PATTERN_LEN) ++cnt;
		if (ps->loop != DEFAULT_PATTERN_LOOP) ++cnt;

		for (int row = 0; row < ps->len; ++row)
		{
			pattern_row_struct* ns = &ps->rows[row];

			for (int e = 0; e < MAX_EFFECTS_PER_ROW; ++e)
			{
				if ((ns->effect[e].type > 0) || (ns->effect[e].param > 0)) ++cnt;
			}

			if (ns->note || ns->ins) ++cnt;
			
			if (cnt) break;
		}

		if (cnt > 0)
		{
			ebt_file_put_line(ebt_make_hex8("pt", ptn));
			ebt_file_put_line(ebt_make_hex8("ln", ps->len));
			ebt_file_put_line(ebt_make_hex8("lp", ps->loop));

			for (int row = 0; row < ps->len; ++row)
			{
				pattern_row_struct* ns = &ps->rows[row];

				if (ns->ins) in_use[ns->ins] = TRUE;	//to be used later during instrument saving

				int effects_cnt = 0;

				for (int e = 0; e < MAX_EFFECTS_PER_ROW; ++e)
				{
					if ((ns->effect[e].type > 0) || (ns->effect[e].param > 0)) effects_cnt = e + 1;
				}

				if (ns->note || ns->ins || (effects_cnt > 0))
				{
					snprintf(buf, sizeof(buf), "%2.2X%2.2X%2.2X", row, ns->note, ns->ins);
					
					for (int e = 0; e < effects_cnt; ++e)
					{
						char eff[5];

						if (ns->effect[e].type == 0)
						{
							snprintf(eff, sizeof(eff), ".%2.2X", ns->effect[e].param);
						}
						else
						{
							snprintf(eff, sizeof(eff), "%1.1X%2.2X", (ns->effect[e].type - 1), ns->effect[e].param);
						}
						

						strncat(buf, eff, sizeof(buf) - 1);
					}

					ebt_file_put_line(buf);
				}
			}

			ebt_file_put_line("pe");
		}
	}

	//instruments section
	//only save instruments that are in use, or not used, but are different from the default one
	//instrument 0 is never used

	for (int ins = 1; ins < MAX_INSTRUMENTS; ++ins)
	{
		if (!in_use[ins] && memcmp(&song->ins[ins], ins_default, sizeof(instrument_struct)) == 0) continue;

		ebt_file_put_line(ebt_make_hex8("in", ins));

		ebt_instrument_put(ins);
	}

	ebt_file_close();

	return TRUE;
}
