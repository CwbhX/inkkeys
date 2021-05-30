//Display
const byte PIN_DIN = 16;
const byte PIN_CLK = 15;
const byte PIN_CS = 19;
const byte PIN_DC = 18;
const byte PIN_RST = 14;
const byte PIN_BUSY = 10;

//Display size
const short DISP_W = 128; //Dispaly width
const short DISP_H = 296; //Display height

//LEDs
const byte PIN_LED = 20;
const byte N_LED = 12; //Number of LEDs

//Rotary encoder
const byte PIN_ROTA = 0;
const byte PIN_ROTB = 1;
const byte PIN_SW1 = 21;

const byte ROT_FACTOR = 4;         //Smallest reported step, typically one "click" on the encoder 
const byte ROT_CIRCLE_STEPS = 20;  //Rotary steps in a full circle

//Keys
const byte ROW_1 = 5;
const byte ROW_2 = 6;
const byte COL_1 = 7;
const byte COL_2 = 8;
const byte COL_3 = 9;
const byte COL_4 = 10;
const byte COL_5 = 11;

//Slider
const byte SLIDER_OUT = 12;


const int DEBOUNCE_TIME = 50; //Debounce reject interval in milliseconds
