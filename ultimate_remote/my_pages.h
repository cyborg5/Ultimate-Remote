/*
 * This file contains the data for the pages of commands that will appear on your display.
 * You will customize this data to create your own layout of IR commands. It will depend on
 * what particular TVs, cable boxes, DVD or Blu-ray devices you want to control as well as
 * commands for mouse and keyboard control of a PC or laptop.
 */

//IR codes and other includes
#include <Keyboard.h>

//These files contain the IR codes for the various devices we want to use.
#include "sa_cable_codes.h"
#include "samsung_necx_codes.h"
#include "sony_amp_codes.h"
#include "samsung_bluray_codes.h"
#include "adafruit_mini_codes.h"

//IRLib2 includes.
//Start out with the base sending library followed by the individual protocols we will use and then
//tie everything together with the combo file.
#include <IRLibSendBase.h>
#include <IRLib_P01_NEC.h>    
#include <IRLib_P02_Sony.h>
#include <IRLib_P05_Panasonic_Old.h>
#include <IRLib_P07_NECx.h>
#include <IRLib_P08_Samsung36.h>
#include <IRLib_P12_CYKM.h>
#include <IRLibCombo.h>     // After all protocols, include this

//These macros make IR definitions easier
#define MM(c,s)   {(c),CYKM_MOUSE_MOVE+(s),CYKM,0}
#define MC(c,s)   {(c),CYKM_MOUSE_CLICK+(s),CYKM,0}
#define TOG(c,s)  {(c),CYKM_TOGGLE+(s),CYKM,0}
#define KEY(c,s)  {(c),CYKM_KEY_WRITE+(s),CYKM,0}
#define KEYC(c)   {(c),CYKM_KEY_WRITE+(c),CYKM,0}
#define CTR(c,s)  {(c),CYKM_KEY_WRITE+CYKM_CONTROL+(s),CYKM,0}
#define BLU SAMSUNG36 //My Blu-ray player uses SAMSUNG36
#define SA PANASONIC_OLD //Scientific-Atlanta cable boxes use PANASONIC_OLD protocol. Saves me some typing :-)

//These are names of the pages
#define PAGE_CBL 0
#define PAGE_MSAR 1
#define PAGE_KB 2
#define PAGE_AMP 3
#define PAGE_TV 4
#define PAGE_BLU 5
#define PAGE_TEST 6
//The BLE  page isn't really a page. It shifts the device into Bluetooth switch control mode. 
//It has to be the last page in the list for everything to work right.
#define PAGE_BLE 7
           

//Each page of commands can have a variable width up to COL_MAX. Define the widths here.
const uint8_t Page_Widths[PAGE_BLE+1]= {10, 10, 14, 10, 10, 10, 3};

