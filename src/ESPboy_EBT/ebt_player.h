#define PLAYER_CHANNELS		4

struct player_channel_struct {
	int16_t base_pitch;
	int16_t base_pitch_to;
	int16_t out_pitch;
	int16_t transpose_pitch;

	int16_t slide_speed;
	int16_t porta_speed;

	int16_t arp_offset[2];
	uint8_t arp_step;
	uint8_t arp_div;
	uint8_t ins;

	uint8_t pattern_num;
	uint8_t pattern_len;
	uint8_t pattern_loop;
	uint8_t pattern_row;
	uint8_t pattern_reached_end;
	uint8_t pattern_stopped;
};

struct player_struct {
	player_channel_struct chn[PLAYER_CHANNELS];

	int32_t frame_acc;
	int32_t frame_add;

	int16_t order_pos;
	int16_t order_pos_in_play;

	uint8_t speed_row;
	uint8_t speed_frame;
	uint8_t speed[2];
	uint8_t speed_interleave;
	int8_t row_in_play;

	uint8_t mode;
	uint8_t active;
};

player_struct player;

#define PLAYER_PAN_LEFT		0x01
#define PLAYER_PAN_RIGHT	0x02

#define EFF_ARP				0x00
#define EFF_SLIDE_UP		0x01
#define EFF_SLIDE_DOWN		0x02
#define EFF_PORTA			0x03
#define EFF_PHASE			0x07
#define EFF_PAN				0x08
#define EFF_WAVE			0x09
#define EFF_VOLUME			0x0c
#define EFF_EXTRA			0x0e
#define EFF_EXTRA_ARP_SPEED	0x00
#define EFF_SPEED			0x0f

enum {
	PLAYER_MODE_ORDER,
	PLAYER_MODE_SONG,
	PLAYER_MODE_PATTERN_ROW,
	PLAYER_MODE_PATTERN_TEST,
	PLAYER_MODE_INSTRUMENT_TEST
};



int16_t ebt_player_report_order_position(void)
{
	if (!player.active) return -1;

	return player.order_pos_in_play;
}



int16_t ebt_player_report_row_position(void)
{
	if (!player.active) return -1;

	return player.row_in_play;
}



void ebt_player_set_ins(player_channel_struct* pcs, uint8_t ins)
{
	if (ins >= MAX_INSTRUMENTS) ins = MAX_INSTRUMENTS - 1;	//limit the number in case less than 256 are enabled

	pcs->ins = ins;
}



void ebt_player_begin_note(uint8_t ch, uint8_t octave, uint8_t note)
{
	player_channel_struct* pcs = &player.chn[ch]; 
	instrument_struct* is = &song->ins[pcs->ins];
	
	if (is->fixed_pitch)
	{
		octave = is->base_note / 12;
		note = is->base_note % 12;
	}

	pcs->base_pitch_to = ebt_synth_octave_note_to_pitch16(octave + is->octave, note, is->detune);

	if (pcs->base_pitch_to < 0) pcs->base_pitch_to = 0;

	ebt_synth_start(ch, is->wave, is->volume, is->slide, is->cut_time, is->mod_delay, is->mod_speed, is->mod_depth, is->fixed_pitch, is->base_note);
}



void ebt_player_stop_note(uint8_t ch)
{
	ebt_synth_stop(ch);
}



void ebt_player_init(int32_t sample_rate)
{
	ebt_synth_init(sample_rate); 
	
	memset(&player, 0, sizeof(player));

	player.frame_acc = 0;
	player.frame_add = 0x10000 * FRAME_RATE / sample_rate;
}



void ebt_player_reset(void)
{
	player.active = FALSE;

	ebt_synth_reset();

	for (int ch = 0; ch < PLAYER_CHANNELS; ++ch)
	{
		player_channel_struct* pcs = &player.chn[ch];

		if (song->pan_default[ch] < 0)
		{
			ebt_synth_set_pan(ch, abs(song->pan_default[ch]) << 4);
		}
		else
		{
			ebt_synth_set_pan(ch, song->pan_default[ch]);
		}

		pcs->ins = 1;
		pcs->slide_speed = 0;
		pcs->porta_speed = 0;
		pcs->pattern_row = 0;
		pcs->pattern_reached_end = TRUE;
		pcs->pattern_stopped = TRUE;
		pcs->base_pitch = 0x7fff;
		pcs->base_pitch_to = 0x7fff;
		pcs->transpose_pitch = 0;
		pcs->out_pitch = 0;
		pcs->arp_offset[0] = 0;
		pcs->arp_offset[1] = 0;
		pcs->arp_step = 0;
		pcs->arp_div = 1;
	}

	player.frame_acc = 0;
	player.speed_row = 0;
	player.speed_frame = 0;
	player.speed[0] = song->speed_even;
	player.speed[1] = song->speed_odd;
	player.speed_interleave = song->speed_interleave;
	player.order_pos_in_play = -1;
	player.row_in_play = -1;
}



