#define SYNTH_FRAME_RATE	240	//how often internal synth updates (slide, vibrato, etc) are performed

#define SYNTH_WAVEFORMS_MAX	32

//player uses 16-bit pitch format is 8:8, msb is a note in semitones, lsb is cents (256 cents per semitone)
//synth uses 32-bit 8:16 pitches for more precise pitch slides (65536 cents per semitone)
//synth adder uses 32-bit 16:16 for more precise pitch slides again

struct player_synth_struct {
	uint32_t add;			//16:16
	uint32_t acc;			//16:16
	int32_t add_pitch;		//8:16 pitch that is matching the calculated base, only update add if it is get changed
	int32_t base_pitch;		//8:16 pitch coming from the player
	int32_t slide_pitch;	//8:16 internal slide pitch
	int32_t slide_delta;	//8:16 pitch delta
	int32_t mod_ptr;
	int32_t mod_ptr_delta;
	int32_t mod_depth;
	int32_t mod_delay;
	int16_t duration;
	uint8_t volume;
	uint8_t pan;	//bit 0 left, bit 1 right
	uint8_t wave;
	uint8_t acc_phase_reset; 
	uint8_t running;
};

struct player_channel_struct {
	player_synth_struct synth;
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
	player_channel_struct chn[MAX_CHANNELS];

	int32_t synth_mod_acc;
	int32_t synth_mod_add;

	int32_t synth_frame_acc;
	int32_t synth_frame_add;

	int32_t frame_acc;
	int32_t frame_add;

	int16_t order_pos;
	int16_t order_pos_in_play;

	int32_t sample_rate;

	uint8_t speed_row;
	uint8_t speed_frame;
	uint8_t speed[2];
	uint8_t speed_interleave;
	int8_t row_in_play;

	uint8_t mode;
	uint8_t active;
};

struct stereo_sample_struct {
	int16_t l;
	int16_t r;
};

player_struct player;

#define PLAYER_PAN_LEFT		0x01
#define PLAYER_PAN_RIGHT	0x02

const int8_t ebt_synth_sine[256] = {
	0,      3,      6,      9,      12,     15,     18,     21,
	24,     27,     30,     33,     36,     39,     42,     45,
	48,     51,     54,     57,     59,     62,     65,     67,
	70,     73,     75,     78,     80,     82,     85,     87,
	89,     91,     94,     96,     98,     100,    102,    103,
	105,    107,    108,    110,    112,    113,    114,    116,
	117,    118,    119,    120,    121,    122,    123,    123,
	124,    125,    125,    126,    126,    126,    126,    126,
	127,    126,    126,    126,    126,    126,    125,    125,
	124,    123,    123,    122,    121,    120,    119,    118,
	117,    116,    114,    113,    112,    110,    108,    107,
	105,    103,    102,    100,    98,     96,     94,     91,
	89,     87,     85,     82,     80,     78,     75,     73,
	70,     67,     65,     62,     59,     57,     54,     51,
	48,     45,     42,     39,     36,     33,     30,     27,
	24,     21,     18,     15,     12,     9,      6,      3,
	0,      -3,     -6,     -9,     -12,    -15,    -18,    -21,
	-24,    -27,    -30,    -33,    -36,    -39,    -42,    -45,
	-48,    -51,    -54,    -57,    -59,    -62,    -65,    -67,
	-70,    -73,    -75,    -78,    -80,    -82,    -85,    -87,
	-89,    -91,    -94,    -96,    -98,    -100,   -102,   -103,
	-105,   -107,   -108,   -110,   -112,   -113,   -114,   -116,
	-117,   -118,   -119,   -120,   -121,   -122,   -123,   -123,
	-124,   -125,   -125,   -126,   -126,   -126,   -126,   -126,
	-127,   -126,   -126,   -126,   -126,   -126,   -125,   -125,
	-124,   -123,   -123,   -122,   -121,   -120,   -119,   -118,
	-117,   -116,   -114,   -113,   -112,   -110,   -108,   -107,
	-105,   -103,   -102,   -100,   -98,    -96,    -94,    -91,
	-89,    -87,    -85,    -82,    -80,    -78,    -75,    -73,
	-70,    -67,    -65,    -62,    -59,    -57,    -54,    -51,
	-48,    -45,    -42,    -39,    -36,    -33,    -30,    -27,
	-24,    -21,    -18,    -15,    -12,    -9,     -6,     -3,
};

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

