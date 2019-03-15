#include <Arduino.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <EEPROM.h>
#include <Esplora.h>  // Try to use hardware but if not (buttons) use Esplora library function

#define DEBUG
#define SCALE 8

typedef uint16_t CRGB;
// SPI pins for Esplora (Arduino Leonardo style numbering)
#define mosi 16
#define miso 14
#define sclk 15

//SCREEN
#define s_width 128
#define s_height 149
#define cs 7  // Esplora uses display chip select on D7
#define dc 0  // Esplora uses LCD DC on D0
#define rst 1 // Esplora uses display reset on D1
Adafruit_ST7735 tft = Adafruit_ST7735(cs, dc, mosi, sclk, rst);  // define tft display (use Adafruit library)
long lastMillis;
boolean monitor = false;

#define RGB(r, g, b) (((r&0xF8)<<8)|((g&0xFC)<<3)|(b>>3))
//uint16_t color565(uint8_t r, uint8_t g, uint8_t b) { return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3); }
#define BLACK   0x0000
#define BLUE    0xF800
#define LBLUE    0xFFE0 // Cyan
#define RED     0x001F
#define ORANGE  0x00AA  //TEST CREATING COLOR
#define GREEN   0x07E0
#define LGREEN   0x04E0    
#define MAGENTA 0xF81F
#define YELLOW  0x07FF
#define WHITE   0xFFFF
#define LWHITE   0x4444

uint16_t colorLib[3] = {YELLOW, BLUE, WHITE};

//SOUND
#define sound_pin 6 // Direct sound on Esplora

//BUTTONS (not used on Esplora as buttons must be read using library function)
// #define Rbtn_pin 1
// #define Lbtn_pin 0
/* *** Game commonly used defines ** */
#define  DIR_UP    1
#define  DIR_RIGHT 2
#define  DIR_DOWN  3
#define  DIR_LEFT  4

#define  BTN_NONE  0
#define  BTN_EXIT  1
#define  BTN_START 2

#define  BTN_UP    4
#define  BTN_DOWN  8
#define  BTN_LEFT  16
#define  BTN_RIGHT  32

//ACCELEROMETER (mod for Esplora)
int acc_avgX, acc_avgY, acc_avgZ;
#define acc_pinX 1  // A5
#define acc_pinY 2  // A7
#define acc_pinZ 3  // A6

#define FIELD_WIDTH       15
#define FIELD_HEIGHT      10
#define LONG_SIDE 15
#define SHORT_SIDE 10

#define  NUM_PIXELS    FIELD_WIDTH*FIELD_HEIGHT
uint16_t leds[NUM_PIXELS];


uint16_t curControl = BTN_NONE;
#define MINPLAYER 1
#define MAXPLAYER 2
uint8_t nbPlayer = 4; // MINPLAYER ;
uint8_t nbPlayerDie ;
boolean appRunning = false;


void readInput(){
  curControl = BTN_NONE;

 // if (!digitalRead(S_pin) )
 //   curControl += BTN_START;
  if ( Esplora.readJoystickSwitch() )
    curControl += BTN_EXIT;
    
  if (Esplora.readButton(SWITCH_LEFT))
    curControl += BTN_LEFT;
  if (Esplora.readButton(SWITCH_UP))
    curControl += BTN_UP;
    
  if (Esplora.readButton(SWITCH_RIGHT))
    curControl += BTN_RIGHT;
  if (Esplora.readButton(SWITCH_DOWN))
    curControl += BTN_DOWN;
}

void setTablePixelRGB(int x, int y, int r,int g, int b){
 // leds[ (y * LONG_SIDE) + (LONG_SIDE - 1) - x ] = col ;
 // tft.drawPixel(x, y, RGB(r,g,b));
  tft.fillRect(x*SCALE, y*SCALE, SCALE, SCALE, RGB(r,g,b));
}

void setTablePixel(int x, int y, unsigned long color){
//   leds [ (y * LONG_SIDE) + x] = (color) ; 
//   tft.drawPixel(x, y, color);
   tft.fillRect(x*SCALE, y*SCALE, SCALE, SCALE, color);
 }