//in the song mode player works through the order-list from the beginning
//mute state does not apply, all channels always audible

void ebt_player_start_song(void)
{
	ebt_player_reset();

	player.order_pos = 0;

	player.mode = PLAYER_MODE_SONG;
	player.active = TRUE;
}



//in the song mode player works through the order-list from a given position
//considers the mute state

void ebt_player_start_order(uint8_t order_pos)
{
	ebt_player_reset();

	player.order_pos = order_pos;

	player.mode = PLAYER_MODE_ORDER;
	player.active = TRUE;
}



//in the pattern test mode player does not use the order list (never fetches new patterns)
//keeps playing a given pattern
//mutes ch1-3

void ebt_player_start_pattern_test(uint8_t pattern, uint8_t row)
{
	ebt_player_reset();

	player.chn[0].pattern_num = pattern;
	player.chn[0].pattern_loop = TRUE;
	player.chn[0].pattern_len = song->ptns[pattern].len;
	player.chn[0].pattern_row = row;
	player.chn[0].pattern_stopped = FALSE;

	player.chn[0].ins = (uint8_t)cur_ins;	//if no instrument is set in a pattern, use current instrument

	for (int i = row; i >= 0; --i)	//seek back and see if there is an instrument number set in the pattern
	{
		if (song->ptns[pattern].rows[i].ins > 0)
		{
			player.chn[0].ins = song->ptns[pattern].rows[i].ins;
			break;
		}
	}

	player.speed_row = row;

	player.mode = PLAYER_MODE_PATTERN_TEST;
	player.active = TRUE;
}



//in the pattern row mode player does not use the order list (never fetches new patterns)
//stops after playing a single row
//mutes ch1-3

void ebt_player_start_pattern_row(uint8_t pattern, uint8_t row)
{
	ebt_player_reset();

	player.chn[0].pattern_num = pattern;
	player.chn[0].pattern_loop = FALSE;
	player.chn[0].pattern_len = song->ptns[pattern].len;
	player.chn[0].pattern_row = row;
	player.chn[0].pattern_stopped = FALSE;

	player.chn[0].ins = (uint8_t)cur_ins;	//if no instrument is set in a pattern, use current instrument

	for (int i = row; i >= 0; --i)	//seek back and see if there is an instrument number set in the pattern
	{
		if (song->ptns[pattern].rows[i].ins > 0)
		{
			player.chn[0].ins = song->ptns[pattern].rows[i].ins;
			break;
		}
	}

	player.speed_row = 0;

	if (config.ptn_sound_len > 0)
	{
		player.speed[0] = config.ptn_sound_len;	//sets duration of a row play
		player.speed[1] = config.ptn_sound_len;
		player.speed_interleave = 1;
	}

	player.mode = PLAYER_MODE_PATTERN_ROW;
	player.active = TRUE;
}



//in the pattern test mode player does not use the order list (never fetches new patterns), keeps playing a constructed pattern 0

void ebt_player_start_instrument_test(uint8_t ins)
{
	ebt_player_reset();

	memset(&song->ptns[0], 0, sizeof(pattern_struct));

	song->ptns[0].loop = FALSE;
	song->ptns[0].len = 2;
	song->ptns[0].rows[0].note = song->ins[ins].base_note / 12 * 16 + song->ins[ins].base_note % 12;
	song->ptns[0].rows[0].ins = ins;

	player.chn[0].pattern_num = 0;
	player.chn[0].pattern_loop = FALSE;
	player.chn[0].pattern_len = song->ptns[0].len;
	player.chn[0].pattern_row = 0;
	player.chn[0].pattern_stopped = FALSE;

	player.mode = PLAYER_MODE_INSTRUMENT_TEST;
	player.active = TRUE;
}



void ebt_player_stop(void)
{
	player.active = FALSE;
}