#define PLAYER_ACC_PRECISION	14


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



int16_t ebt_synth_octave_note_to_pitch16(uint8_t octave, uint8_t note, int8_t cent)
{
	return ((octave * 12 + note) << 8) + (cent * 2);
}



uint32_t ebt_synth_pitch32_to_add32(int32_t pitch32)
{
	int32_t freq_table[] = {
	(int32_t)(2093.0f*(1 << PLAYER_ACC_PRECISION)),
	(int32_t)(2217.4f*(1 << PLAYER_ACC_PRECISION)),
	(int32_t)(2349.2f*(1 << PLAYER_ACC_PRECISION)),
	(int32_t)(2489.0f*(1 << PLAYER_ACC_PRECISION)),
	(int32_t)(2637.0f*(1 << PLAYER_ACC_PRECISION)),
	(int32_t)(2793.8f*(1 << PLAYER_ACC_PRECISION)),
	(int32_t)(2960.0f*(1 << PLAYER_ACC_PRECISION)),
	(int32_t)(3136.0f*(1 << PLAYER_ACC_PRECISION)),
	(int32_t)(3322.4f*(1 << PLAYER_ACC_PRECISION)),
	(int32_t)(3520.0f*(1 << PLAYER_ACC_PRECISION)),
	(int32_t)(3729.2f*(1 << PLAYER_ACC_PRECISION)),
	(int32_t)(3951.0f*(1 << PLAYER_ACC_PRECISION))
	};

	int32_t octave = (pitch32 >> 16) / 12;
	int32_t note = (pitch32 >> 16) % 12;
	int32_t cent = (pitch32 >> 8) & 255;

	uint32_t div = (1 << 18) >> octave;

	if (div == 0) div = 1;

	uint32_t hz1 = freq_table[note] / div;

	++note;

	if (note >= 12)
	{
		note = 0;
		div >>= 1;
		if (div == 0) div = 1;
	}

	int32_t hz2 = freq_table[note] / div;

	int32_t hz = (hz1 + (((hz2 - hz1)*cent) >> 8));

	int32_t add = ((1 << 14) * hz) / (player.sample_rate >> 10);

	return add;
}



void ebt_synth_set_pitch16(player_synth_struct* pss, int16_t pitch16)
{
	pss->base_pitch = pitch16 << 8;
}



void ebt_synth_set_wave(player_synth_struct* pss, uint8_t wave)
{
	pss->wave = wave;
}



void ebt_synth_set_volume(player_synth_struct* pss, uint8_t volume)
{
	if (volume > 4) volume = 4;

	pss->volume = volume;
}



void ebt_synth_set_pan(player_synth_struct* pss, uint8_t pan)
{
	if (pan == 0)
	{
		pss->pan = PLAYER_PAN_LEFT | PLAYER_PAN_RIGHT;
	}
	else
	{
		pss->pan = 0;

		if (pan & 0xf0) pss->pan |= PLAYER_PAN_LEFT;
		if (pan & 0x0f) pss->pan |= PLAYER_PAN_RIGHT;
	}
}



void ebt_synth_start(player_synth_struct* pss, uint8_t ins)
{
	instrument_struct* is = &song->ins[ins];

	pss->acc_phase_reset = TRUE;
	pss->volume = is->volume;
	pss->wave = is->wave;
	pss->slide_pitch = 0;
	pss->slide_delta = is->slide << 10;
	pss->duration = is->cut_time << 1;
	pss->mod_ptr = 0;	//reset vibrato phase
	pss->mod_ptr_delta = is->mod_speed << 13;
	pss->mod_depth = is->mod_depth << 4;
	pss->mod_delay = is->mod_delay << 2;

	pss->running = TRUE;
}



void ebt_player_set_ins(player_channel_struct* pcs, uint8_t ins)
{
	if (ins >= MAX_INSTRUMENTS) ins = MAX_INSTRUMENTS - 1;	//limit the number in case less than 256 are enabled

	pcs->ins = ins;
}



void ebt_player_begin_note(player_channel_struct* pcs, uint8_t octave, uint8_t note)
{
	instrument_struct* is = &song->ins[pcs->ins];

	if (is->fixed_pitch)
	{
		octave = is->base_note / 12;
		note = is->base_note % 12;
	}

	pcs->base_pitch_to = ebt_synth_octave_note_to_pitch16(octave + is->octave, note, is->detune);

	if (pcs->base_pitch_to < 0) pcs->base_pitch_to = 0;

	ebt_synth_start(&pcs->synth, pcs->ins);
}



