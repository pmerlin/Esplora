#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <EEPROM.h>
#include <Esplora.h>  // Try to use hardware but if not (buttons) use Esplora library function

#define DEBUG

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


void setPixel(int n, unsigned long color){
  leds[n] = color;
  tft.drawPixel(n%s_width, n/s_width, color );
}

void setPixelRGB(int n, int r,int g, int b){
  leds[n] = RGB(r,g,b);
  tft.drawPixel(n%s_width, n/s_width, RGB(r,g,b) );
}

void setTablePixelrgb(int x, int y, CRGB col){
//  leds[ (y * LONG_SIDE) + (LONG_SIDE - 1) - x ] = col ;
  tft.drawPixel(x, y, col);
}

void setTablePixelRGB(int x, int y, int r,int g, int b){
 // leds[ (y * LONG_SIDE) + (LONG_SIDE - 1) - x ] = col ;
  tft.drawPixel(x, y, RGB(r,g,b));
}

void setTablePixelDouble(int x, int y, unsigned long col){
   setTablePixel( (x<<1), (y<<1), col);
   setTablePixel( (x<<1)+1, (y<<1), col);
   setTablePixel( (x<<1), (y<<1)+1, col);
   setTablePixel( (x<<1)+1, (y<<1)+1, col);
}

void setTablePixel(int x, int y, unsigned long color){
   leds [ (y * LONG_SIDE) + x] = (color) ; 
   tft.drawPixel(x, y, color);
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
  initPixels();
  delay(1000);
  testMatrix();
  delay(1000);
//  ScoreSetup();
}

void loop() {
  // put your main code here, to run repeatedly:
  fadeOut();
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