BOOL ebt_player_is_active(void)
{
	return player.active;
}



BOOL ebt_player_is_pattern_test_mode(void)
{
	return (player.mode == PLAYER_MODE_PATTERN_TEST) ? TRUE : FALSE;
}



//parse a pattern row in every channel, start/stop notes, set up effects

void ebt_player_row_fetch_and_advance(void)
{
	for (int ch = 0; ch < PLAYER_CHANNELS; ++ch)
	{
		player_channel_struct* pcs = &player.chn[ch];

		if (pcs->pattern_stopped) continue;

		pattern_row_struct* rs = &song->ptns[pcs->pattern_num].rows[pcs->pattern_row];

		if (rs->ins)
		{
			ebt_player_set_ins(pcs, rs->ins);
		}

		if (rs->note != 0)
		{
			if ((rs->note & 15) < 12)
			{
				ebt_player_begin_note(ch, rs->note >> 4, rs->note & 0x0f);
			}
			else
			{
				ebt_player_stop_note(ch);
			}
		}

		for (int e = 0; e < MAX_EFFECTS_PER_ROW; ++e)
		{
			effect_struct* es = &rs->effect[e];

			switch (es->type - 1)	//effects in the data starts from 1, 0 means no effect
			{
			case EFF_ARP:
				if (es->param > 0)
				{
					pcs->arp_offset[0] = (es->param & 0x0f) << 8;
					pcs->arp_offset[1] = (es->param >> 4) << 8;
				}
				else
				{
					pcs->arp_offset[0] = 0;
					pcs->arp_offset[1] = 0;
				}
				break;

			case EFF_SLIDE_UP:
				pcs->slide_speed = ((int16_t)es->param) * 2;
				break;
			case EFF_SLIDE_DOWN:
				pcs->slide_speed = -((int16_t)es->param) * 2;
				break;

			case EFF_PORTA:
				pcs->porta_speed = es->param * 2;
				break;

			case EFF_PHASE:
				ebt_synth_set_phase(ch, es->param);
				break;

			case EFF_PAN:
				ebt_synth_set_pan(ch, es->param);
				break;

			case EFF_WAVE:
				ebt_synth_set_wave(ch, es->param);
				break;

			case EFF_VOLUME:
				ebt_synth_set_volume(ch, es->param);
				break;

			case EFF_EXTRA:
				switch (es->param & 0xf0)
				{
				case EFF_EXTRA_ARP_SPEED:
					pcs->arp_div = es->param & 0x0f;
					if (pcs->arp_div < 1) pcs->arp_div = 1;
					break;
				}
				break;

			case EFF_SPEED:
				if (es->param & 0xf0)
				{
					player.speed[0] = es->param >> 4;
					player.speed[1] = es->param & 0x0f;
				}
				else
				{
					player.speed_interleave = es->param & 0x0f;
				}
				break;
			}
		}

		player.row_in_play = pcs->pattern_row;

		++pcs->pattern_row;

		if (pcs->pattern_row >= pcs->pattern_len)
		{
			pcs->pattern_reached_end = TRUE;

			if (pcs->pattern_loop)
			{
				pcs->pattern_row = 0;
			}
			else
			{
				pcs->pattern_stopped = TRUE;
			}
		}
	}
}



//fetch pattern number, pattern len, pattern loop, and transpose from the order list, then advance the order position

void ebt_player_order_fetch_and_advance(void)
{
	for (int ch = 0; ch < PLAYER_CHANNELS; ++ch)
	{
		player_channel_struct* pcs = &player.chn[ch];

		int trans = song->order.pos[player.order_pos].trans[ch];

		pcs->transpose_pitch = trans << 8;	//i.e. *cents

		int ptn = song->order.pos[player.order_pos].ptn[ch];

		if (!ptn) continue;

		pattern_struct* ps = &song->ptns[ptn];

		pcs->pattern_len = ps->len;
		pcs->pattern_loop = ps->loop;
		pcs->pattern_num = ptn;
		pcs->pattern_row = 0;
		pcs->pattern_reached_end = FALSE;
		pcs->pattern_stopped = FALSE;
	}

	player.speed_row = 0;

	player.order_pos_in_play = player.order_pos;

	++player.order_pos;

	if (player.order_pos >= song->order.loop_end)
	{
		player.order_pos = song->order.loop_start;
	}
}



//advance in-song frame-based effects such as slides and arps

