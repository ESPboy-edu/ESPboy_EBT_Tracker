#define VERSION_STR	"v1.0  201023"


#define OSD_MSG_TIMEOUT			30

#define DOUBLE_CLICK_TIMEOUT	16

#ifdef TARGET_ESPBOY
#define MAX_PATTERNS			(160+1)
#define MAX_PATTERN_LEN			32
#define MAX_ORDER_LEN			128
#define MAX_CHANNELS			4
#define MAX_INSTRUMENTS			(128+1)
#define MAX_EFFECTS_PER_ROW		2
#endif

#ifdef TARGET_SDL//absolute max for SDL build
#define MAX_PATTERNS      256
#define MAX_PATTERN_LEN     32
#define MAX_ORDER_LEN     256
#define MAX_CHANNELS      4
#define MAX_INSTRUMENTS     256
#define MAX_EFFECTS_PER_ROW   2
#endif

#define FILE_EXT_SONG			".ebt"
#define FILE_EXT_INSTRUMENT		".eti"

#define FILE_NAME_CFG			"ebt.cfg"

#define DEFAULT_SONG_SPEED		6
#define DEFAULT_PATTERN_LEN		16
#define DEFAULT_PATTERN_LOOP	0
#define DEFAULT_VOLUME			4
#define DEFAULT_BASE_NOTE		3 * 12

#define COL_BACK				0x00
#define COL_BACK_ROW			0x01
#define COL_BACK_BEAT			0x02
#define COL_BACK_SEL			0x03
#define COL_BACK_WARN			0x04
#define COL_HEAD_PTN			0x05
#define COL_HEAD_INS			0x06
#define COL_HEAD_INFO			0x07
#define COL_HEAD_SONG			0x08
#define COL_HEAD_ORD			0x09
#define COL_HEAD_FILE			0x0a
#define COL_HEAD_NAME			0x0b
#define COL_BACK_CUR_1			0x0c
#define COL_BACK_CUR_2			0x0d
#define COL_TEXT_DARK			0x0e
#define COL_TEXT				0x0f

#define COL_FADE				COL_BACK_ROW
#define COL_BACK_OSD			COL_BACK_SEL
#define COL_HEAD_CONF			COL_HEAD_INFO


enum {
	EDIT_MODE_INFO = 0,
	EDIT_MODE_SONG,
	EDIT_MODE_ORDER,
	EDIT_MODE_PATTERN,
	EDIT_MODE_INSTRUMENT,
	EDIT_MODE_CONFIG,
	EDIT_MODE_FILE_BROWSER,
	EDIT_MODE_INPUT_NAME
};

uint8_t edit_mode;

struct effect_struct {
	uint8_t type;	//effect type stored as 0 for no effect, 1 for effect that is displayed as 0 (arpeggio) and so on
	uint8_t param;
};

struct pattern_row_struct {
	uint8_t note;
	uint8_t ins;
	effect_struct effect[MAX_EFFECTS_PER_ROW];
};

struct pattern_struct {
	pattern_row_struct rows[MAX_PATTERN_LEN];
	uint8_t len;
	uint8_t loop;
};

struct order_pos_struct {
	uint8_t ptn[MAX_CHANNELS];
	signed char trans[MAX_CHANNELS];
};

struct order_struct {
	order_pos_struct pos[MAX_ORDER_LEN];
	int16_t loop_start;
	int16_t loop_end;
};

struct instrument_struct {
	uint8_t wave;				//0..31
	uint8_t volume;			//1..4
	int8_t octave;				//-8..8
	int8_t detune;				//-100..100
	int8_t slide;				//-127..127
	uint8_t mod_delay;	//0..255
	uint8_t mod_speed;	//0..255
	uint8_t mod_depth;	//0..255
	uint8_t cut_time;			//0..255
	uint8_t fixed_pitch;		//0..1
	uint8_t base_note;		//0..96
};

struct song_struct {
	pattern_struct ptns[MAX_PATTERNS];	//actual patterns are 01..FF (as seen in the list), pattern 0 is only used to test a single pattern
	instrument_struct ins[MAX_INSTRUMENTS];	//actual instruments are 01..FF (as seen in the pattern), instrument 0 used to store default a instrument configuration
	order_struct order;
	uint8_t speed_even;
	uint8_t speed_odd;
	uint8_t speed_interleave;
};

