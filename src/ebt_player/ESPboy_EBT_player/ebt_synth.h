#define SYNTH_CHANNELS		8
#define SYNTH_FRAME_RATE	240	//how often internal synth updates (slide, vibrato, etc) are performed

#define SYNTH_WAVEFORMS_MAX	64

#define SYNTH_ACC_PRECISION	14

//player uses 16-bit pitch format is 8:8, msb is a note in semitones, lsb is cents (256 cents per semitone)
//synth uses 32-bit 8:16 pitches for more precise pitch slides (65536 cents per semitone)
//synth adder uses 32-bit 16:16 for more precise pitch slides again

struct synth_channel_struct {
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
	uint8_t base_note;
	uint8_t fix_pitch;
	int8_t offset;				//in semitones
	int8_t detune;
	uint8_t volume;				//original instrument volume
	uint8_t volume_l;			//0..4 multipliers calculated from pan
	uint8_t volume_r;			//0..4
	uint8_t wave;			//00..1f
	uint8_t acc_phase_reset;
	uint8_t running;		//set while the sound is produced
	uint8_t output;
	uint8_t aux_mix;
};

struct synth_struct {
	synth_channel_struct chn[SYNTH_CHANNELS];

	int32_t mod_acc;
	int32_t mod_add;
	int32_t frame_acc;
	int32_t frame_add;

	int32_t sample_rate;
};

struct stereo_sample_struct {
	int16_t l;
	int16_t r;
};

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

synth_struct synth;

enum {
	SYNTH_MIX_ADD = 0,
	SYNTH_MIX_SUB,
	SYNTH_MIX_NONE
};



void ebt_synth_reset(void)
{
	for (int ch = 0; ch < SYNTH_CHANNELS; ++ch)
	{
		memset(&synth.chn[ch], 0, sizeof(synth_channel_struct));
	}

	synth.mod_acc = 0;	//reset modulation phase
}



void ebt_synth_init(int32_t sample_rate)
{
	ebt_synth_reset();

	synth.sample_rate = sample_rate;

	synth.mod_acc = 0;
	synth.mod_add = 0x10000 / sample_rate;

	synth.frame_acc = 0;
	synth.frame_add = 0x10000 * SYNTH_FRAME_RATE / sample_rate;
}



