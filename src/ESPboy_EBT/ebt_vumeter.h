int8_t vu_meter_level[MAX_CHANNELS];



void ebt_vumeter_init(void)
{
	memset(vu_meter_level, 0, sizeof(vu_meter_level));
}



void ebt_vumeter_draw(uint8_t sx, uint8_t sy)
{
	const char vu_bar[] = {
		0,0,
		1,0,
		2,0,
		2,1,
		2,2
	};

	set_font_color(COL_TEXT_DARK);
	set_back_color(COL_BACK);

	for (int ch = 0; ch < MAX_CHANNELS; ++ch)
	{
		int level = (vu_meter_level[ch] >> 4);

		put_char(sx + ch, sy + 0, vu_bar[level * 2 + 1]);
		put_char(sx + ch, sy + 1, vu_bar[level * 2 + 0]);
	}
}



void ebt_vumeter_update(void)
{
	for (int ch = 0; ch < MAX_CHANNELS; ++ch)
	{
		int volume = ebt_synth_get_volume(ch + 0) + ebt_synth_get_volume(ch + 4);

		if (player.active&&volume > 0)
		{
			if (volume > 4) volume = 4;
			vu_meter_level[ch] = volume << 4;
		}
		else
		{
			vu_meter_level[ch] -= 16;

			if (vu_meter_level[ch] < 0) vu_meter_level[ch] = 0;
		}
	}
}