song_struct* song = NULL;

instrument_struct* ins_default = NULL;

instrument_struct clipboard_ins;
BOOL clipboard_ins_empty = TRUE;

pattern_struct clipboard_pattern;
BOOL clipboard_pattern_empty = TRUE;

enum {
	ORDER_CLIPBOARD_NONE = 0,
	ORDER_CLIPBOARD_PATTERNS,
	ORDER_CLIPBOARD_TRANS
};

uint8_t clipboard_order[4 * MAX_ORDER_LEN];
signed char clipboard_order_w = 0;
short int clipboard_order_h = 0;
uint8_t clipboard_order_type = ORDER_CLIPBOARD_NONE;



void ebt_player_init(int sample_rate);
void ebt_player_start_song(void);
void ebt_player_start_order(uint8_t order_pos);
void ebt_player_start_pattern_test(uint8_t pattern, uint8_t row);
void ebt_player_start_instrument_test(uint8_t ins);
void ebt_player_stop(void);
BOOL ebt_player_is_active(void);
BOOL ebt_player_is_pattern_test_mode(void);

void ebt_edit_song_init(void);
void ebt_edit_info_init(void);
void ebt_edit_config_init(void);
void ebt_edit_pattern_init(void);
void ebt_edit_order_init(void);
void ebt_edit_instrument_init(void);
void ebt_file_browser_init(void);
void ebt_input_name_init(void);

void ebt_edit_song_draw(void);
void ebt_edit_info_draw(void);
void ebt_edit_config_draw(void);
void ebt_edit_pattern_draw(void);
void ebt_edit_order_draw(void);
void ebt_edit_instrument_draw(void);
void ebt_file_browser_draw(void);
void ebt_input_name_draw(void);

void ebt_edit_song_update(void);
void ebt_edit_info_update(void);
void ebt_edit_config_update(void);
void ebt_edit_pattern_update(void);
void ebt_edit_order_update(void);
void ebt_edit_instrument_update(void);
void ebt_file_browser_update(void);
void ebt_input_name_update(void);
BOOL ebt_confirm_update(void);

void song_clear(BOOL clear_instruments);
uint8_t ebt_config_blink_mask(void);
void ebt_config_set_default(void);
void ebt_config_load(void);
void ebt_item_color(BOOL active);
void ebt_order_selection_reset();

void ebt_ask_confirm(const char* msg, void(*cb_yes)(void), void(*cb_no)(void));


int16_t cur_ptn_num = 0;
int8_t cur_ptn_row = 0;
int8_t cur_ptn_col = 0;
int16_t cur_ord_pos = 0;
int8_t cur_ord_chn = 0;
uint8_t cur_ord_prev_ptn = 0;
int8_t cur_ord_prev_trans = 0;
uint8_t cur_ord_same_pos = 0;
uint8_t cur_ptn_prev_note = 0;
uint8_t cur_ptn_prev_ins = 0;
uint8_t cur_ptn_prev_effect = 1;
uint8_t cur_ptn_prev_param = 1;
int16_t cur_ins = 1;
int8_t cur_eff_column = 0;

uint8_t order_trans_mode = 0;
uint8_t order_sel_active = FALSE;
int8_t order_sel_from_chn = 0;
int8_t order_sel_to_chn = 0;
int16_t order_sel_from_pos = 0;
int16_t order_sel_to_pos = 0;
int8_t order_sel_start_chn = 0;
int16_t order_sel_start_pos = 0;

uint8_t mute_state = 0;

#define MUTE_CH1	0x01
#define MUTE_CH2	0x02
#define MUTE_CH3	0x04
#define MUTE_CH4	0x08


const char* osd_message_text = NULL;
signed char osd_message_time = 0;

uint8_t ebt_frame_cnt = 0;
uint8_t ebt_blink_cnt = 0;

const char* note_names[12] = { "C-","C#","D-","D#","E-","F-","F#","G-","G#","A-","A#","B-" };

BOOL ebt_active = FALSE;

uint8_t navi_active = FALSE;



