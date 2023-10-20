
#define PAD_FIRST_REPEAT	16
#define PAD_SECOND_REPEAT	2

struct {
	uint8_t state;
	uint8_t state_prev;
	uint8_t state_down;
	uint8_t state_up;
	uint8_t state_rep;
	uint8_t time_rep;

} Input;



void ebt_input_init(void)
{
	memset(&Input, 0, sizeof(Input));
}



void ebt_input_update(uint8_t input)
{
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