void ebt_player_frame_advance(void)
{
	for (int ch = 0; ch < PLAYER_CHANNELS; ++ch)
	{
		player_channel_struct* pcs = &player.chn[ch];

		if (player.mode == PLAYER_MODE_ORDER)
		{
			if (((ch == 0) && (mute_state&MUTE_CH1)) ||
				((ch == 1) && (mute_state&MUTE_CH2)) ||
				((ch == 2) && (mute_state&MUTE_CH3)) ||
				((ch == 3) && (mute_state&MUTE_CH4)))
			{
				ebt_synth_stop(ch);
				continue;
			}
		}
		else if (player.mode != PLAYER_MODE_SONG)
		{
			if (ch > 0) //ch1..3 always muted in the pattern and instrument test modes
			{
				ebt_synth_stop(ch);
				continue;
			}
		}
		
		if (pcs->porta_speed != 0)
		{
			ebt_synth_clear_phase_reset(ch);	//do not reset phase when portamento is active
		}

		if ((pcs->porta_speed == 0) || (pcs->base_pitch == 0x7fff) || (pcs->base_pitch_to == 0x7fff))
		{
			pcs->base_pitch = pcs->base_pitch_to;
		}
		else
		{
			if (pcs->base_pitch < pcs->base_pitch_to)
			{
				pcs->base_pitch += pcs->porta_speed;

				if (pcs->base_pitch > pcs->base_pitch_to)
				{
					pcs->base_pitch = pcs->base_pitch_to;
				}
			}

			if (pcs->base_pitch > pcs->base_pitch_to)
			{
				pcs->base_pitch -= pcs->porta_speed;

				if (pcs->base_pitch < pcs->base_pitch_to)
				{
					pcs->base_pitch = pcs->base_pitch_to;
				}
			}
		}

		pcs->base_pitch_to += pcs->slide_speed;

		if (pcs->base_pitch_to < 0) pcs->base_pitch_to = 0;
		if (pcs->base_pitch_to > 10 * 12 * 256) pcs->base_pitch_to = 10 * 12 * 256;

		pcs->out_pitch = pcs->base_pitch + pcs->transpose_pitch;

		int arp_step = (pcs->arp_step / pcs->arp_div) % 3;

		if (arp_step == 1) pcs->out_pitch += pcs->arp_offset[0];
		if (arp_step == 2) pcs->out_pitch += pcs->arp_offset[1];

		++pcs->arp_step;

		if (pcs->arp_step >= 3 * 16) pcs->arp_step = 0;

		if (pcs->out_pitch < 0) pcs->out_pitch = 0;
		if (pcs->out_pitch > 10 * 12 * 256) pcs->out_pitch = 10 * 12 * 256;

		ebt_synth_set_pitch16(ch, pcs->out_pitch);
	}
}



//player inter-row logic runs at a typical chiptune frame rate

void ebt_player_frame_update(void)
{
	if (player.speed_frame == 0)
	{
		//check if all patterns has ended, fetch an order position and advance

		if ((player.mode == PLAYER_MODE_SONG) || (player.mode == PLAYER_MODE_ORDER))
		{
			int done_cnt = 0;

			for (int ch = 0; ch < PLAYER_CHANNELS; ++ch)
			{
				if (player.chn[ch].pattern_reached_end) ++done_cnt;
			}

			if (done_cnt == PLAYER_CHANNELS)
			{
				ebt_player_order_fetch_and_advance();
			}
		}

		//fetch and advance a row

		ebt_player_row_fetch_and_advance();

		//count frames and interleave speeds

		player.speed_frame = player.speed[(player.speed_row / player.speed_interleave) & 1];

		++player.speed_row;
	}

	if (player.speed_frame)
	{
		--player.speed_frame;

		if (player.mode == PLAYER_MODE_PATTERN_ROW)
		{
			if (!player.speed_frame)
			{
				ebt_player_stop();
			}
		}
	}

	ebt_player_frame_advance();
}



stereo_sample_struct ebt_player_render_sample(void)
{
	stereo_sample_struct output;

	output.l = 0;
	output.r = 0;

	if (!player.active) return output;

	ebt_synth_render_sample(&output);
	
	player.frame_acc += player.frame_add;

	if (player.frame_acc >= 0x10000)
	{
		ebt_player_frame_update();

		player.frame_acc -= 0x10000;
	}

	return output;
}