BOOL ebt_change_param_u8(uint8_t* pparam_u8, uint8_t pad, uint8_t def, uint8_t min, uint8_t max, uint8_t large_step)
{
	int add = 0;
	int param = *pparam_u8;

	if (pad&PAD_ESC)
	{
		param = def;
	}
	else
	{
		if (pad&PAD_LEFT) add = -1;
		if (pad&PAD_RIGHT) add = 1;
		if (pad&PAD_UP) add = large_step;
		if (pad&PAD_DOWN) add = -large_step;

		param += add;

		if (param < min) param = max;
		if (param > max) param = min;
	}

	if (*pparam_u8 != param)
	{
		*pparam_u8 = param;
		return TRUE;
	}

	return FALSE;
}



BOOL ebt_change_param_i8(signed char* pparam_i8, uint8_t pad, int8_t def, int8_t min, int8_t max, int8_t large_step, uint8_t no_zero)
{
	int add = 0;
	int param = *pparam_i8;

	if (pad&PAD_ESC)
	{
		param = def;
	}
	else
	{
		if (pad&PAD_LEFT) add = -1;
		if (pad&PAD_RIGHT) add = 1;
		if (pad&PAD_UP) add = large_step;
		if (pad&PAD_DOWN) add = -large_step;

		param += add;

		if (param == 0 && no_zero) param += add;

		if (param < min) param = max;
		if (param > max) param = min;
	}

	if (*pparam_i8 != param)
	{
		*pparam_i8 = param;
		return TRUE;
	}

	return FALSE;
}



uint8_t ebt_get_frame(void)
{
	return ebt_frame_cnt;
}



uint8_t ebt_get_blink(void)
{
	return ebt_blink_cnt;
}



void ebt_item_color_custom(BOOL active, uint8_t col_active_1, uint8_t col_active_2, uint8_t col_inactive)
{
	if (active)
	{
		set_font_color(COL_TEXT);
		set_back_color(!(ebt_get_blink() & ebt_config_blink_mask() && !navi_active) ? col_active_1 : col_active_2);
	}
	else
	{
		set_font_color(COL_TEXT);
		set_back_color(col_inactive);
	}
}



void ebt_item_color(BOOL active)
{
	ebt_item_color_custom(active, COL_BACK_CUR_1, COL_BACK_CUR_2, COL_BACK);
}



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



char hex_to_char(uint8_t n)
{
	return (n < 10) ? ('0' + n) : 'A' + n - 10;
}



void osd_message_show(const char* msg)
{
	osd_message_text = msg;
	osd_message_time = 1;
}



void osd_message_set(const char* msg)
{
	osd_message_text = msg;
	osd_message_time = OSD_MSG_TIMEOUT;
}



void osd_message_clear(void)
{
	osd_message_text = NULL;
	osd_message_time = 0;
}



void set_edit_mode(int new_mode)
{
	if (edit_mode == new_mode) return;

	edit_mode = new_mode;

	switch (edit_mode)
	{
	case EDIT_MODE_INFO: ebt_edit_info_init(); break;
	case EDIT_MODE_SONG: ebt_edit_song_init(); break;
	case EDIT_MODE_ORDER: ebt_edit_order_init(); break;
	case EDIT_MODE_PATTERN: ebt_edit_pattern_init(); break;
	case EDIT_MODE_INSTRUMENT: ebt_edit_instrument_init(); break;
	case EDIT_MODE_CONFIG: ebt_edit_config_init(); break;
	case EDIT_MODE_FILE_BROWSER: ebt_file_browser_init(); break;
	case EDIT_MODE_INPUT_NAME: ebt_input_name_init(); break;
	}

	ebt_player_stop();

	osd_message_clear();
}



