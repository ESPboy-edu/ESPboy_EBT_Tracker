
#define PAD_FIRST_REPEAT	16
#define PAD_SECOND_REPEAT	2

struct {
	uint8_t state;
	uint8_t state_prev;
	uint8_t state_down;
	uint8_t state_up;
	uint8_t state_rep;
	uint8_t time_rep;
	uint8_t kb_buf;

} Input;

enum {
	KB_NONE = 0,
	KB_F1,
	KB_F2,
	KB_F3,
	KB_F4,
	KB_F5,
	KB_F6,
	KB_F7,
	KB_F8,
	KB_F9,
	KB_F10,
	KB_0,
	KB_1,
	KB_2,
	KB_3,
	KB_4,
	KB_5,
	KB_6,
	KB_7,
	KB_8,
	KB_9,
	KB_A,
	KB_B,
	KB_C,
	KB_D,
	KB_E,
	KB_F,
	KB_G,
	KB_H,
	KB_I,
	KB_J,
	KB_K,
	KB_L,
	KB_M,
	KB_N,
	KB_O,
	KB_P,
	KB_Q,
	KB_R,
	KB_S,
	KB_T,
	KB_U,
	KB_V,
	KB_W,
	KB_X,
	KB_Y,
	KB_Z,
	KB_MINUS,
	KB_BACKSPACE,
	KB_INSERT,
	KB_DELETE,
	KB_HOME,
	KB_END,
	KB_PGUP,
	KB_PGDOWN,
	KB_TAB
};



void ebt_input_init(void)
{
	memset(&Input, 0, sizeof(Input));
}



BOOL ebt_config_get_swap_lft_rgt(void);

void ebt_input_update_pad(uint8_t input)
{
	if (ebt_config_get_swap_lft_rgt())
	{
		uint8_t temp = input;

		input &= ~(PAD_LFT | PAD_RGT);

		if (temp&PAD_LFT) input |= PAD_RGT;
		if (temp&PAD_RGT) input |= PAD_LFT;
	}

	Input.state_prev = Input.state;
	Input.state = input;
	Input.state_down = Input.state ^ Input.state_prev & Input.state;
	Input.state_up = ~Input.state ^ ~Input.state_prev & ~Input.state;
	Input.state_rep = Input.state_down;

	if (Input.state_down)
	{
		Input.time_rep = PAD_FIRST_REPEAT;
	}

	if (Input.state)
	{
		if (Input.time_rep)
		{
			--Input.time_rep;

			if (!Input.time_rep)
			{
				Input.time_rep = PAD_SECOND_REPEAT;

				Input.state_rep |= Input.state;
			}
		}
	}
	else
	{
		Input.time_rep = 0;
	}
}



uint8_t ebt_input_get_state(void)
{
	return Input.state;
}



uint8_t ebt_input_get_trigger(void)
{
	return Input.state_down;
}



uint8_t ebt_input_get_repeat(void)	//trigger with autorepeat
{
	return Input.state_rep;
}



void ebt_input_update_kb(uint8_t kb_code)
{
	Input.kb_buf = kb_code;
}



uint8_t ebt_input_get_kb(void)
{
	return Input.kb_buf;
}



int8_t ebt_input_get_kb_hex(void)
{
	uint8_t kb_code = ebt_input_get_kb();

	if ((kb_code >= KB_0) && (kb_code <= KB_9)) return kb_code - KB_0;
	if ((kb_code >= KB_A) && (kb_code <= KB_F)) return kb_code - KB_A + 10;

	return -1;
}



char ebt_input_get_kb_char(void)
{
	uint8_t kb_code = ebt_input_get_kb();

	switch (kb_code)
	{
	case KB_0: return '0';
	case KB_1: return '1';
	case KB_2: return '2';
	case KB_3: return '3';
	case KB_4: return '4';
	case KB_5: return '5';
	case KB_6: return '6';
	case KB_7: return '7';
	case KB_8: return '8';
	case KB_9: return '9';
	case KB_A: return 'A';
	case KB_B: return 'B';
	case KB_C: return 'C';
	case KB_D: return 'D';
	case KB_E: return 'E';
	case KB_F: return 'F';
	case KB_G: return 'G';
	case KB_H: return 'H';
	case KB_I: return 'I';
	case KB_J: return 'J';
	case KB_K: return 'K';
	case KB_L: return 'L';
	case KB_M: return 'M';
	case KB_N: return 'N';
	case KB_O: return 'O';
	case KB_P: return 'P';
	case KB_Q: return 'Q';
	case KB_R: return 'R';
	case KB_S: return 'S';
	case KB_T: return 'T';
	case KB_U: return 'U';
	case KB_V: return 'V';
	case KB_W: return 'W';
	case KB_X: return 'X';
	case KB_Y: return 'Y';
	case KB_Z: return 'Z';
	}

	return 0;
}