//An array of rows of commands each row is at most COL_MAX long but could be shorter.
const cmnd_t  MyCommands[][COL_MAX] = {
  //Page change commands: cable, mouse/arrows, keyboard, kitchen, amplifier, TV, Blu-ray, extra, Bluetooth, blank, blank
  #define ROW_PAGES     0
  {{'C',PAGE_CBL,0,SPL_CHANGE_PAGE},  {'M',PAGE_MSAR,0,SPL_CHANGE_PAGE}, {'K',PAGE_KB,0,SPL_CHANGE_PAGE}, 
   {'A',PAGE_AMP,0,SPL_CHANGE_PAGE},  {'T',PAGE_TV,0,SPL_CHANGE_PAGE},  {'B',PAGE_BLU,0,SPL_CHANGE_PAGE}, 
   {'X',PAGE_TEST,0,SPL_CHANGE_PAGE}, {MY_BLUETOOTH,PAGE_BLE,0,SPL_CHANGE_PAGE},
   {' ',0x0,0,0}, {' ',0x0,0,0}, {' ',0x0,0,0}, {' ',0x0,0,0}, {' ',0x0,0,0}, {' ',0x0,0,0}},
  //Cable-TV Play commands: Jump back, rewind, play, ff, pause, stop, live, mute, vol down, vol up
  #define ROW_CBL_PLAY  1
  {{MY_SKIP_LEFT,SA_PREV,SA,0}, {MY_REWIND,SA_REWIND,SA,0}, {MY_PLAY,SA_PLAY,SA,0}, 
   {MY_FF,SA_FF,SA,0}, {MY_PAUSE,SA_PAUSE,SA,0}, {MY_STOP,SA_STOP,SA,SPL_GOTO_DOWN}, 
   {MY_SKIP_RIGHT,SA_LIVE,SA,0}, {'0',TV_MUTE,NECX,0}, {'-',TV_VOLDN,NECX,0}, {'+',TV_VOLUP,NECX,0}},
  //Cable channel commands: ch up, ch down, info, favorites, previous, exit, list, guide, menu, record
  #define ROW_CBL_CH    2
  {{MY_CH_UP,SA_CHUP,SA,0}, {MY_CH_DOWN,SA_CHDN,SA,0}, {'?',SA_INFO,SA,0}, 
   {'f',SA_FAV,SA,0}, {'v',SA_RECALL,SA,0}, {'x',SA_EXIT,SA,0}, 
   {'L',SA_LIST,SA,0}, {'G',SA_GUIDE,SA,SPL_GOTO_RIGHT}, {'M',SA_OPTIONS,SA,SPL_GOTO_RIGHT}, 
   {'R',SA_RECORD,SA,SPL_REC}},
  //Cable arrow commands: left, right, up, down, select, pg down, pg up, A button, B button, C button 
  #define ROW_CBL_ARROW 3
  {{MY_LEFT_ARROW,SA_LEFT,SA,0}, {MY_RIGHT_ARROW,SA_RIGHT,SA,0}, {MY_UP_ARROW,SA_UP,SA,0},
   {MY_DOWN_ARROW,SA_DOWN,SA,0}, {'*',SA_SELECT,SA,0}, {MY_PAGE_DOWN,SA_PAGE_DOWN,SA,0}, 
   {MY_PAGE_UP,SA_PAGE_UP,SA,0}, {'A',SA_A, SA,SPL_GOTO_RIGHT}, {'B',SA_B,SA,SPL_GOTO_RIGHT}, {'C',SA_C,SA,0}},
  //Cable PIP commands: toggle, move, swap, ch down, ch up, day backwards, day forward, cable power, TV power, quit 
  #define ROW_CBL_PIP   4
  {{'T',SA_PIP_TGL,SA,0}, {'M',SA_PIP_MOVE,SA,0}, {'S',SA_PIP_SWAP,SA,0}, 
   {MY_CH_DOWN,SA_PIP_CHDN,SA,0}, {MY_CH_UP,SA_PIP_CHUP,SA,0}, {MY_DOUBLE_LESS_THAN,SA_PREV_DAY,SA,0},
   {MY_DOUBLE_GREATER_THAN,SA_NEXT_DAY,SA,0}, {'C',SA_POWER,SA,0}, {'T',TV_POWER,NECX,0}, {'Q',0,0,SPL_QUIT}},

  //Mouse and arrow 1: up left, up, up right, left, left click, right, down left, down, down right, right-click 
  #define ROW_MSAR_1   5
  {MM(MY_UPPER_LEFT_ARROW,CYKM_DIR_UP+CYKM_DIR_LEFT), MM(MY_UP_ARROW,CYKM_DIR_UP), 
   MM(MY_UPPER_RIGHT_ARROW,CYKM_DIR_UP+CYKM_DIR_RIGHT), MM(MY_LEFT_ARROW,CYKM_DIR_LEFT), MC('l',CYKM_LEFT_BUTTON), 
   MM(MY_RIGHT_ARROW,CYKM_DIR_RIGHT), MM(MY_LOWER_LEFT_ARROW,CYKM_DIR_DOWN+CYKM_DIR_LEFT), 
   MM(MY_DOWN_ARROW,CYKM_DIR_DOWN), MM(MY_LOWER_RIGHT_ARROW,CYKM_DIR_DOWN+CYKM_DIR_RIGHT), MC('r',CYKM_RIGHT_BUTTON)},
  //Mouse and arrow 2: left drag, right drag, middle drag, wheel down, wheel up, shift, control, alt, faster, slower 
  #define ROW_MSAR_2   6
  {TOG('L',CYKM_TOGGLE_LEFT), TOG('R',CYKM_TOGGLE_RIGHT), TOG('M',CYKM_MIDDLE_BUTTON), 
   MM('w',CYKM_WHEEL_DOWN), MM('W',CYKM_WHEEL_UP), TOG('S',CYKM_TOGGLE_SHIFT), 
   TOG('C',CYKM_TOGGLE_CONTROL), TOG('A',CYKM_TOGGLE_ALT), MM('f',CYKM_SPEED_INCREASE), MM('s',CYKM_SPEED_DECREASE)},
  //Mouse and arrow 3: home, up, page up, left, return, right, end, down, page down, Windows Key
  #define ROW_MSAR_3   7
  {KEY('h',KEY_HOME), KEY(MY_UP_ARROW,KEY_UP_ARROW), KEY(MY_PAGE_UP,KEY_PAGE_UP),
   KEY(MY_LEFT_ARROW,KEY_LEFT_ARROW), KEY(MY_RETURN,KEY_RETURN), KEY(MY_RIGHT_ARROW,KEY_RIGHT_ARROW), 
   KEY('e',KEY_END), KEY(MY_DOWN_ARROW,KEY_DOWN_ARROW), KEY(MY_PAGE_DOWN,KEY_PAGE_DOWN), TOG('W',CYKM_TOGGLE_GUI)},
  //Mouse and arrow 4: undo, redo, cut, copy, paste, escape, backspace, insert, delete, space
  #define ROW_MSAR_4   8
  {CTR('z','z'), CTR('y','y'), CTR('x','x'), CTR('c','c'), CTR('v','v'), KEY('E',KEY_ESC), 
   KEY(MY_BACKSPACE,KEY_BACKSPACE), KEY('i',KEY_INSERT), KEY(MY_DELETE,KEY_DELETE), KEY('t',KEY_TAB)},

  //Amplifier 1: power, mute, volume up, volume down, home menu, menu, FM, AM, Blu-ray, TV
  #define ROW_AMP_1 9
  {{MY_POWER_SYMBOL,AMP_POWER,SONY,0}, {'0',AMP_MUTE,SONY,0}, {'+',AMP_VOL_UP,SONY,0},
   {'-',AMP_VOL_DN,SONY,0}, {'H',AMP_HOME,SONY,0}, {'m',AMP_MENU,SONY,0},
   {'F',AMP_FM,SONY,0}, {'A',AMP_AM,SONY,0}, {'B',AMP_BD_DVD,SONY,0}, {'T',AMP_TV,SONY,0}}, 
  //Amplifier 2: left, right, up, down, select, video, USB, iPhone, info, exit
  #define ROW_AMP_2 10
  {{MY_LEFT_ARROW,AMP_LEFT,SONY,0}, {MY_RIGHT_ARROW,AMP_RIGHT,SONY,0}, {MY_UP_ARROW,AMP_UP,SONY,0},
   {MY_DOWN_ARROW,AMP_DOWN,SONY,0}, {'*',AMP_SELECT,SONY,0}, {'V',AMP_VIDEO,SONY,0},
   {'U',AMP_USB,SONY,0}, {'I',AMP_IPHONE,SONY,0}, {'?',AMP_DISPLAY,SONY,0}, {'x',AMP_RETURN,SONY,0}}, 
  //Amplifier 3: options, game, CD, cable, AFD/2ch, movie, music (tunes), empty, empty, empty
  #define ROW_AMP_3 11
  {{'o',AMP_OPTIONS,SONY,0}, {'g',AMP_GAME,SONY,0}, {'c',AMP_SACD_CD,SONY,0},
   {'C',AMP_SAT_CATV,SONY,0}, {'m',AMP_AFD_2CH,SONY,0}, {'m',AMP_MOVIE,SONY,0},
   {'t',AMP_MUSIC,SONY,0}, {' ',0x0,0,0}, {' ',0x0,0,0}, {' ',0x0,0,0}}, 

  //TV 1: power, input, ch up, ch down, vol up, vol down, info, zoom, CC
  #define ROW_TV_1 12
  {{MY_POWER_SYMBOL,TV_POWER,NECX,0}, {'i',TV_INPUT,NECX,0}, {MY_CH_UP,TV_CH_UP,NECX,0},
   {MY_CH_DOWN,TV_CH_DN,NECX,0}, {'+',TV_VOLUP,NECX,0}, {'-',TV_VOLDN,NECX,0},
   {'0',TV_MUTE,NECX,0}, {'?',TV_INFO,NECX,0}, {'z',TV_ZOOM,NECX,0}, {'c',TV_CC,NECX,0}},
  //TV 2: menu, tools, left, right, up, down, select, exit, return, 3-D
  #define ROW_TV_2 13
  {{'m',TV_MENU,NECX,0}, {'t',TV_TOOLS,NECX,0}, {MY_LEFT_ARROW,TV_LEFT,NECX,0},
   {MY_RIGHT_ARROW,TV_RIGHT,NECX,0}, {MY_UP_ARROW,TV_UP,NECX,0}, {MY_DOWN_ARROW,TV_DOWN,NECX,0},
   {'*',TV_SELECT,NECX,0}, {'x',TV_EXIT,NECX,0}, {'r',TV_RETURN,NECX,0}, {'3',TV_3D,NECX,0}},
  //TV 3: rew, play, ff, pause, stop, A,B,C,D, Media.p
  #define ROW_TV_3 14
  {{MY_REWIND,TV_REW,NECX,0}, {MY_PLAY,TV_PLAY,NECX,0}, {MY_FF,TV_FF,NECX,0},
   {MY_PAUSE,TV_PAUSE,NECX,0}, {MY_STOP,TV_STOP,NECX,SPL_GOTO_TVAR}, {'A',TV_A,NECX,0},
   {'B',TV_B,NECX,0}, {'C',TV_C,NECX,0}, {'D',TV_D,NECX,0}, {'M',TV_MEDIAP,NECX,SPL_GOTO_TVAR}},

  //Mouse and Arrows 5:  space, reset toggles, empty, empty, empty, empty, empty, empty, empty, empty
  #define ROW_MSAR_5 15
  {KEY(MY_SPACE_BAR,' '),TOG('0',CYKM_TOGGLE_RESET),{' ',0x0,0,0},{' ',0x0,0,0},{' ',0x0,0,0},
   {' ',0x0,0,0}, {' ',0x0,0,0}, {' ',0x0,0,0}, {' ',0x0,0,0}, {' ',0x0, 0, 0}},

  //Blu-ray 1: power, rewind, play, fast-forward, pause, stop, info,  mute, vol-,vol+
  #define ROW_BLU_1 16
  {{MY_POWER_SYMBOL,BR_POWER,BLU,0}, {MY_REWIND,BR_REW,BLU,0}, {MY_PLAY,BR_PLAY,BLU,0},
   {MY_FF,BR_FF,BLU,0}, {MY_PAUSE,BR_PAUSE,BLU,0}, {MY_STOP,BR_STOP,BLU,0}, 
   {'?',BR_INFO,BLU,0}, {'0',TV_MUTE,NECX,0}, {'-',TV_VOLDN,NECX,0}, {'+',TV_VOLUP,NECX,0}}, 
  //Blu-ray 2: chap-,chap+, menu, disc menu, title menu, audio, subtitle,bonus view,full-screen, eject
  #define ROW_BLU_2 17
  {{MY_SKIP_LEFT,BR_PREV_CHAP,BLU,0}, {MY_SKIP_RIGHT,BR_NEXT_CHAP,BLU,0}, {'M',BR_MENU,BLU,0},
   {'D',BR_DISK_MENU,BLU,0}, {'T',BR_TITLE_MENU,BLU,0}, {'A',BR_AUDIO,BLU,0},
   {'S',BR_SUBTITLE,BLU,0}, {'B',BR_BONUS_VIEW,BLU,0}, {'F',BR_FULL_SCREEN,BLU,0}, {MY_UP_ARROW,BR_EJECT,BLU,0}},
  //Blu-ray 3: left, right, up, down,, select, exit, A, B, C, D
  #define ROW_BLU_3 18
  {{MY_LEFT_ARROW,BR_LEFT,BLU,0}, {MY_RIGHT_ARROW,BR_RIGHT,BLU,0}, {MY_UP_ARROW,BR_UP,BLU,0},
   {MY_DOWN_ARROW,BR_DOWN,BLU,0}, {'*',BR_SELECT,BLU,0}, {'x',BR_EXIT,BLU,0},
   {'A',BR_A,BLU,0}, {'B',BR_B,BLU,0}, {'C',BR_C,BLU,0}, {'D',BR_D,BLU,0}},
  //Blu-ray 4: return, internet, tools, repeat, empty, empty, empty, amp mute, amp vol-,amp vol+
  #define ROW_BLU_4 19
  {{'r',BR_RETURN,BLU,0}, {'I',BR_INTERNET,BLU,0}, {'t',BR_TOOLS,BLU,0}, 
   {'~',BR_REPEAT,BLU,0}, {' ',0x0,0,0}, {' ',0x0,0,0}, 
   {' ',0x0,0,0},  {'0',AMP_MUTE,SONY,0}, {'+',AMP_VOL_UP,SONY,0}, {'-',AMP_VOL_DN,SONY,0}}, 

  //Test 1 Adafruit mini-remote: minus,play, plus 
  #define ROW_TEST_1 20
  {{'-',ADAF_MINI_VOLUME_DOWN,NEC,0}, {MY_PLAY,ADAF_MINI_PLAY_PAUSE,NEC,0}, {'+',ADAF_MINI_VOLUME_UP,NEC,0}},
  //Test 2 Adafruit mini-remote:  set up, up, stop
  #define ROW_TEST_2 21
  {{'S',ADAF_MINI_SETUP,NEC,0}, {MY_UP_ARROW,ADAF_MINI_UP_ARROW,NEC,0}, {MY_STOP,ADAF_MINI_STOP_MODE,NEC,0}}, 
  //Test 3 Adafruit mini-remote: left, enter, right
  #define ROW_TEST_3 22
  {{MY_LEFT_ARROW,ADAF_MINI_LEFT_ARROW,NEC,0}, {MY_RETURN,ADAF_MINI_ENTER_SAVE,NEC,0}, {MY_RIGHT_ARROW,ADAF_MINI_RIGHT_ARROW,NEC,0}},
  //Test 4 Adafruit mini-remote: 0, down arrow, repeat
  #define ROW_TEST_4 23
  {{'0',ADAF_MINI_0_10_PLUS,NEC,0},{MY_DOWN_ARROW,ADAF_MINI_DOWN_ARROW,NEC,0},{'r',ADAF_MINI_REPEAT,NEC,0}},
  //Test 5 Adafruit mini-remote:   1, 2, 3
  #define ROW_TEST_5 24
  {{'1',ADAF_MINI_1,NEC,0}, {'2',ADAF_MINI_2,NEC,0}, {'3',ADAF_MINI_3,NEC,0}}, 
  //Test 6 Adafruit mini-remote:  4, 5, 6
  #define ROW_TEST_6 25
  {{'4',ADAF_MINI_4,NEC,0}, {'5',ADAF_MINI_5,NEC,0}, {'6',ADAF_MINI_6,NEC,0}}, 
  //Test 7 Adafruit mini-remote: 7, 8, 9
  #define ROW_TEST_7 26
  {{'7',ADAF_MINI_7,NEC,0}, {'8',ADAF_MINI_8,NEC,0}, {'9',ADAF_MINI_9,NEC,0}},
   
  #define ROW_KB_1 27
  {KEYC('`'), KEYC('1'), KEYC('2'), KEYC('3'), KEYC('4'), KEYC('5'), KEYC('6'), 
   KEYC('7'), KEYC('8'), KEYC('9'), KEYC('0'), KEYC('-'), KEYC('='), KEY(MY_BACKSPACE,KEY_BACKSPACE)}, 
  #define ROW_KB_2 28
  {KEYC('q'), KEYC('w'), KEYC('e'), KEYC('r'), KEYC('t'), KEYC('y'), KEYC('u'),
   KEYC('i'), KEYC('o'), KEYC('p'), KEYC('['), KEYC(']'), KEYC('\\'), KEY(MY_DELETE,KEY_DELETE)}, 
  #define ROW_KB_3 29
  {KEYC('a'), KEYC('s'), KEYC('d'), KEYC('f'), KEYC('g'), KEYC('h'), KEYC('j'), 
   KEYC('k'), KEYC('l'), KEYC(';'), KEYC('\''), KEY(MY_RETURN, KEY_RETURN),KEY(MY_SPACE_BAR,' '), {' ',0x0,0,0}},  
  #define ROW_KB_4 30
  {KEYC('z'), KEYC('x'), KEYC('c'), KEYC('v'), KEYC('b'), KEYC('n'), KEYC('m'), 
   KEYC(','), KEYC('.'), KEYC('/'), {'S',0x0,0,SPL_KB_SHIFT}, 
   TOG('C',CYKM_TOGGLE_CONTROL), TOG('A',CYKM_TOGGLE_ALT),{' ',0x0,0,0}}, 
  #define ROW_KB_5 31
  {KEY('h',KEY_HOME), KEY(MY_UP_ARROW,KEY_UP_ARROW), KEY(MY_PAGE_UP,KEY_PAGE_UP), 
   KEY(MY_LEFT_ARROW,KEY_LEFT_ARROW), KEY(MY_RETURN,KEY_RETURN), KEY(MY_RIGHT_ARROW,KEY_RIGHT_ARROW), 
   KEY('e',KEY_END), KEY(MY_DOWN_ARROW,KEY_DOWN_ARROW), KEY(MY_PAGE_DOWN,KEY_PAGE_DOWN), 
   KEY('w',KEY_LEFT_GUI), TOG('W',CYKM_TOGGLE_GUI),KEY('e',KEY_ESC),TOG('0',CYKM_TOGGLE_RESET),{' ',0x0,0,0}},
  #define ROW_KB_6 32
  {KEY(MY_F1,KEY_F1), KEY(MY_F2,KEY_F2), KEY(MY_F3,KEY_F3), KEY(MY_F4,KEY_F4), KEY(MY_F5,KEY_F5), 
   KEY(MY_F6,KEY_F6), KEY(MY_F7,KEY_F7), KEY(MY_F8,KEY_F8), KEY(MY_F9,KEY_F9), KEY(MY_F10,KEY_F10), 
   KEY(MY_F11,KEY_F11), KEY(MY_F12,KEY_F12), {' ',0x0,0,0},{' ',0x0,0,0}},
  //Extras: just in case
  {{' ',0x0,0,0}, {' ',0x0,0,0}, {' ',0x0,0,0},
   {' ',0x0,0,0}, {' ',0x0,0,0}, {' ',0x0,0,0},
   {' ',0x0,0,0}, {' ',0x0,0,0}, {' ',0x0,0,0}, {' ',0x0,0,0}}
};
const char Num_Rows_in_Page[]= {5, 6, 7, 4, 4, 5, 8, 1};

//Define which pages contain which rows
const uint8_t Pages [] [10]= {
  /*Cable*/   {ROW_PAGES, ROW_CBL_PLAY, ROW_CBL_CH, ROW_CBL_ARROW, ROW_CBL_PIP},
  /*Mouse*/   {ROW_PAGES, ROW_MSAR_1, ROW_MSAR_2, ROW_MSAR_3, ROW_MSAR_4, ROW_MSAR_5},
  /*Keyboard*/{ROW_PAGES, ROW_KB_1,ROW_KB_2,ROW_KB_3,ROW_KB_4,ROW_KB_5,ROW_KB_6},
  /*Amp*/     {ROW_PAGES, ROW_AMP_1, ROW_AMP_2, ROW_AMP_3},
  /*TV*/      {ROW_PAGES, ROW_TV_1, ROW_TV_2, ROW_TV_3},
  /*Bluray*/  {ROW_PAGES, ROW_BLU_1, ROW_BLU_2, ROW_BLU_3, ROW_BLU_4},
  /*Test*/    {ROW_PAGES, ROW_TEST_1, ROW_TEST_2, ROW_TEST_3, ROW_TEST_4, ROW_TEST_5, ROW_TEST_6, ROW_TEST_7}
};