void ebt_init(void)
{
	ebt_frame_cnt = 0;

	ebt_config_set_default();
	ebt_config_load();

	song = new song_struct;	//needs to be allocated rather than static in the ESP8266 environment to allow free RAM for OTA and file web server

	memset(&clipboard_ins, 0, sizeof(clipboard_ins));
	clipboard_ins_empty = true;

	memset(&clipboard_pattern, 0, sizeof(clipboard_pattern));
	clipboard_pattern_empty = true;

	memset(clipboard_order, 0, sizeof(clipboard_order));
	clipboard_order_w = 0;
	clipboard_order_h = 0;
	clipboard_order_type = ORDER_CLIPBOARD_NONE;

	ebt_input_init();
	ebt_player_init(SAMPLE_RATE);

	song_clear(TRUE);

	set_edit_mode(EDIT_MODE_INFO);

	ebt_active = TRUE;
}



BOOL ebt_is_active(void)
{
	return ebt_active;
}



void ebt_stop(void)
{
	ebt_active = FALSE;
}



void ebt_shut(void)
{
	ebt_active = FALSE;

	if (song)
	{
		delete song;
		song = NULL;
	}
}



void ebt_fade_screen(void)
{
	for (int i = 0; i < TEXT_SCREEN_HGT; ++i)
	{
		for (int j = 0; j < TEXT_SCREEN_WDT; ++j)
		{
			uint8_t attr = get_attr(j, i);

			if ((attr & 0xf0) == (COL_BACK_ROW << 4)) attr &= 0x0f;

			if (attr & 0xf0) attr = COL_FADE << 4; else attr = COL_FADE;

			put_attr(j, i, attr);
		}
	}
}



void put_header(const char* text, uint8_t color)
{
	int x = strlen(text);

	set_font_color(COL_TEXT);
	set_back_color(color);
	put_str(0, 0, text);

	set_back_color(COL_BACK);
	set_font_color(color);
	put_char(x + 0, 0, 0x12);
	set_font_color(color);
	put_char(x + 1, 0, 0x13);
}



void ebt_navi_item_color(uint8_t active, uint8_t head_color)
{
	uint8_t back_color = head_color;

	if (active && !(ebt_get_blink() & ebt_config_blink_mask())) back_color = COL_BACK_CUR_2;

	set_font_color(COL_TEXT);
	set_back_color(back_color);
}



void ebt_toggle_song_play(void)
{
	if (!ebt_player_is_active())
	{
		ebt_player_start_song();
		osd_message_set("PLAY SONG");
	}
	else
	{
		ebt_player_stop();
		osd_message_set("STOP");
	}
}