void setPixel(int n, unsigned long color){
//  leds[n] = color;
//  tft.drawPixel(n%s_width, n/s_width, color );
  uint8_t x,y;
  x=n%s_width;
  y=n/s_width;
  setTablePixel(x,y,color);
}

void setPixelRGB(int n, int r,int g, int b){
  uint8_t x,y;
  x=n%s_width;
  y=n/s_width;
  setTablePixelRGB(x,y,r,g,b);
//  leds[n] = RGB(r,g,b);
//  tft.drawPixel(n%s_width, n/s_width, RGB(r,g,b) );
}

void setTablePixelDouble(int x, int y, unsigned long col){
   setTablePixel( (x<<1), (y<<1), col);
   setTablePixel( (x<<1)+1, (y<<1), col);
   setTablePixel( (x<<1), (y<<1)+1, col);
   setTablePixel( (x<<1)+1, (y<<1)+1, col);
}

void clearTablePixels(){
  for (int n=0; n<FIELD_WIDTH*FIELD_HEIGHT; n++){
    setPixel(n,0);
  }
}

void showPixels(){
//  FastLED.show();
}

void testMatrix() {
    setTablePixel(0, 0, WHITE);
    showPixels();
    delay(2000);
    setTablePixel(0, 9, WHITE);
    showPixels();
    delay(2000);
    setTablePixel(9, 0, WHITE);
    showPixels();
    delay(2000);
    setTablePixel(9, 9, WHITE);
}

void initPixels(){

#ifdef DEBUG
  leds[0] = RGB(255,0,0); 
  leds[1] = RGB(0,255,0);
  leds[2] = RGB(0,255,0);
  leds[3] = RGB(0,0,255);
  leds[4] = RGB(0,0,255);
  leds[5] = RGB(0,0,255);
setTablePixel(0,1,RED );
setTablePixel(1,1,GREEN );
setTablePixel(2,1,BLUE );
setTablePixel(3,1,YELLOW);
setTablePixel(4,1,LBLUE);
//setTablePixel(5,1,PURPLE);
setTablePixel(6,1,WHITE);
setTablePixel(0,2,0xFF0000 );
setTablePixel(1,2,0x00FF00 );
setTablePixel(2,2,0x0000FF );
setTablePixel(0,3,CRGB(0xFF0000) );
setTablePixel(1,3,CRGB(0x00FF00) );
setTablePixel(2,3,CRGB(0x0000FF) );
//   FastLED.show();
   delay(1000);
for (int i =0xFF; i; i--)
{
  setTablePixel(0,2,i<<16 );
  setTablePixel(1,2,i<<8 );
  setTablePixel(2,2,i );
  unsigned long tmp=i<<16+i<<8+i;
  setTablePixel(3,2,tmp );  
  
  setTablePixelRGB(0,3,i,0,0 );
  setTablePixelRGB(1,3,0,i,0 );
  setTablePixelRGB(2,3,0,0,i );
  setTablePixelRGB(3,3,i,i,i );
  delay(20);
}
#endif
}

void fadeOut(){

     //Fade out by swiping from left to right with ruler
      const int ColumnDelay = 10;
      int curColumn = 0;
      for (int i=0; i<FIELD_WIDTH*ColumnDelay; i++){
//        dimLeds(0.97);
        if (i%ColumnDelay==0){
          //Draw vertical line
          for (int y=0;y<FIELD_HEIGHT;y++){
            setTablePixel(curColumn, y, GREEN);
          }
          curColumn++;
        }
        showPixels();
        delay(5);
      }
      //Sweep complete, keep dimming leds for short time
      for (int i=0; i<100; i++){
//        dimLeds(0.9);
        showPixels();
        delay(5);
      }
}

void setup() {
  // put your setup code here, to run once:
  // initialize a ST7735R TFT
  tft.initR();
  tft.setRotation(1);  // Set for landscape display on Esplora
  tft.setTextWrap(false); // Allow text to run off right edge


  //Serial.begin(9600);
 // FillWorld();

  tft.fillScreen(BLACK);
//  initPixels();
  delay(1000);
  testMatrix();
  delay(1000);
//  ScoreSetup();
}

////////////

uint8_t printmode=0;

// https://xantorohara.github.io/led-matrix-editor/

