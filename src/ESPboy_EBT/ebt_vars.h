#define VERSION_STR	"v1.1  091123"


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
#define FILE_EXT_HEADER			".h"

#define FILE_NAME_CFG			"ebt.cfg"

#define DEFAULT_SONG_SPEED		6
#define DEFAULT_PATTERN_LEN		16
#define DEFAULT_PATTERN_LOOP	0
#define DEFAULT_VOLUME			4
#define DEFAULT_BASE_NOTE		3 * 12

#define MAX_SONG_INFO_LEN		10

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
	int8_t pan_default[MAX_CHANNELS];	//-4..0..4
	char name[MAX_SONG_INFO_LEN];
	char author[MAX_SONG_INFO_LEN];
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

void ebt_song_clear(BOOL clear_instruments);
uint8_t ebt_config_get_blink_mask(void);
void ebt_config_set_default(void);
void ebt_config_load(void);
void ebt_item_color(BOOL active);
void ebt_order_selection_reset();

void ebt_ask_confirm(const char* msg, void(*cb_yes)(void), void(*cb_no)(void));

void ebt_instrument_copy(void);
void ebt_instrument_paste(void);
void ebt_pattern_copy(void);
void ebt_pattern_paste(void);
void ebt_order_copy(void);
void ebt_order_paste(void);
