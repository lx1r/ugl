#ifndef UGL_INPUT_H
#define UGL_INPUT_H

#define KEY_ESC		1
#define KEY_1		2
#define KEY_2		3
#define KEY_3		4
#define KEY_4		5
#define KEY_5		6
#define KEY_6		7
#define KEY_7		8
#define KEY_8		9
#define KEY_9		10
#define KEY_0		11
#define KEY_MINUS	12
#define KEY_EQUAL	13
#define KEY_BACKSPACE	14
#define KEY_TAB		15
#define KEY_Q		16
#define KEY_W		17
#define KEY_E		18
#define KEY_R		19
#define KEY_T		20
#define KEY_Y		21
#define KEY_U		22
#define KEY_I		23
#define KEY_O		24
#define KEY_P		25
#define KEY_LBRACKET	26
#define KEY_RBRACKET	27
#define KEY_ENTER	28
#define KEY_CONTROL	29
#define KEY_A		30
#define KEY_S		31
#define KEY_D		32
#define KEY_F		33
#define KEY_G		34
#define KEY_H		35
#define KEY_J		36
#define KEY_K		37
#define KEY_L		38
#define KEY_SEMICOLON	39
#define KEY_APOSTROPHE	40
#define KEY_GRAVE	41
#define KEY_LSHIFT	42
#define KEY_BACKSLASH	43
#define KEY_Z		44
#define KEY_X		45
#define KEY_C		46
#define KEY_V		47
#define KEY_B		48
#define KEY_N		49
#define KEY_M		50
#define KEY_COMMA	51
#define KEY_PERIOD	52
#define KEY_SLASH	53
#define KEY_RSHIFT	54
#define KEY_NPAD_MUL	55
#define KEY_ALT		56
#define KEY_SPACE	57
#define KEY_CAPSLOCK	58
#define KEY_F1		59
#define KEY_F2		60
#define KEY_F3		61
#define KEY_F4		62
#define KEY_F5		63
#define KEY_F6		64
#define KEY_F7		65
#define KEY_F8		66
#define KEY_F9		67
#define KEY_F10		68
#define KEY_NUMLOCK	69
#define KEY_SCROLL_LOCK	70
#define KEY_NPAD7	71
#define KEY_NPAD8	72
#define KEY_NPAD9	73
#define KEY_NPAD_MINUS	74
#define KEY_NPAD4	75
#define KEY_NPAD5	76
#define KEY_NPAD6	77
#define KEY_NPAD_PLUS	78
#define KEY_NPAD1	79
#define KEY_NPAD2	80
#define KEY_NPAD3	81
#define KEY_NPAD0	82
#define KEY_NPAD_PERIOD	83
#define KEY_F11		87
#define KEY_F12		88

#define MICE_LEFT	128
#define MICE_RIGHT	129
#define MICE_MIDDLE	130
#define MICE_FOUR	131
#define MICE_FIVE	132
#define MICE_DX		256
#define MICE_DY		257
#define MICE_WHEEL	258

#define KEY_PRESSED	0
#define KEY_RELEASED	1

/* packed event (code and data) */
#define ev(code, data) (((data) << 16) | (code))
/* event code (key or axis) */
#define evcode(ev) ((ev) & 0xffff)
/* key state or axis movement */
#define evdata(ev) ((ev) >> 16)

#endif
