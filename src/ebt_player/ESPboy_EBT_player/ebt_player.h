#define PLAYER_CHANNELS			4
#define PLAYER_FRAME_RATE		60
#define MAX_EFFECTS_PER_ROW		2

#define H_FLAG_INS		0x01
#define H_FLAG_NOTE		0x02
#define H_FLAG_EFFECTS	0x04
#define H_FLAG_EFFECT_1	0x04
#define H_FLAG_EFFECT_2	0x08
#define H_FLAG_EFFECT_3	0x10
#define H_FLAG_LOOP		0x20
#define H_FLAG_END		0x40
#define H_FLAG_REPEAT	0x80

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

	const uint8_t* pattern_data;
	const uint8_t* pattern_data_orig;

	uint8_t pattern_num;
	uint8_t pattern_reached_end;
	uint8_t pattern_stopped;

	uint8_t repeat_cnt;
	uint8_t prev_note;
	uint8_t prev_ins;
	uint8_t prev_effect[MAX_EFFECTS_PER_ROW];
	uint8_t prev_param[MAX_EFFECTS_PER_ROW];
};

struct player_struct {
	player_channel_struct chn[PLAYER_CHANNELS];

	const uint8_t* order_list;
	const uint8_t** ins_list;
	const uint8_t** ptn_list;

	int32_t frame_acc;
	int32_t frame_add;

	uint8_t order_loop_start;
	uint8_t order_loop_end;
	int16_t order_pos;
	uint16_t order_off;
	uint16_t order_loop_start_off;