uint32_t ebt_synth_pitch32_to_add32(int32_t pitch32)
{
	int32_t freq_table[] = {
	(int32_t)(2093.0f*(1 << SYNTH_ACC_PRECISION)),
	(int32_t)(2217.4f*(1 << SYNTH_ACC_PRECISION)),
	(int32_t)(2349.2f*(1 << SYNTH_ACC_PRECISION)),
	(int32_t)(2489.0f*(1 << SYNTH_ACC_PRECISION)),
	(int32_t)(2637.0f*(1 << SYNTH_ACC_PRECISION)),
	(int32_t)(2793.8f*(1 << SYNTH_ACC_PRECISION)),
	(int32_t)(2960.0f*(1 << SYNTH_ACC_PRECISION)),
	(int32_t)(3136.0f*(1 << SYNTH_ACC_PRECISION)),
	(int32_t)(3322.4f*(1 << SYNTH_ACC_PRECISION)),
	(int32_t)(3520.0f*(1 << SYNTH_ACC_PRECISION)),
	(int32_t)(3729.2f*(1 << SYNTH_ACC_PRECISION)),
	(int32_t)(3951.0f*(1 << SYNTH_ACC_PRECISION))
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

	int32_t add = ((1 << 14) * hz) / (synth.sample_rate >> 10);

	return add;
}



void ebt_synth_set_pitch16(uint8_t ch, int16_t pitch16)
{
	synth_channel_struct* scs = &synth.chn[ch];

	if (scs->fix_pitch)
	{
		pitch16 = scs->base_note << 8;
	}

	pitch16 += scs->offset << 8;
	pitch16 += scs->detune * 2;

	synth.chn[ch].base_pitch = pitch16 << 8;
}



void ebt_synth_set_wave(uint8_t ch, uint8_t wave)
{
	synth.chn[ch].wave = wave;
}



void ebt_synth_set_volume(uint8_t ch, uint8_t volume)
{
	if (volume > 4) volume = 4;

	synth.chn[ch].volume = volume;
}



int ebt_synth_get_volume(uint8_t ch)
{
	if (!synth.chn[ch].running) return 0;

	return synth.chn[ch].volume;
}



void ebt_synth_set_phase(uint8_t ch, uint8_t phase)
{
	synth.chn[ch].acc = phase << 16;
}



void ebt_synth_clear_phase_reset(uint8_t ch)
{
	synth.chn[ch].acc_phase_reset = FALSE;
}



void ebt_synth_set_pan(uint8_t ch, uint8_t pan)
{
	synth_channel_struct* scs = &synth.chn[ch];

	if ((pan == 0) || ((pan & 0xf0) && (pan & 0x0f)))
	{
		scs->volume_l = 4;
		scs->volume_r = 4;
	}
	else
	{
		scs->volume_l = 4 - (pan & 0x0f);
		scs->volume_r = 4 - (pan >> 4);
	}
}



void ebt_synth_start(uint8_t ch, uint8_t wave, uint8_t volume, int8_t offset, int8_t detune, int8_t slide, uint8_t cut_time, uint8_t mod_delay, uint8_t mod_speed, uint8_t mod_depth, uint8_t fix_pitch, uint8_t base_note, uint8_t aux_mix)
{
	synth_channel_struct* scs = &synth.chn[ch];

	scs->acc_phase_reset = TRUE;
	scs->volume = volume;
	scs->wave = wave;
	scs->slide_pitch = 0;
	scs->slide_delta = slide << 10;
	scs->duration = cut_time << 1;
	scs->mod_ptr = 0;	//reset vibrato phase
	scs->mod_ptr_delta = mod_speed << 13;
	scs->mod_depth = mod_depth << 4;
	scs->mod_delay = mod_delay << 2;
	scs->offset = offset;
	scs->detune = detune;
	scs->fix_pitch = fix_pitch;
	scs->base_note = base_note;
	scs->aux_mix = aux_mix;

	scs->running = TRUE;
}



void ebt_synth_stop(uint8_t ch)
{
	synth.chn[ch].running = FALSE;
}



//synth update runs at a higher frame rate to handle smooth slides, vibratos, and extremely short note durations

void ebt_synth_frame_update(void)
{
	for (int ch = 0; ch < SYNTH_CHANNELS; ++ch)
	{
		synth_channel_struct* scs = &synth.chn[ch];

		if (!scs->running) continue;

		int32_t new_slide_pitch = scs->slide_pitch + scs->slide_delta;

		if (new_slide_pitch < -(32767 << 8)) new_slide_pitch = -(32767 << 8);
		if (new_slide_pitch > (32767 << 8)) new_slide_pitch = (32767 << 8);

		scs->slide_pitch = new_slide_pitch;

		int32_t internal_pitch = scs->base_pitch + scs->slide_pitch;

		if (scs->duration > 0)
		{
			--scs->duration;

			if (!scs->duration)
			{
				ebt_synth_stop(ch);
			}
		}

		if (scs->mod_delay)
		{
			--scs->mod_delay;
		}
		else
		{
			internal_pitch += (ebt_synth_sine[(scs->mod_ptr >> 16) & 255] * scs->mod_depth);
			scs->mod_ptr += scs->mod_ptr_delta;
		}

		if (internal_pitch < 0) internal_pitch = 0;

		if (internal_pitch > (32767 << 8)) internal_pitch = (32767 << 8);

		//plus vibrato, plus slide

		if (scs->add_pitch != internal_pitch)
		{
			scs->add_pitch = internal_pitch;

			scs->add = ebt_synth_pitch32_to_add32(internal_pitch);
		}
	}
}



void ebt_synth_render_sample(stereo_sample_struct* output)
{
	uint8_t mod_l = (synth.mod_acc >> 9) & 0xff;
	uint8_t mod_h = (synth.mod_acc >> 11) & 0xff;
	
	synth.mod_acc += synth.mod_add;

	for (int ch = 0; ch < SYNTH_CHANNELS; ++ch)
	{
		synth_channel_struct* scs = &synth.chn[ch];

		if (!scs->running) continue;

		if (scs->acc_phase_reset)	//only reset phase for a new one on an external request (which gets cleared while portamento is active)
		{
			scs->acc_phase_reset = FALSE;
			scs->acc = 0;
		}

		scs->acc += scs->add;

		uint8_t l = (scs->acc >> (SYNTH_ACC_PRECISION + 0)) & 0xff;
		uint8_t h = (scs->acc >> (SYNTH_ACC_PRECISION + 8));
		uint8_t a = h;

		//waves 00..1f are mostly matching to the original wtbeep (noises are a bit different)
		//waves 20 and above are EBT-specific extras

		uint8_t insert_h = FALSE;

		switch (scs->wave)
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
			a = (((a + mod_h) ^ 255) - 1)&h;
			break;

			//duty sweep(very slow, start hi)
		case 0x09:
			a = (a + mod_h)&h;
			break;

			//duty sweep(slow) - oct
		case 0x0a:
			a += mod_h;
			a = (a << 1) | (a >> 7);
			a ^= h;
			break;

			//duty sweep(slow) + oct
		case 0x0b:
			a = a + mod_h;
			a = (a >> 1) | (a << 7);
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
			a = (a << 1) | (a >> 7);
			a = (a << 1) | (a >> 7);
			a ^= 255;
			a &= h;
			break;

			//phat 4
		case 0x18:
			if ((a & 0x0f) >= 0x0a) a += 0x06;
			if ((a & 0xf0) >= 0xa0) a += 0x60;
			a = (a << 1) | (a >> 7);
			a ^= 255;
			a &= h;
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
			insert_h = TRUE;
			a &= h;
			break;

			//noise 2
		case 0x1d:
			h = (h << 1) | (h >> 7);
			insert_h = TRUE;
			a = h & 1 ? 255 : h;
			break;

			//noise 3
		case 0x1e:
			h = (h << 1) | (h >> 7);
			insert_h = TRUE;
			a ^= l;
			break;

			//noise 4
		case 0x1f:
			h = (h << 1) | (h >> 7);
			insert_h = TRUE;
			a = (a | h) ^ l;
			break;


			//alt duty sweep(very slow, start lo)
		case 0x20:
			a = ((a + mod_h) >> 1)&h;
			break;

			//alt duty sweep(very slow, start hi)
		case 0x21:
			a = (((a + mod_h) >> 1) ^ 255) ^ h;
			break;

			//alt duty sweep(slow) + oct
		case 0x22:
			a += mod_h;
			a ^= h;
			break;

			//alt phat 1
		case 0x23:
			if ((a & 0x0f) >= 0x0a) a += 0x06;
			if ((a & 0xf0) >= 0xa0) a += 0x60;
			a = (a >> 1) | (a << 7);
			a = (a >> 1) | (a << 7);
			a = (a ^ 255)&h;
			break;

			//alt phat 2
		case 0x24:
			if ((a & 0x0f) >= 0x0a) a += 0x06;
			if ((a & 0xf0) >= 0xa0) a += 0x60;
			a = (a >> 1) | (a << 7);
			a = (a ^ 255) & h;
			break;

			//harm
		case 0x25:
			a = a ^ (a >> 1) ^ (a >> 2);
			break;

			//fpls
		case 0x26:
			a = (a ^ (a >> 1)) + mod_l;
			break;

			//fplo
		case 0x27:
			a = (a ^ (a >> 2)) + mod_l;
			break;

			//spls
		case 0x28:
			a = (a ^ (a >> 1)) + mod_h;
			break;

			//splo
		case 0x29:
			a = (a ^ (a >> 2)) + mod_h;
			break;

			//spla
		case 0x2a:
			a = (a ^ (a >> 1) ^ (a >> 2)) + mod_l;
			break;

			//hpt1
		case 0x2b:
			a = (a + (l << 3)) + mod_l;
			break;

			//hpt2
		case 0x2c:
			a = (a + (l << 4)) + mod_l;
			break;

			//drty
		case 0x2d:
			a = (a + (a >> 1) + (a >> 2)) + mod_l;
			break;

			//pns1
		case 0x2e:
			h = (h << 1) | (h >> 7);
			insert_h = TRUE;
			a = (a&h) + mod_l;
			break;

			//pns2
		case 0x2f:
			h = (h << 1) | (h >> 7);
			insert_h = TRUE;
			a = (a&h) + mod_h;
			break;

			//mtlc
		case 0x30:
			h = (h << 1) + mod_h;
			insert_h = TRUE;
			break;

			//wns1
		case 0x31:
			h ^= l;
			insert_h = TRUE;
			break;

			//wns2
		case 0x32:
			h ^= (l >> 1);
			insert_h = TRUE;
			break;

			//wns3
		case 0x33:
			h ^= (l >> 2);
			insert_h = TRUE;
			break;

			//wns4
		case 0x34:
			h ^= (l >> 3);
			insert_h = TRUE;
			break;

			//lns1
		case 0x35:
			h ^= (l >> 4);
			insert_h = TRUE;
			break;

			//lns2
		case 0x36:
			h ^= (l >> 5);
			insert_h = TRUE;
			break;

			//lns3
		case 0x37:
			h ^= (l >> 6);
			insert_h = TRUE;
			break;

			//lns4
		case 0x38:
			h ^= (l >> 7);
			insert_h = TRUE;
			break;

			//hns1
		case 0x39:
			h += l ^ h;
			insert_h = TRUE;
			break;

			//hns2
		case 0x3a:
			h -= l ^ h;
			insert_h = TRUE;
			break;

			//hns3
		case 0x3b:
			h += (l ^ h) + mod_l;
			insert_h = TRUE;
			break;

			//hns4
		case 0x3c:
			h += (l ^ h) + mod_h;
			insert_h = TRUE;
			break;

			//mod1
		case 0x3d:
			a &= ((l >> 4) + mod_l + mod_h);
			break;

			//mod2
		case 0x3e:
			a &= ((l >> 3) + mod_l ^ mod_h);
			break;

			//mod3
		case 0x3f:
			a &= ((l >> 2) + mod_l - mod_h);
			break;
		}

		if (insert_h)
		{
			scs->acc = (scs->acc & ~(0xff << (SYNTH_ACC_PRECISION + 8))) | (h << (SYNTH_ACC_PRECISION + 8));
		}

		scs->output = a & 0x10;

		if (ch < (SYNTH_CHANNELS / 2))	//output only first half of the channels, pair them with the extra four if needed
		{
			int16_t volume = 0;

			if (scs->output) volume = scs->volume;

			switch(scs->aux_mix)
			{
			case SYNTH_MIX_ADD: if (synth.chn[ch + 4].output) volume += synth.chn[ch + 4].volume; break;
			case SYNTH_MIX_SUB: if (synth.chn[ch + 4].output) volume -= synth.chn[ch + 4].volume; break;
			}

			if (volume < 0) volume = 0;
			if (volume > 4) volume = 4;

			output->l += (volume*scs->volume_l) / 4;
			output->r += (volume*scs->volume_r) / 4;
		}
	}

	synth.frame_acc += synth.frame_add;

	if (synth.frame_acc >= 0x10000)
	{
		ebt_synth_frame_update();

		synth.frame_acc -= 0x10000;
	}
}