void ebt_synth_stop(player_synth_struct* pss)
{
	pss->running = FALSE;
}



void ebt_player_stop_note(player_channel_struct* pcs)
{
	ebt_synth_stop(&pcs->synth);
}



void ebt_player_init(int sample_rate)
{
	memset(&player, 0, sizeof(player));

	player.sample_rate = sample_rate;

	player.synth_mod_acc = 0;
	player.synth_mod_add = 0x10000 / sample_rate;

	player.synth_frame_acc = 0;
	player.synth_frame_add = 0x10000 * SYNTH_FRAME_RATE / sample_rate;

	player.frame_acc = 0;
	player.frame_add = 0x10000 * FRAME_RATE / sample_rate;

	/*for (int i = 0; i < 12 * 8 * 256; i += 256)
	{
		int pitch = i << 8;

		int div = ebt_synth_pitch32_to_add32(pitch);

		log_add("%8.8x\t%8.8x\n", pitch, div);
	}*/
}



void ebt_player_reset(void)
{
	player.active = FALSE;

	for (int ch = 0; ch < MAX_CHANNELS; ++ch)
	{
		player_channel_struct* pcs = &player.chn[ch];

		memset(&pcs->synth, 0, sizeof(player_synth_struct));

		pcs->synth.pan = PLAYER_PAN_LEFT | PLAYER_PAN_RIGHT;
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

	player.synth_mod_acc = 0;	//reset modulation phase

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
	for (int ch = 0; ch < MAX_CHANNELS; ++ch)
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
				ebt_player_begin_note(pcs, rs->note >> 4, rs->note & 0x0f);
			}
			else
			{
				ebt_player_stop_note(pcs);
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
				pcs->synth.acc = es->param << 16;
				break;

			case EFF_PAN:
				ebt_synth_set_pan(&pcs->synth, es->param);
				break;

			case EFF_WAVE:
				ebt_synth_set_wave(&pcs->synth, es->param);
				break;

			case EFF_VOLUME:
				ebt_synth_set_volume(&pcs->synth, es->param);
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
	for (int ch = 0; ch < MAX_CHANNELS; ++ch)
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
	for (int ch = 0; ch < MAX_CHANNELS; ++ch)
	{
		player_channel_struct* pcs = &player.chn[ch];

		if (player.mode == PLAYER_MODE_ORDER)
		{
			if (((ch == 0) && (mute_state&MUTE_CH1)) ||
				((ch == 1) && (mute_state&MUTE_CH2)) ||
				((ch == 2) && (mute_state&MUTE_CH3)) ||
				((ch == 3) && (mute_state&MUTE_CH4)))
			{
				pcs->synth.running = FALSE;
				continue;
			}
		}
		else if (player.mode != PLAYER_MODE_SONG)
		{
			if (ch > 0) //ch1..3 always muted in the pattern and instrument test modes
			{
				pcs->synth.running = FALSE;
				continue;
			}
		}

		if (pcs->porta_speed != 0) pcs->synth.acc_phase_reset = FALSE;	//do not reset phase when portamento is active

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

		ebt_synth_set_pitch16(&pcs->synth, pcs->out_pitch);
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

			for (int ch = 0; ch < MAX_CHANNELS; ++ch)
			{
				if (player.chn[ch].pattern_reached_end) ++done_cnt;
			}

			if (done_cnt == MAX_CHANNELS)
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



//synth update runs at a higher frame rate to handle smooth slides, vibratos, and extremely short note durations

void ebt_synth_frame_update(void)
{
	for (int ch = 0; ch < MAX_CHANNELS; ++ch)
	{
		player_synth_struct* pss = &player.chn[ch].synth;

		if (!pss->running) continue;

		int32_t new_slide_pitch = pss->slide_pitch + pss->slide_delta;

		if (new_slide_pitch < -(32767 << 8)) new_slide_pitch = -(32767 << 8);
		if (new_slide_pitch > (32767 << 8)) new_slide_pitch = (32767 << 8);

		pss->slide_pitch = new_slide_pitch;

		int32_t internal_pitch = pss->base_pitch + pss->slide_pitch;

		if (pss->duration > 0)
		{
			--pss->duration;

			if (!pss->duration)
			{
				ebt_synth_stop(pss);
			}
		}

		if (pss->mod_delay)
		{
			--pss->mod_delay;
		}
		else
		{
			internal_pitch += (ebt_synth_sine[(pss->mod_ptr >> 16) & 255] * pss->mod_depth);
			pss->mod_ptr += pss->mod_ptr_delta;
		}

		if (internal_pitch < 0) internal_pitch = 0;

		if (internal_pitch > (32767 << 8)) internal_pitch = (32767 << 8);

		//plus vibrato, plus slide

		if (pss->add_pitch != internal_pitch)
		{
			pss->add_pitch = internal_pitch;

			pss->add = ebt_synth_pitch32_to_add32(internal_pitch);
		}
	}
}



stereo_sample_struct ebt_player_render_sample(void)
{
	stereo_sample_struct output;

	output.l = 0;
	output.r = 0;

	if (!player.active) return output;

	int mod_l = (player.synth_mod_acc >> 9) & 255;
	int mod_h = (player.synth_mod_acc >> 11) & 255;

	player.synth_mod_acc += player.synth_mod_add;

	for (int ch = 0; ch < MAX_CHANNELS; ++ch)
	{
		player_synth_struct* pss = &player.chn[ch].synth;

		if (!pss->running) continue;

		if (pss->acc_phase_reset)	//only reset phase for a new one on an external request (which gets cleared while portamento is active)
		{
			pss->acc_phase_reset = FALSE;
			pss->acc = 0;
		}

		pss->acc += pss->add;

		uint8_t l = (pss->acc >> (PLAYER_ACC_PRECISION + 0)) & 0xff;
		uint8_t h = (pss->acc >> (PLAYER_ACC_PRECISION + 8));
		uint8_t a = h;

		switch (pss->wave)
		{
			//50% square
		case 0x00:
			break;

			//32% square 
		case 0x01:
			if ((a & 0x0f) >= 0x0a) a += 0x06;
			if ((a & 0xf0) >= 0xa0) a += 0x60;
			a &= h;
			break;

			//25% square
		case 0x02:
			a = ((a << 1) | (a >> 7))&h;
			break;

			//19% square
		case 0x03:
			if ((a & 0x0f) >= 0x0a) a += 0x06;
			if ((a & 0xf0) >= 0xa0) a += 0x60;
			a = (a ^ 255)&h;
			break;

			//12.5% square
		case 0x04:
			a = (a + 2) ^ h;
			a = (a >> 1) | (a << 7);
			break;

			//6.25% square
		case 0x05:
			a = (a + 1) ^ h;
			a = (a >> 1) | (a << 7);
			break;

			//duty sweep(fast)
		case 0x06:
			a = ((a + mod_l) ^ 255) | h;
			break;

			//duty sweep(slow)
		case 0x07:
			a = ((a + mod_h) ^ 255) | h;
			break;

			//duty sweep(very slow, start lo)
		case 0x08:
			a = ((a + mod_h) >> 1)&h;
			break;

			//duty sweep(very slow, start hi)
		case 0x09:
			a = (((a + mod_h) >> 1) ^ 255) ^ h;
			break;

			//duty sweep(slow) - oct
		case 0x0a:
			a += mod_h;
			a = (a << 1) | (a >> 7);
			a ^= h;
			break;

			//duty sweep(slow) + oct
		case 0x0b:
			a += mod_h;
			a ^= h;
			break;

			//duty sweep(fast) - oct
		case 0x0c:
			a += mod_l;
			a = (a >> 1) | (a << 7);
			a ^= h;
			break;

			//vowel 1
		case 0x0d:
			if ((a & 0x0f) >= 0x0a) a += 0x06;
			if ((a & 0xf0) >= 0xa0) a += 0x60;
			a = (a << 1) | (a >> 7);
			a = (a ^ 255) ^ h;
			break;

			//vowel 2
		case 0x0e:
			if ((a & 0x0f) >= 0x0a) a += 0x06;
			if ((a & 0xf0) >= 0xa0) a += 0x60;
			a = (a << 1) | (a >> 7);
			a = (a << 1) | (a >> 7);
			a = (a ^ 255) ^ h;
			break;

			//vowel 3
		case 0x0f:
			if ((a & 0x0f) >= 0x0a) a += 0x06;
			if ((a & 0xf0) >= 0xa0) a += 0x60;
			a = (a ^ 255) ^ h;
			break;

			//vowel 4
		case 0x10:
			a = (a & 2) ? h : 0;
			break;

			//vowel 5
		case 0x11:
			a = (a << 1) | (a >> 7);
			a = (a << 1) | (a >> 7);
			a ^= h;
			a = (a << 1) | (a >> 7);
			break;

			//vowel 6
		case 0x12:
			a = a & 1 ? h : 0;
			a = (a << 1) | (a >> 7);
			break;

			//rasp 1
		case 0x13:
			if ((a & 0x0f) >= 0x0a) a += 0x06;
			if ((a & 0xf0) >= 0xa0) a = h; else a = 0;
			break;

			//rasp 2
		case 0x14:
			a = a & 64 ? h : 0;
			break;

			//phat rasp
		case 0x15:
			if ((a & 0x0f) >= 0x0a) a += 0x06;
			if ((a & 0xf0) >= 0xa0) a += 0x60;
			a = (a >> 1) | (a << 7);
			a = (a >> 1) | (a << 7);
			a = (a ^ 255) | h;
			break;

			//phat 2
		case 0x16:
			if ((a & 0x0f) >= 0x0a) a += 0x06;
			if ((a & 0xf0) >= 0xa0) a += 0x60;
			a = (a >> 1) | (a << 7);
			a = (a >> 1) | (a << 7);
			a &= h;
			break;

			//phat 3
		case 0x17:
			if ((a & 0x0f) >= 0x0a) a += 0x06;
			if ((a & 0xf0) >= 0xa0) a += 0x60;
			a = (a >> 1) | (a << 7);
			a = (a >> 1) | (a << 7);
			a = (a ^ 255)&h;
			break;

			//phat 4
		case 0x18:
			if ((a & 0x0f) >= 0x0a) a += 0x06;
			if ((a & 0xf0) >= 0xa0) a += 0x60;
			a = (a >> 1) | (a << 7);
			a = (a ^ 255) & h;
			break;

			//phat 5
		case 0x19:
			if ((a & 0x0f) >= 0x0a) a += 0x06;
			if ((a & 0xf0) >= 0xa0) a += 0x60;
			a = (a >> 1) | (a << 7);
			a = (a >> 1) | (a << 7);
			a = (a ^ 255) ^ h;
			break;

			//phat 6
		case 0x1a:
			if ((a & 0x0f) >= 0x0a) a += 0x06;
			if ((a & 0xf0) >= 0xa0) a = 255; else a = 0;
			a ^= h;
			break;

			//phat 7
		case 0x1b:
			a = (a & 64) ? h : 0;
			a = (a << 1) | (a >> 7);
			break;

			//noise 1
		case 0x1c:
			h = (h << 1) | (h >> 7);
			pss->acc = (pss->acc & ~(0xff << (PLAYER_ACC_PRECISION + 8))) | (h << (PLAYER_ACC_PRECISION + 8));
			a &= h;
			break;

			//noise 2
		case 0x1d:
			h = (h << 1) | (h >> 7);
			pss->acc = (pss->acc & ~(0xff << (PLAYER_ACC_PRECISION + 8))) | (h << (PLAYER_ACC_PRECISION + 8));
			a = h & 1 ? 255 : h;
			break;

			//noise 3
		case 0x1e:
			h = (h << 1) | (h >> 7);
			pss->acc = (pss->acc & ~(0xff << (PLAYER_ACC_PRECISION + 8))) | (h << (PLAYER_ACC_PRECISION + 8));
			a ^= l;
			break;

			//noise 4
		case 0x1f:
			h = (h << 1) | (h >> 7);
			pss->acc = (pss->acc & ~(0xff << (PLAYER_ACC_PRECISION + 8))) | (h << (PLAYER_ACC_PRECISION + 8));
			a = (a | h) ^ l;
			break;
		}

		if (a & 0x10)
		{
			if (pss->pan&PLAYER_PAN_LEFT) output.l += pss->volume;
			if (pss->pan&PLAYER_PAN_RIGHT) output.r += pss->volume;
		}
	}

	player.synth_frame_acc += player.synth_frame_add;

	if (player.synth_frame_acc >= 0x10000)
	{
		ebt_synth_frame_update();

		player.synth_frame_acc -= 0x10000;
	}

	player.frame_acc += player.frame_add;

	if (player.frame_acc >= 0x10000)
	{
		ebt_player_frame_update();

		player.frame_acc -= 0x10000;
	}

	return output;
}