void ebt_update(void)
{
	++ebt_frame_cnt;
	++ebt_blink_cnt;

	int pad = ebt_input_get_state();
	int pad_t = ebt_input_get_trigger();

	if (pad_t) ebt_blink_cnt = 0;	//blink resets on button presses to make cursor more vibisle
	if (!(pad&PAD_LFT)) if (pad) ebt_blink_cnt = 0;

	screen_clear();

	switch (edit_mode)
	{
	case EDIT_MODE_INFO: ebt_edit_info_draw(); break;
	case EDIT_MODE_SONG: ebt_edit_song_draw(); break;
	case EDIT_MODE_ORDER: ebt_edit_order_draw(); break;
	case EDIT_MODE_PATTERN: ebt_edit_pattern_draw(); break;
	case EDIT_MODE_INSTRUMENT: ebt_edit_instrument_draw(); break;
	case EDIT_MODE_CONFIG: ebt_edit_config_draw(); break;
	case EDIT_MODE_FILE_BROWSER: ebt_file_browser_draw(); break;
	case EDIT_MODE_INPUT_NAME: ebt_input_name_draw(); break;
	}

	if (osd_message_text != NULL)
	{
		if (osd_message_time > 0)
		{
			int ox = TEXT_SCREEN_WDT / 2 - strlen(osd_message_text) / 2;
			int oy = TEXT_SCREEN_HGT / 2;

			ebt_fade_screen();

			set_font_color(COL_TEXT);
			set_back_color(COL_BACK_OSD);

			for (int i = 0; i < TEXT_SCREEN_WDT; ++i) put_char(i, oy, ' ');

			put_str(ox, oy, osd_message_text);

			--osd_message_time;
		}
		else
		{
			osd_message_text = NULL;
		}
	}

	if (ebt_confirm_update()) return;

	switch (edit_mode)
	{
	case EDIT_MODE_SONG:
	case EDIT_MODE_INFO:
	{
		if (pad_t&PAD_RGT)
		{
			//play and stop

			ebt_toggle_song_play();

			return;
		}

		/*if (pad&PAD_RGT)	//mute control
		{
			if (pad_t&PAD_LEFT) mute_state ^= MUTE_CH1;
			if (pad_t&PAD_UP) mute_state ^= MUTE_CH2;
			if (pad_t&PAD_DOWN) mute_state ^= MUTE_CH3;
			if (pad_t&PAD_RIGHT) mute_state ^= MUTE_CH4;

			if (pad_t&PAD_ACT) mute_state = 0;

			if (pad_t&PAD_ESC)
			{
				mute_state = MUTE_CH1 | MUTE_CH2 | MUTE_CH3 | MUTE_CH4;

				switch (cur_ord_chn)
				{
				case 0: mute_state &= ~MUTE_CH1; break;
				case 1: mute_state &= ~MUTE_CH2; break;
				case 2: mute_state &= ~MUTE_CH3; break;
				case 3: mute_state &= ~MUTE_CH4; break;
				}
			}

			return;
		}*/
	}
	break;

	case EDIT_MODE_ORDER:
	{
		if (pad_t&PAD_RGT)
		{
			//play and stop

			if (!ebt_player_is_active())
			{
				ebt_player_start_order((uint8_t)cur_ord_pos);
				osd_message_set("PLAY POS");
			}
			else
			{
				ebt_player_stop();
				osd_message_set("STOP");
			}

			return;
		}
	}
	break;

	case EDIT_MODE_PATTERN:
	{
		if (!(pad&PAD_RGT))
		{
			if (ebt_player_is_active() && ebt_player_is_pattern_test_mode())
			{
				ebt_player_stop();
			}
		}

		if (pad_t&PAD_RGT)
		{
			ebt_player_start_pattern_test((uint8_t)cur_ptn_num, (uint8_t)cur_ptn_row);
		}

		if (pad&PAD_RGT)
		{
			osd_message_show("PLAY PTN");
			return;
		}
	}
	break;

	case EDIT_MODE_INSTRUMENT:
	{
		if (!(pad&PAD_RGT)) ebt_player_stop();

		if (pad_t&PAD_RGT)
		{
			ebt_player_start_instrument_test((uint8_t)cur_ins);
		}

		if (pad&PAD_RGT)
		{
			osd_message_show("PLAY INS");
			return;
		}
	}
	break;
	}

	if (pad_t) osd_message_clear();

	int navi = TRUE;

	if (edit_mode == EDIT_MODE_FILE_BROWSER) navi = FALSE;
	if (edit_mode == EDIT_MODE_INPUT_NAME) navi = FALSE;

	if ((pad&PAD_LFT) && navi)
	{
		navi_active = TRUE;

		ebt_fade_screen();

		set_font_color(COL_TEXT_DARK);
		set_back_color(COL_BACK);

		put_str(6, 1, "NAVI");

		set_font_color(COL_TEXT);

		ebt_navi_item_color((edit_mode == EDIT_MODE_INFO), COL_HEAD_INFO);
		put_str(6, 3, "INFO");

		ebt_navi_item_color((edit_mode == EDIT_MODE_SONG), COL_HEAD_SONG);

		put_str(6, 5, "SONG");

		ebt_navi_item_color((edit_mode == EDIT_MODE_ORDER), COL_HEAD_ORD);

		put_str(2, 7, "ORD");

		ebt_navi_item_color((edit_mode == EDIT_MODE_PATTERN), COL_HEAD_PTN);

		put_str(11, 7, "PTN");

		ebt_navi_item_color((edit_mode == EDIT_MODE_INSTRUMENT), COL_HEAD_INS);

		put_str(6, 9, "INST");

		ebt_navi_item_color((edit_mode == EDIT_MODE_CONFIG), COL_HEAD_CONF);

		put_str(6, 11, "CONF");

		//mode-specific copy/paste functions

		switch (edit_mode)
		{
		case EDIT_MODE_INSTRUMENT:
		{
			set_back_color(COL_HEAD_FILE);
			put_str(1, 14, "ACT");
			set_back_color(COL_HEAD_NAME);
			put_str(9, 14, "ESC");
			set_back_color(COL_BACK);
			put_str(4, 14, "CPY");
			put_str(12, 14, "PST");

			if (pad_t&PAD_ACT) ebt_instrument_copy();
			if (pad_t&PAD_ESC) ebt_instrument_paste();
		}
		break;

		case EDIT_MODE_PATTERN:
		{
			set_back_color(COL_HEAD_FILE);
			put_str(1, 14, "ACT");
			set_back_color(COL_HEAD_NAME);
			put_str(9, 14, "ESC");
			set_back_color(COL_BACK);
			put_str(4, 14, "CPY");
			put_str(12, 14, "PST");

			if (pad_t&PAD_ACT) ebt_pattern_copy();
			if (pad_t&PAD_ESC) ebt_pattern_paste();
		}
		break;

		case EDIT_MODE_ORDER:
		{
			set_back_color(COL_HEAD_FILE);
			put_str(1, 14, "ACT");
			set_back_color(COL_HEAD_NAME);
			put_str(9, 14, "ESC");
			set_back_color(COL_BACK);
			put_str(4, 14, "CPY");
			put_str(12, 14, "PST");

			if (pad_t&PAD_ACT) ebt_order_copy();
			if (pad_t&PAD_ESC) ebt_order_paste();
		}
		break;
		}

		//in the UDLR order

		const uint8_t mode_matrix[] = {
			EDIT_MODE_CONFIG		,EDIT_MODE_SONG			,EDIT_MODE_ORDER		,EDIT_MODE_PATTERN,	//EDIT_MODE_TITLE
			EDIT_MODE_INFO			,EDIT_MODE_INSTRUMENT	,EDIT_MODE_ORDER		,EDIT_MODE_PATTERN,	//EDIT_MODE_SONG		
			EDIT_MODE_SONG			,EDIT_MODE_INSTRUMENT	,EDIT_MODE_ORDER		,EDIT_MODE_PATTERN,	//EDIT_MODE_ORDER
			EDIT_MODE_SONG			,EDIT_MODE_INSTRUMENT	,EDIT_MODE_ORDER		,EDIT_MODE_PATTERN,	//EDIT_MODE_PATTERN
			EDIT_MODE_SONG			,EDIT_MODE_CONFIG		,EDIT_MODE_ORDER		,EDIT_MODE_PATTERN,	//EDIT_MODE_INSTRUMENT
			EDIT_MODE_INSTRUMENT	,EDIT_MODE_INFO			,EDIT_MODE_ORDER		,EDIT_MODE_PATTERN,	//EDIT_MODE_CONFIG
			EDIT_MODE_FILE_BROWSER	,EDIT_MODE_FILE_BROWSER	,EDIT_MODE_FILE_BROWSER	,EDIT_MODE_FILE_BROWSER, //EDIT_MODE_FILE_BROWSER
			EDIT_MODE_INPUT_NAME			,EDIT_MODE_INPUT_NAME			,EDIT_MODE_INPUT_NAME			,EDIT_MODE_INPUT_NAME, //EDIT_MODE_NAME
		};

		int off = -1;

		if (pad_t&PAD_UP) off = 0;
		if (pad_t&PAD_DOWN) off = 1;
		if (pad_t&PAD_LEFT) off = 2;
		if (pad_t&PAD_RIGHT) off = 3;

		if (off >= 0) set_edit_mode(mode_matrix[edit_mode * 4 + off]);

		return;
	}
	else
	{
		navi_active = FALSE;
	}

	switch (edit_mode)
	{
	case EDIT_MODE_INFO: ebt_edit_info_update(); break;
	case EDIT_MODE_SONG: ebt_edit_song_update(); break;
	case EDIT_MODE_ORDER: ebt_edit_order_update(); break;
	case EDIT_MODE_PATTERN: ebt_edit_pattern_update(); break;
	case EDIT_MODE_INSTRUMENT: ebt_edit_instrument_update(); break;
	case EDIT_MODE_CONFIG: ebt_edit_config_update(); break;
	case EDIT_MODE_FILE_BROWSER: ebt_file_browser_update(); break;
	case EDIT_MODE_INPUT_NAME: ebt_input_name_update(); break;
	}
}