	uint8_t speed_row;
	uint8_t speed_frame;
	uint8_t speed[2];
	uint8_t speed_interleave;

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



void ebt_player_set_ins(player_channel_struct* pcs, uint8_t ins)
{
	pcs->ins = ins;
}



uint8_t ebt_player_ins_fetch(uint8_t ins, uint8_t off)
{
	const uint8_t* ins_data = (const uint8_t*)pgm_read_dword((const uint32_t*)&player.ins_list[ins]);

	return pgm_read_byte(&ins_data[off]);
}



int16_t ebt_player_octave_note_to_pitch16(uint8_t octave, uint8_t note)
{
	return ((octave * 12 + note) << 8);
}



void ebt_player_begin_note(uint8_t ch, uint8_t octave, uint8_t note)
{
	player_channel_struct* pcs = &player.chn[ch];

	uint8_t ins_wave = ebt_player_ins_fetch(pcs->ins, 0);
	uint8_t ins_volume = ebt_player_ins_fetch(pcs->ins, 1);
	int8_t ins_offset = ebt_player_ins_fetch(pcs->ins, 2);
	int8_t ins_detune = ebt_player_ins_fetch(pcs->ins, 3);
	int8_t ins_slide = ebt_player_ins_fetch(pcs->ins, 4);
	uint8_t ins_mod_delay = ebt_player_ins_fetch(pcs->ins, 5);
	uint8_t ins_mod_speed = ebt_player_ins_fetch(pcs->ins, 6);
	uint8_t ins_mod_depth = ebt_player_ins_fetch(pcs->ins, 7);
	uint8_t ins_cut_time = ebt_player_ins_fetch(pcs->ins, 8);
	uint8_t ins_fix_pitch = ebt_player_ins_fetch(pcs->ins, 9);
	uint8_t ins_base_note = ebt_player_ins_fetch(pcs->ins, 10);
	int8_t ins_aux_id = ebt_player_ins_fetch(pcs->ins, 11);
	uint8_t ins_aux_mix = ebt_player_ins_fetch(pcs->ins, 12);

	if (ins_fix_pitch)
	{
		octave = ins_base_note / 12;
		note = ins_base_note % 12;
	}

	pcs->base_pitch_to = ebt_player_octave_note_to_pitch16(octave, note);

	if (pcs->base_pitch_to < 0) pcs->base_pitch_to = 0;

	ebt_synth_start(ch + 0, ins_wave, ins_volume, ins_offset, ins_detune, ins_slide, ins_cut_time, ins_mod_delay, ins_mod_speed, ins_mod_depth, ins_fix_pitch, ins_base_note, (ins_aux_id != 0) ? ins_aux_mix : SYNTH_MIX_NONE);

	if (ins_aux_id != 0)
	{
		int ins_ref = pcs->ins + ins_aux_id;

		ins_wave = ebt_player_ins_fetch(ins_ref, 0);
		ins_volume = ebt_player_ins_fetch(ins_ref, 1);
		ins_offset = ebt_player_ins_fetch(ins_ref, 2);
		ins_detune = ebt_player_ins_fetch(ins_ref, 3);
		ins_slide = ebt_player_ins_fetch(ins_ref, 4);
		ins_mod_delay = ebt_player_ins_fetch(ins_ref, 5);
		ins_mod_speed = ebt_player_ins_fetch(ins_ref, 6);
		ins_mod_depth = ebt_player_ins_fetch(ins_ref, 7);
		ins_cut_time = ebt_player_ins_fetch(ins_ref, 8);
		ins_fix_pitch = ebt_player_ins_fetch(ins_ref, 9);
		ins_base_note = ebt_player_ins_fetch(ins_ref, 10);

		ebt_synth_start(ch + 4, ins_wave, ins_volume, ins_offset, ins_detune, ins_slide, ins_cut_time, ins_mod_delay, ins_mod_speed, ins_mod_depth, ins_fix_pitch, ins_base_note, SYNTH_MIX_NONE);
	}
}



void ebt_player_stop_note(uint8_t ch)
{
	ebt_synth_stop(ch + 0);
	ebt_synth_stop(ch + 4);
}



void ebt_player_start(const void** data, int32_t sample_rate)
{
	memset(&player, 0, sizeof(player));

	player.frame_acc = 0;
	player.frame_add = 0x10000 * PLAYER_FRAME_RATE / sample_rate;

	ebt_synth_init(sample_rate);

	const uint8_t* params = (const uint8_t*)pgm_read_dword((const uint32_t*)&data[0]);

	player.speed[0] = pgm_read_byte(&params[0]);
	player.speed[1] = pgm_read_byte(&params[1]);
	player.speed_interleave = pgm_read_byte(&params[2]);
	player.order_loop_start = pgm_read_byte(&params[3]);
	player.order_loop_end = pgm_read_byte(&params[4]);
	player.order_off = 0;
	player.order_loop_start_off = 0;

	player.order_list = (const uint8_t*)pgm_read_dword((const uint32_t*)&data[1]);
	player.ptn_list = (const uint8_t**)pgm_read_dword((const uint32_t*)&data[2]);
	player.ins_list = (const uint8_t**)pgm_read_dword((const uint32_t*)&data[3]);

	for (int ch = 0; ch < PLAYER_CHANNELS; ++ch)
	{
		player_channel_struct* pcs = &player.chn[ch];

		int8_t pan_default = pgm_read_byte(&params[5 + ch]);

		if (pan_default < 0)
		{
			ebt_synth_set_pan(ch, abs(pan_default) << 4);
		}
		else
		{
			ebt_synth_set_pan(ch, pan_default);
		}

		pcs->ins = 1;
		pcs->pattern_reached_end = TRUE;
		pcs->pattern_stopped = TRUE;
		pcs->base_pitch = 0x7fff;
		pcs->base_pitch_to = 0x7fff;
		pcs->arp_div = 1;
		pcs->prev_note = 0x0f;
	}

	player.frame_acc = 0;
	player.speed_row = 0;
	player.speed_frame = 0;

	player.order_pos = 0;

	player.active = TRUE;
}



void ebt_player_stop(void)
{
	player.active = FALSE;
}




uint8_t ebt_player_ptn_fetch_and_advance(player_channel_struct* pcs)
{
	uint8_t n = pgm_read_byte(pcs->pattern_data);

	++pcs->pattern_data;

	return n;
}



//parse a pattern row in every channel, start/stop notes, set up effects

void ebt_player_row_fetch_and_advance(void)
{
	for (int ch = 0; ch < PLAYER_CHANNELS; ++ch)
	{
		player_channel_struct* pcs = &player.chn[ch];

		if (pcs->pattern_stopped) continue;

		if (pcs->repeat_cnt > 0)
		{
			--pcs->repeat_cnt;
		}
		else
		{
			uint8_t flags = ebt_player_ptn_fetch_and_advance(pcs);

			if (flags&H_FLAG_REPEAT)
			{
				pcs->repeat_cnt = (flags & 0x7f) - 1;
				flags = 0;
			}

			if (flags&H_FLAG_INS)
			{
				pcs->prev_ins = ebt_player_ptn_fetch_and_advance(pcs);
			}
			else
			{
				pcs->prev_ins = 0;
			}

			if (flags&H_FLAG_NOTE)
			{
				pcs->prev_note = ebt_player_ptn_fetch_and_advance(pcs);
			}
			else
			{
				pcs->prev_note = 0;
			}

			for (int e = 0; e < MAX_EFFECTS_PER_ROW; ++e)
			{
				if (flags&(H_FLAG_EFFECTS << e))
				{
					pcs->prev_effect[e] = ebt_player_ptn_fetch_and_advance(pcs);
					pcs->prev_param[e] = ebt_player_ptn_fetch_and_advance(pcs);
				}
				else
				{
					pcs->prev_effect[e] = 0;
					pcs->prev_param[e] = 0;
				}
			}

			if (flags&H_FLAG_END)
			{
				pcs->pattern_reached_end = TRUE;

				if (flags&H_FLAG_LOOP)
				{
					pcs->pattern_data = pcs->pattern_data_orig;
				}
				else
				{
					pcs->pattern_stopped = TRUE;
				}
			}
		}

		if (pcs->prev_ins > 0)
		{
			ebt_player_set_ins(pcs, pcs->prev_ins);
		}

		if (pcs->prev_note > 0)
		{
			if ((pcs->prev_note & 15) < 12)
			{
				ebt_player_begin_note(ch, pcs->prev_note >> 4, pcs->prev_note & 0x0f);
			}
			else
			{
				ebt_player_stop_note(ch);
			}
		}

		for (int e = 0; e < MAX_EFFECTS_PER_ROW; ++e)
		{
			uint8_t type = pcs->prev_effect[e];
			uint8_t param = pcs->prev_param[e];

			switch (type - 1)	//effects in the data starts from 1, 0 means no effect
			{
			case EFF_ARP:
				if (param > 0)
				{
					pcs->arp_offset[0] = (param & 0x0f) << 8;
					pcs->arp_offset[1] = (param >> 4) << 8;
				}
				else
				{
					pcs->arp_offset[0] = 0;
					pcs->arp_offset[1] = 0;
				}
				break;

			case EFF_SLIDE_UP:
				pcs->slide_speed = ((int16_t)param) * 2;
				break;
			case EFF_SLIDE_DOWN:
				pcs->slide_speed = -((int16_t)param) * 2;
				break;

			case EFF_PORTA:
				pcs->porta_speed = param * 2;
				break;

			case EFF_PHASE:
				ebt_synth_set_phase(ch, param);
				break;

			case EFF_PAN:
				ebt_synth_set_pan(ch, param);
				break;

			case EFF_WAVE:
				ebt_synth_set_wave(ch, param);
				break;

			case EFF_VOLUME:
				ebt_synth_set_volume(ch, param);
				break;

			case EFF_EXTRA:
				switch (param & 0xf0)
				{
				case EFF_EXTRA_ARP_SPEED:
					pcs->arp_div = param & 0x0f;
					if (pcs->arp_div < 1) pcs->arp_div = 1;
					break;
				}
				break;

			case EFF_SPEED:
				if (param & 0xf0)
				{
					player.speed[0] = param >> 4;
					player.speed[1] = param & 0x0f;
				}
				else
				{
					player.speed_interleave = param & 0x0f;
				}
				break;
			}
		}
	}
}



//fetch pattern number, pattern len, pattern loop, and transpose from the order list, then advance the order position

void ebt_player_order_fetch_and_advance(void)
{
	if (player.order_pos == player.order_loop_start) player.order_loop_start_off = player.order_off;

	uint8_t flags = pgm_read_byte(&player.order_list[player.order_off++]);

	for (int ch = 0; ch < PLAYER_CHANNELS; ++ch)
	{
		player_channel_struct* pcs = &player.chn[ch];

		if (flags&(0x01 << ch))
		{
			pcs->pattern_num = pgm_read_byte(&player.order_list[player.order_off++]);
		}

		if (flags&(0x10 << ch))
		{
			int trans = pgm_read_byte(&player.order_list[player.order_off++]);

			pcs->transpose_pitch = trans << 8;	//i.e. *cents
		}

		if (!pcs->pattern_num) continue;

		pcs->pattern_data_orig = player.ptn_list[pcs->pattern_num];
		pcs->pattern_data = pcs->pattern_data_orig;
		pcs->pattern_reached_end = FALSE;
		pcs->pattern_stopped = FALSE;
	}

	player.speed_row = 0;

	++player.order_pos;

	if (player.order_pos >= player.order_loop_end)
	{
		player.order_pos = player.order_loop_start;
		player.order_off = player.order_loop_start_off;
	}
}



//advance in-song frame-based effects such as slides and arps

void ebt_player_frame_advance(void)
{
	for (int ch = 0; ch < PLAYER_CHANNELS; ++ch)
	{
		player_channel_struct* pcs = &player.chn[ch];

		if (pcs->porta_speed != 0)
		{
			ebt_synth_clear_phase_reset(ch + 0);	//do not reset phase when portamento is active
			ebt_synth_clear_phase_reset(ch + 4);
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

		ebt_synth_set_pitch16(ch + 0, pcs->out_pitch);
		ebt_synth_set_pitch16(ch + 4, pcs->out_pitch);
	}
}



//player inter-row logic runs at a typical chiptune frame rate

void ebt_player_frame_update(void)
{
	if (player.speed_frame == 0)
	{
		//check if all patterns has ended, fetch an order position and advance

		int done_cnt = 0;

		for (int ch = 0; ch < PLAYER_CHANNELS; ++ch)
		{
			if (player.chn[ch].pattern_reached_end) ++done_cnt;
		}

		if (done_cnt == PLAYER_CHANNELS)
		{
			ebt_player_order_fetch_and_advance();
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