const uint64_t DAFTPUNK[] = {
  0x0000001b1b001b1b,
  0x000000001b1b1b00,
  0x000000000e0e0e00,
  0x0000000e0e000e0e,
  0x0000001b1b001b1b,
  0x0000001500110015,
  0x000000000e0a0e00,
  0x0000001500110015,
  0x00000007051f141c,
  0x0000001c141f0507,
  0x0000001f1515151f,
  0x000000001f111f00,
  0x0000001119151311,
  0x0000001113151911,
  0x00000000181b0300,
  0x00000000031b1800,
  0x0000001111111111,
  0x0000000a0a0a0a0a,
  0x0000000404040404,
  0x0000000c0c0c0c0c,
  0x000000111b1f0e04,
  0x000000040e1f1b11,
  0x0000001f1111111f,
  0x000000001f111f00,
  0x000000110a040a11,
  0x000000111b0e1b11,
  0x000000000a1f0a00,
  0x00000000040e0400,
  0x000000000a040a00,
  0x00000004041f0404,
  0x000000110a000a11
};
const int DAFTPUNK_LEN = sizeof(DAFTPUNK)/8;

               
void initDP() {
  appRunning = true;

//  randomSeed(analogRead(0));
  clearTablePixels();
  showPixels();
}

void displayImageDP(uint64_t image) 
{
  for (int y = 0; y < 5; y++) 
  {
    byte row = (image >> y * 8) & 0xFF;
    for (int x = 0; x<5; x++) 
    {
	    if( bitRead(row, x) )
      {
        if (!printmode)
        {
          setTablePixelDouble (x+1, y, RED );
        }
        else 
        {
          setTablePixel (x, y, RED);
          setTablePixel (x+5, y, YELLOW);
          setTablePixel (x+10, y, RED);
          setTablePixel (x, y+5, YELLOW);
          setTablePixel (x+5, y+5, RED);
          setTablePixel (x+10, y+5, YELLOW);
        }
      }
      else
      {
        if (!printmode)
        {
          setTablePixelDouble (x+1, y, BLACK);
        }
        else 
        {  
          setTablePixel (x, y, BLACK);
          setTablePixel (x+5, y, BLACK);
          setTablePixel (x+10, y, BLACK);
          setTablePixel (x, y+5, BLACK);
          setTablePixel (x+5, y+5, BLACK);
          setTablePixel (x+10, y+5, BLACK);
        }
      }
    }
  }
  showPixels();
//  delay (500);
}

void runDP() {
  initDP();
  unsigned long curTime, click=0;
  uint8_t i = 1;

  while(appRunning) 
  {
    displayImageDP(DAFTPUNK[i]);
    if (++i >= DAFTPUNK_LEN ) i = 0;
   
    curTime=millis();
    do
    {
      readInput();
      if (curControl == BTN_EXIT){
        appRunning = false;
        break;
      }
      else if (curControl != BTN_NONE && millis()-click > 600)
      {
        printmode=1-printmode;
        click=millis();
        clearTablePixels();
      }

    }
    while ((millis()- curTime) <1000);//Once enough time  has passed, proceed. The lower this number, the faster the game is //20

  }
//  displayLogo();
}







/////////////



void loop() {
  // put your main code here, to run repeatedly:
  fadeOut();
  runDP();
/*
  if(millis()-lastMillis > 20){
    lastMillis = millis();
    CheckButtons();
    MovePlayer();
    ScrollWorld();
    CollideBorders();
    CollideWorld();
    DrawWorld();
    DrawPlayer();
    if(monitor){
      ShowMonitor();
    }
    */
}

//tft.drawPixel(b_remaining*95/b_max, s_height+1, b_colors[b_onUseID]);
//tft.fillRect(b_remaining*95/b_max, s_height+2, 1, 10, b_colors[b_onUseID]);

//Esplora.readJoystickSwitch()
//Esplora.readButton(SWITCH_DOWN);

void drawString(byte x, byte y, char *text, uint16_t color, bool wrap) { // replicate tft.drawString
  tft.setCursor(x,y);
  tft.setTextColor(color);
  tft.setTextWrap(wrap);
  tft.print(text);
}

void drawChar(byte x, byte y, char text, uint16_t color) { // replicate tft.drawChar
  tft.setCursor(x,y);
  tft.setTextColor(color);
  tft.print(text);
}