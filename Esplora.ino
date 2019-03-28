//#include <Arduino.h>
//#define ADA

//#define TABLE

#include <Esplora.h>  // Try to use hardware but if not (buttons) use Esplora library function
#include <SPI.h>
//#include <EEPROM.h>

#ifdef ADA
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h>
#else
#include <TFT.h>
#endif

#include <avr/pgmspace.h>


//#define DEBUG
#define DECAL 3
#define SCALE 8

typedef uint16_t CRGB;
// SPI pins for Esplora (Arduino Leonardo style numbering)
#define mosi 16
#define miso 14
#define sclk 15

//SCREEN

#define s_width 128
#define s_height 149

#ifdef ADA
#define cs 7  // Esplora uses display chip select on D7
#define dc 0  // Esplora uses LCD DC on D0
#define rst 1 // Esplora uses display reset on D1
Adafruit_ST7735 tft = Adafruit_ST7735(cs, dc, mosi, sclk, rst);  // define tft display (use Adafruit library)
#endif

long lastMillis;
boolean monitor = false;

//#define RGB(r, g, b) (((r&0xF8)<<8)|((g&0xFC)<<3)|(b>>3))
#define RGB(r, g, b) (((b&0xF8)<<8)|((g&0xFC)<<3)|(r>>3))

//uint16_t color565(uint8_t r, uint8_t g, uint8_t b) { return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3); }
#define BLACK   0x0000
#define BLUE    0xF800
#define LBLUE    0xFFE0 // Cyan
#define RED     0x001F
#define ORANGE  0x00AA  // 0xFD20      /* 255, 165,   0 */
#define GREEN   0x07E0  /*   0, 255,   0 */
#define LGREEN   0x04E0    
#define MAGENTA 0xF81F
#define YELLOW  0x07FF
#define WHITE   0xFFFF
#define LWHITE   0x4444
#define PURPLE   0x780F      /* 128,   0, 128 */
#define LPURPLE   0x3C07      

#define Black           0x0000      /*   0,   0,   0 */
#define Navy            0x000F      /*   0,   0, 128 */
#define DarkGreen       0x03E0      /*   0, 128,   0 */
#define DarkCyan        0x03EF      /*   0, 128, 128 */
#define Maroon          0x7800      /* 128,   0,   0 */
#define Olive           0x7BE0      /* 128, 128,   0 */
#define LightGrey       0xC618      /* 192, 192, 192 */
#define DarkGrey        0x7BEF      /* 128, 128, 128 */
#define Blue            0x001F      /*   0,   0, 255 */
#define Cyan            0x07FF      /*   0, 255, 255 */
#define Red             0xF800      /* 255,   0,   0 */
#define Magenta         0xF81F      /* 255,   0, 255 */
#define Yellow          0xFFE0      /* 255, 255,   0 */
#define Orange          0xFD20      /* 255, 165,   0 */
#define GreenYellow     0xAFE5      /* 173, 255,  47 */
#define Pink            0xF81F

uint16_t colorLib[3] = {YELLOW, BLUE, WHITE};
uint16_t PrintCol[2]= {YELLOW, RED};
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

#define  BTN_UP2    64
#define  BTN_DOWN2  128
#define  BTN_LEFT2  256
#define  BTN_RIGHT2 512


//ACCELEROMETER (mod for Esplora)
//int acc_avgX, acc_avgY, acc_avgZ;
#define acc_pinX 1  // A5
#define acc_pinY 2  // A7
#define acc_pinZ 3  // A6

#define FIELD_WIDTH       15
#define FIELD_HEIGHT      10
#define LONG_SIDE 15
#define SHORT_SIDE 10

#define  NUM_PIXELS    FIELD_WIDTH*FIELD_HEIGHT
#ifdef TABLE
uint16_t leds[NUM_PIXELS];
#endif

uint16_t curControl = BTN_NONE;
#define MINPLAYER 1
#define MAXPLAYER 2
uint8_t nbPlayer = 2; // MINPLAYER ;
uint8_t nbPlayerDie ;
boolean appRunning = false;


void readInput(){
  curControl = BTN_NONE;

  if ( Esplora.readJoystickSwitch() ==0 )
    curControl += BTN_START;
  
  if ( Esplora.readSlider() < 512 )
    curControl += BTN_EXIT;
    
  if (Esplora.readButton(SWITCH_LEFT) == LOW)
    curControl += BTN_LEFT;
  if (Esplora.readButton(SWITCH_UP) == LOW)
    curControl += BTN_UP;
    
  if (Esplora.readButton(SWITCH_RIGHT) == LOW)
    curControl += BTN_RIGHT;
  if (Esplora.readButton(SWITCH_DOWN) == LOW)
    curControl += BTN_DOWN;
}        

void setPixel(uint8_t n, uint16_t color){
  #ifdef TABLE
  leds[n] = color;
  #endif
//  tft.drawPixel(n%s_width, n/s_width, color );

#ifdef ADA
  tft.fillRect(n%s_width<<DECAL, n/s_width<<DECAL, SCALE, SCALE, color );
#else
//  EsploraTFT.stroke(color);
  EsploraTFT.fill(color);
  EsploraTFT.rect(n%s_width<<DECAL, n/s_width<<DECAL, SCALE, SCALE);
#endif
}

void setPixelRGB(uint8_t n, uint8_t r,uint8_t g, uint8_t b){
  #ifdef TABLE
  leds[n] = RGB(r,g,b);
  #endif
//  tft.drawPixel(n%s_width, n/s_width, RGB(r,g,b) );
#ifdef ADA
  tft.fillRect(n%s_width<<DECAL, n/s_width<<DECAL, SCALE, SCALE, RGB(r,g,b) );
#else
//  EsploraTFT.stroke(r,g,b);
  EsploraTFT.fill(r,g,b);
  EsploraTFT.rect(n%s_width<<DECAL, n/s_width<<DECAL, SCALE, SCALE);
#endif
}

void setTablePixelrgb(uint8_t x, uint8_t y, CRGB color){
  #ifdef TABLE
  leds[ (y * LONG_SIDE) + (LONG_SIDE - 1) - x ] = color ;
  #endif
//  tft.drawPixel(x, y, color);
#ifdef ADA
  tft.fillRect(x<<DECAL, y<<DECAL, SCALE, SCALE, color );
#else
//  EsploraTFT.stroke(color);
  EsploraTFT.fill(color);
  EsploraTFT.rect(x<<DECAL, y<<DECAL, SCALE, SCALE);
#endif
}

void setTablePixelRGB(uint8_t x, uint8_t y, uint8_t r,uint8_t g, uint8_t b){
  #ifdef TABLE
  leds[ (y * LONG_SIDE) + (LONG_SIDE - 1) - x ] = RGB(r,g,b) ;
  #endif
//  tft.drawPixel(x, y, RGB(r,g,b));
#ifdef ADA
  tft.fillRect(x<<DECAL, y<<DECAL, SCALE, SCALE, RGB(r,g,b) );
#else
//  EsploraTFT.stroke(r,g,b);
  EsploraTFT.fill(r,g,b);
  EsploraTFT.rect(x<<DECAL, y<<DECAL, SCALE, SCALE);
#endif  
}

void setTablePixel(uint8_t x, uint8_t y, uint16_t color){
   #ifdef TABLE
   leds [ (y * LONG_SIDE) + x] = (color) ; 
   #endif
//   tft.drawPixel(x, y, color);
#ifdef ADA
 tft.fillRect(x<<DECAL, y<<DECAL, SCALE, SCALE, color);
#else
//  EsploraTFT.stroke(color);
  EsploraTFT.fill(color);
  EsploraTFT.rect(x<<DECAL, y<<DECAL, SCALE, SCALE);
#endif
}

void setTablePixelDouble(uint8_t x, uint8_t y, uint16_t color){
   setTablePixel( (x<<1), (y<<1), color);
   setTablePixel( (x<<1)+1, (y<<1), color);
   setTablePixel( (x<<1), (y<<1)+1, color);
   setTablePixel( (x<<1)+1, (y<<1)+1, color);
}

void setTablePixelv(uint8_t x, uint8_t y, uint16_t color){
/*
  if (x & 0x01)
   leds[ x*LONG_SIDE + y ] = CRGB(color) ; //15
 else 
 leds [ (LONG_SIDE-1-y)+ x*LONG_SIDE ] = color ; */
 #ifdef TABLE
 leds[ (LONG_SIDE-1-y) + x ] = CRGB(color);
 #endif
#ifdef ADA
 tft.fillRect(x<<DECAL*LONG_SIDE, (x)<<DECAL, SCALE, SCALE, color);
#else
//  EsploraTFT.stroke(color);
  EsploraTFT.fill(color);
  EsploraTFT.rect( (LONG_SIDE-1-y)<<DECAL, (x)<<DECAL, SCALE, SCALE);
#endif   
}


uint16_t getPixel(uint8_t n){
//return (EsploraTFT.readPixel( n%s_width<<DECAL, n/s_width<<DECAL )); 
#ifdef TABLE
return (leds[n]);
#else
return 0;
#endif
}


void clearTablePixels(){
#ifdef ADA  
  tft.fillScreen(BLACK);
#else
  EsploraTFT.fillScreen(BLACK);
#endif
/*
  for (int n=0; n<FIELD_WIDTH*FIELD_HEIGHT; n++){
    setPixel(n,0);
  }
*/
}

void showPixels(){
//  FastLED.show();
}
/*
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
*/
void initPixels(){

#ifdef DEBUG
#ifdef TABLE
  leds[0] = RGB(255,0,0); 
  leds[1] = RGB(0,255,0);
  leds[2] = RGB(0,255,0);
  leds[3] = RGB(0,0,255);
  leds[4] = RGB(0,0,255);
  leds[5] = RGB(0,0,255);
#endif
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
for (uint8_t i =0x7F; i; i--)
{
  setTablePixelDouble(0,4,RGB(i,0,0) ); //16
  setTablePixelDouble(1,4,RGB(0,i,0) ); //8
  setTablePixelDouble(2,4,RGB(0,0,i) );
  setTablePixelDouble(3,4,RGB(i,0,i) );
  setTablePixelDouble(4,4,RGB(0,i,i) );
  setTablePixelDouble(5,4,RGB(i,i,0) );
  setTablePixelDouble(6,4,RGB(i,i,i) );
  
  unsigned long tmp=i<<16+i<<8+i;
  setTablePixel(3,2,tmp );  
  
  setTablePixelRGB(0,3,i,0,0 );
  setTablePixelRGB(1,3,0,i,0 );
  setTablePixelRGB(2,3,0,0,i );
  setTablePixelRGB(3,3,i,i,i );
  delay(25);
}
#endif
}
/*
void initPixelsv(){

//  FastLED.addLeds<FAST_LED_CHIPSET, FAST_LED_DATA_PIN, COLOR_ORDER>(leds, NUM_PIXELS).setCorrection(TypicalSMD5050);
//  FastLED.addLeds<FAST_LED_CHIPSET, FAST_LED_DATA_PIN>(leds, NUM_PIXELS).setCorrection(TypicalSMD5050);
//  FastLED.setBrightness(BRIGHTNESS);
#ifdef TABLE
  leds[14] = CRGB(255,0,0);  //0,1
  leds[15] = CRGB(0,255,0);  //0,2
  leds[44] = CRGB(0,255,0);  //0,3
  leds[45] = CRGB(0,0,255);
  leds[74] = CRGB(0,0,255);
  leds[75] = CRGB(0,0,255);
#endif  
//   FastLED.show();
   delay(1000);

  for (uint8_t y = 0; y < LONG_SIDE; y++)
  {
    for (uint8_t x = 0; x < SHORT_SIDE  ; x++)
    {
      setTablePixelv (x, y, YELLOW );
//      FastLED.show();
      delay(5);
      setTablePixelv(x, y, BLUE);
    }
  }   
  delay(1000);
}
*/



void dimLeds(float factor){
  //Reduce brightness of all LEDs, typical factor is 0.97
  for (uint8_t n=0; n<(FIELD_WIDTH*FIELD_HEIGHT); n++)
  {
    uint16_t curColor = getPixel(n);

    //Derive the tree colors
    byte  b = ( curColor >>11 );
    byte  g = ((curColor & 0x003F)>>5);
    byte  r = (curColor & 0x001F);
    //Reduce brightness
    r = r*factor;
    g = g*factor;
    b = b*factor;
    //Pack into single variable again
    curColor = RGB(r,g,b);
    //Set led again 
    setPixel(n,curColor);  
    }
}


void fadeOut(){

  uint8_t selection = 1;//random(3);

  
  switch(selection){
    case 0:
    case 1:
    {
      //Fade out by dimming all pixels
      for (uint8_t i=0; i<100; i++){
        dimLeds(0.80); //0.97
        showPixels();
        delay(20);
      }
      break;
    }
    case 2:
    {
      //Fade out by swiping from left to right with ruler
      const uint8_t ColumnDelay = 10;
      uint8_t curColumn = 0;
      for (uint8_t i=0; i<FIELD_WIDTH*ColumnDelay; i++){
        dimLeds(0.97);
        if (i%ColumnDelay==0){
          //Draw vertical line
          for (uint8_t y=0;y<FIELD_HEIGHT;y++){
            setTablePixel(curColumn, y, GREEN);
          }
          curColumn++;
        }
        showPixels();
        delay(5);
      }
      //Sweep complete, keep dimming leds for short time
      for (uint8_t i=0; i<100; i++){
        dimLeds(0.9);
        showPixels();
        delay(5);
      }
      break;
    }
  }
}


void displayLogo(){
  CRGB ipal[9];
  ipal[0] = RGB(0,0,0); 
  ipal[1] = RGB(255,0,0); 
  ipal[2] = RGB(0,255,0); 
  ipal[3] = RGB(30,30,255); 
  ipal[4] = RGB(100,220,220); 
  ipal[5] = RGB(160,190,200); 
  ipal[6] = RGB(100,220,250); 
  ipal[7] = RGB(110,80,210); 
  ipal[8] = RGB(210,190,200); 

  const uint8_t PROGMEM ledtable[10][15] = {
  {1,1,1,0,2,2,2,3,0,0,0,3,4,0,0},
  {1,0,0,1,0,2,0,0,3,0,3,0,4,0,0},
  {1,0,0,1,0,2,0,0,0,3,0,0,4,0,0},
  {1,1,1,0,0,2,0,0,3,0,3,0,4,0,0},
  {1,0,0,0,2,2,2,3,0,0,0,3,4,4,4},
  {5,5,5,0,6,6,0,7,7,7,0,8,0,0,0},
  {0,5,0,6,0,0,6,7,0,0,7,8,0,0,0},
  {0,5,0,6,0,0,6,7,7,7,0,8,0,0,0},
  {0,5,0,6,6,6,6,7,0,0,7,8,0,0,0},
  {0,5,0,6,0,0,6,7,7,7,0,8,8,8,0}
};

  for (uint8_t y = 0; y < SHORT_SIDE; ++y) 
    for (uint8_t x = 0; x < LONG_SIDE; ++x)
    {
//      uint8_t idx = pgm_read_byte_near (&ledtable[y][x] );
      uint8_t idx = ledtable[y][x] ;
      setTablePixelrgb(x, y, ipal[idx]);
    }
  delay(2000);
}

static const unsigned char pong []PROGMEM = {
0xff,0xe0,0x0,0x3f,0x80,0x7,0xe0,0x7,0xc0,0x3,0xfc,0x0,
0xff,0xf8,0x1,0xff,0xe0,0x7,0xf0,0x7,0xc0,0x1f,0xff,0x0,
0xff,0xfc,0x3,0xff,0xf0,0x7,0xf0,0x7,0xc0,0x3f,0xff,0x0,
0xff,0xfe,0x7,0xff,0xf8,0x7,0xf8,0x7,0xc0,0xff,0xff,0x0,
0xf8,0x7f,0xf,0xff,0xfc,0x7,0xfc,0x7,0xc0,0xff,0xff,0x0,
0xf8,0x3f,0xf,0xe0,0xfe,0x7,0xfc,0x7,0xc1,0xfc,0x7,0x0,
0xf8,0x1f,0x1f,0x80,0x7e,0x7,0xfe,0x7,0xc3,0xf8,0x1,0x0,
0xf8,0x1f,0x1f,0x0,0x3e,0x7,0xfe,0x7,0xc3,0xf0,0x0,0x0,
0xf8,0x1f,0x3f,0x0,0x3f,0x7,0xdf,0x7,0xc7,0xe0,0x0,0x0,
0xf8,0x1f,0x3e,0x0,0x1f,0x7,0xdf,0x87,0xc7,0xc0,0x0,0x0,
0xf8,0x3f,0x3e,0x0,0x1f,0x7,0xcf,0x87,0xc7,0xc1,0xff,0x80,
0xf8,0x7e,0x3e,0x0,0x1f,0x7,0xc7,0xc7,0xc7,0xc1,0xff,0x80,
0xff,0xfe,0x3e,0x0,0x1f,0x7,0xc7,0xe7,0xc7,0xc1,0xff,0x80,
0xff,0xfc,0x3e,0x0,0x1f,0x7,0xc3,0xe7,0xc7,0xc1,0xff,0x80,
0xff,0xf8,0x3e,0x0,0x1f,0x7,0xc1,0xf7,0xc7,0xc0,0xf,0x80,
0xff,0xe0,0x3f,0x0,0x3f,0x7,0xc1,0xf7,0xc7,0xe0,0xf,0x80,
0xf8,0x0,0x1f,0x0,0x3e,0x7,0xc0,0xff,0xc3,0xe0,0xf,0x80,
0xf8,0x0,0x1f,0x80,0x7e,0x7,0xc0,0x7f,0xc3,0xf0,0xf,0x80,
0xf8,0x0,0x1f,0xc0,0xfc,0x7,0xc0,0x7f,0xc3,0xfc,0xf,0x80,
0xf8,0x0,0xf,0xff,0xfc,0x7,0xc0,0x3f,0xc1,0xff,0xff,0x80,
0xf8,0x0,0x7,0xff,0xf8,0x7,0xc0,0x3f,0xc0,0xff,0xff,0x80,
0xf8,0x0,0x3,0xff,0xf0,0x7,0xc0,0x1f,0xc0,0x7f,0xff,0x80,
0xf8,0x0,0x1,0xff,0xe0,0x7,0xc0,0xf,0xc0,0x3f,0xff,0x0,
0xf8,0x0,0x0,0x7f,0x0,0x7,0xc0,0xf,0xc0,0x7,0xf8,0x0
};

static const unsigned char game []PROGMEM ={
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x80,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0xff,0x80,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc3,0xff,0x80,
0x0,0x0,0x0,0x0,0x0,0x0,0xf,0xc3,0xff,0x80,
0x0,0x0,0x0,0x0,0x3,0xe0,0xf,0xc3,0xe0,0x0,
0x0,0x0,0x0,0x0,0x7,0xf0,0x1f,0xc1,0xe0,0x0,
0x0,0x0,0x0,0xf8,0x7,0xf0,0x1f,0xc1,0xe0,0x0,
0x0,0xfc,0x1,0xfc,0x7,0xf8,0x1f,0xc1,0xe0,0x0,
0x7,0xfc,0x1,0xfc,0x3,0xf8,0x1f,0xe1,0xff,0x80,
0x1f,0xfc,0x1,0xde,0x3,0xbc,0x3d,0xe1,0xff,0x80,
0x3f,0xfe,0x1,0xde,0x3,0xbc,0x39,0xe1,0xff,0x80,
0x7e,0x0,0x3,0xdf,0x3,0xde,0x39,0xe1,0xfc,0x0,
0x7c,0x0,0x3,0xcf,0x3,0xde,0x39,0xe1,0xe0,0x0,
0xf8,0x0,0x3,0xcf,0x3,0xcf,0x39,0xe1,0xf0,0x0,
0xf8,0x0,0x3,0x87,0x83,0xcf,0x79,0xe0,0xf0,0x0,
0xf0,0x7f,0x7,0x87,0x83,0xc7,0xf1,0xe0,0xf0,0xe0,
0xf0,0xff,0x7,0x83,0xc3,0xc7,0xf1,0xe0,0xff,0xe0,
0xf0,0xff,0x7,0xff,0xc1,0xc3,0xf1,0xf0,0xff,0xe0,
0xf0,0xff,0x7,0xff,0xe1,0xc3,0xf0,0xf0,0xff,0xe0,
0xf8,0xf,0xf,0xff,0xe1,0xc1,0xe0,0xf0,0xe0,0x0,
0xf8,0xf,0x8f,0x1,0xf1,0xe1,0xe0,0xf0,0x0,0x0,
0x7c,0xf,0x8f,0x0,0xf1,0xe1,0xe0,0x0,0x0,0x0,
0x7f,0x1f,0x8f,0x0,0xf9,0xc0,0x0,0x0,0x0,0x0,
0x3f,0xff,0x9f,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x1f,0xff,0x1f,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x7,0xfc,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0
};

void setup() {
  // put your setup code here, to run once:
  // initialize a ST7735R TFT
#ifdef ADA  
  tft.initR();
//  tft.setRotation(1);  // Set for landscape display on Esplora
  tft.setTextWrap(false); // Allow text to run off right edge
 // FillWorld();
  tft.fillScreen(BLACK);
#else
  EsploraTFT.begin();
  EsploraTFT.background(0,0,0);
  EsploraTFT.noStroke(); //(200,20,180);
#endif

  EsploraTFT.drawBitmap(3,3,pong,89,24,GREEN);
  EsploraTFT.drawBitmap(10,30,game,75,26,RED);
  delay(2000);
  
  initPixels();
  delay(1000);
//  testMatrix();
  displayLogo();
  delay(1000);
//  ScoreSetup();
}

//////////////////////////////////////////////////


uint8_t printmode=0;

// https://xantorohara.github.io/led-matrix-editor/

const uint64_t PROGMEM DAFTPUNK[] = {
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
const uint8_t DAFTPUNK_LEN = sizeof(DAFTPUNK)/8;

               
void initDP() {
  appRunning = true;

//  randomSeed(analogRead(0));
  clearTablePixels();
  showPixels();
}

void displayImageDP(uint64_t image) 
{
//  EsploraTFT.stroke(RED);
//  EsploraTFT.fill(RED);

  for (uint8_t y = 0; y < 5; y++) 
  {
    byte row = (image >> y * 8) & 0xFF;
    for (uint8_t x = 0; x<5; x++) 
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
  uint64_t lb;
  
  while(appRunning) 
  {
memcpy_P(&lb, & (DAFTPUNK[i]) , 8);
displayImageDP(lb);
//    displayImageDP(DAFTPUNK[i]);
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
  displayLogo();
}

/////////////////////////////////////////////////

const uint64_t CHIFFRE[] = {
  0x0000000705050507,
  0x0000000702020302,
  0x0000000701070407,
  0x0000000704070407,
  0x0000000404070505,
  0x0000000704070107,
  0x0000000705070107,
  0x0000000404060407,
  0x0000000705070507,
  0x0000000704070507
};

void printDigit (uint8_t num, uint8_t x, uint8_t y, unsigned long col)
{
     for (uint8_t i=0; i <5 ; i++)
     {
        byte row= (CHIFFRE[num] >>i *8) & 0xFF;
        for (uint8_t j = 0; j<3; j++)
        {
          if (bitRead(row,j))
            setTablePixel (j+x, i+y, col);
          else
            setTablePixel (j+x, i+y, BLACK);
        }
     }
}

void printNumber (uint8_t num, uint8_t x, uint8_t y, unsigned long col)
{
  uint8_t d=num/10;
  uint8_t u=num%10;

  if (d){
    printDigit (d,x,y, col);
    printDigit (u,x+4,y, col);
  }
  else printDigit (u,x+2,y, col);
}

/*
//#include "font.h"

uint8_t charBuffer[8][8];

uint8_t loadCharInBuffer(char letter){
  uint8_t* tmpCharPix;
  uint8_t tmpCharWidth;
  
  int letterIdx = (letter-32)*8;
  
  int x=0; int y=0;
  for (int idx=letterIdx; idx<letterIdx+8; idx++){
    for (int x=0; x<8; x++){
//      charBuffer[x][y] = ((font[idx]) & (1<<(7-x)))>0;
    }
    y++;
  }
  
  tmpCharWidth = 8;
  return tmpCharWidth;
}


void printText(char* text, unsigned int textLength, int xoffset, int yoffset, int color){
  uint8_t curLetterWidth = 0;
  int curX = xoffset;
  clearTablePixels();
  
  //Loop over all the letters in the string
  for (int i=0; i<textLength; i++){
    //Determine width of current letter and load its pixels in a buffer
    curLetterWidth = loadCharInBuffer(text[i]);
    //Loop until width of letter is reached
    for (int lx=0; lx<curLetterWidth; lx++){
      //Now copy column per column to field (as long as within the field
      if (curX>=LONG_SIDE){//If we are to far to the right, stop loop entirely
        break;
      } else if (curX>=0){//Draw pixels as soon as we are "inside" the drawing area
        for (int ly=0; ly<8; ly++){//Finally copy column
          setTablePixel(curX,yoffset+ly,charBuffer[lx][ly]*color);
        }
      }
      curX++;
    }
  }
  
  showPixels();
}
*/

#include "font2.h"
void printText3(char* text, uint8_t xoffset, uint8_t yoffset, CRGB color[2] ){
//  uint8_t curLetterWidth = 0;
  uint8_t curX = xoffset, col;
  
  //Loop over all the letters in the string
  for (uint8_t i=0; i<strlen(text); i++){
//    loadLetter(text[i]);
    
     
    //Loop until width of letter is reached
    for (uint8_t lx=0; lx<3; lx++){
      curX= 3*i + xoffset +lx;
      col=pgm_read_byte_near (& Wendy3x5[ (text[i]-32)*3 +lx ] );
      //Now copy column per column to field (as long as within the field
      if ( curX < LONG_SIDE && curX >= 0)   //If we are to far to the right, stop loop entirely
 //     if ( curX < 0) break;//If we are to far to the right, stop loop entirely      
    
      for (uint8_t ly=0; ly<5; ly++){//Finally copy column
//        if (letter[lx][ly])
        if ( bitRead(col, ly) )
          setTablePixel(curX, yoffset+ly, color[i%2]);
        else
          setTablePixel(curX, yoffset+ly, BLACK);
      }
    }
  }
}

void printText4(char* text, uint8_t xoffset, uint8_t yoffset, CRGB color[2] ){
//  uint8_t curLetterWidth = 0;
  uint8_t curX = xoffset, col;
  
  //Loop over all the letters in the string
  for (uint8_t i=0; i<strlen(text); i++){
//    loadLetter(text[i]);
    
     
    //Loop until width of letter is reached
    for (uint8_t lx=0; lx<4; lx++){
      curX= 4*i + xoffset +lx;
      if (lx==3) col = 0;
      else col=Wendy3x5[ (text[i]-32)*3 +lx ];
      
      //Now copy column per column to field (as long as within the field
      if ( curX < LONG_SIDE && curX >= 0)   //If we are to far to the right, stop loop entirely
 //     if ( curX < 0) break;//If we are to far to the right, stop loop entirely      
    
      for (uint8_t ly=0; ly<5; ly++){//Finally copy column
//        if (letter[lx][ly])
        if ( bitRead(col, ly) )
          setTablePixel(curX, yoffset+ly, color[i%2]);
        else
          setTablePixel(curX, yoffset+ly, BLACK);
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////
#define DECAL 4
boolean mappRunning;


void scrollText3(char* text, uint8_t lx, uint8_t ly, CRGB color[2]){
  uint8_t size=strlen(text)*3;
  
  for (int8_t x=0; x>-(size); x--){
    printText3(text, x+lx, ly, color);
    showPixels();
    delay (300);
  }
}

void scrollText4(char* text, uint8_t lx, uint8_t ly, CRGB color[2]){
  uint8_t size=strlen(text)*4;
  
  for (int8_t x=0; x>-(size); x--){
    printText4(text, x+lx, ly, color);
    showPixels();
    delay (300);
  }
}

void initNbPlayer(){  
  mappRunning = true;
  clearTablePixels();
  showPixels();
}

void runNbPlayer(){
  char *text= "Select NB PLAYER ";
  uint8_t size=(strlen(text)*3) + DECAL;
//  unsigned long PrintCol[2];
  unsigned long startTime, click=0, t;
//  PrintCol[0]= YELLOW;
//  PrintCol[1]= RED;


  initNbPlayer();
  
  while(mappRunning){
 
//    scrollText3 ("ABCDEFGHIJKLMNOPQRSTUVWXYZ ", 7, 0, PrintCol);
//    scrollText3 ("abcdefghijklmnopqrstuvwxyz ", 0, 0, PrintCol);
//    scrollText3 ("0123456789 ", 7, 0, PrintCol);
//    printText3 ("NB PLAYER", -1, 0, PrintCol);

  
    for (int8_t x=0; x>-(size); x--){
      printText3(text, x+DECAL, 0, PrintCol);
//      showPixels();
    
//// some stuff
      printNumber (nbPlayer, 4, 5, RED);
      showPixels();
    
      //Check input keys
      startTime=millis();
      do{
        readInput();
//        t=millis()-click;
  //      Serial.println(t);
        
        if (curControl == BTN_EXIT || curControl == BTN_START){
          mappRunning = false;
          break;
        }
        else if (curControl != BTN_NONE && millis()-click >400)
        {
          click=millis();
          if (curControl & BTN_RIGHT) nbPlayer++;
          else if (curControl & BTN_LEFT) nbPlayer--;

          if (nbPlayer<MINPLAYER) nbPlayer=MINPLAYER;
          else if (nbPlayer>MAXPLAYER) nbPlayer=MAXPLAYER;
        
        }
      }
      while ( mappRunning && (millis()- startTime) < 200); //Once enough time  has passed, proceed. The lower this number, the faster the game is
    }
  }
  displayLogo();
}
//////////////////////////////////////////////////////////////////////////////////////////////////

/* LedTable
 * 
 * Main code for Pong game
 */


#define PLAYER_HEIGHT 3
#define MAX_SCORE 5
#define AUTO_PLAYER_SPEED 200
#define MAXSNAKEPLAYER 2

/*
int8_t scorePlayer[MAXSNAKEPLAYER];
int8_t positionPlayer[MAXSNAKEPLAYER];
*/

int8_t scorePlayerLeft;
int8_t scorePlayerRight;
  
int8_t positionPlayerLeft;
int8_t positionPlayerRight;
  
int8_t ballx;
//int8_t previousBallx;
int8_t  bally;
//int8_t  previousBally;
int8_t  velocityx;
int8_t  velocityy;
int8_t  ballBounces;
  
//int8_t  gameSpeed;
  
unsigned long lastAutoPlayerMoveTime;
// unsigned long rumbleUntil;
//unsigned long waitUntil;

unsigned long curTime;
unsigned long prevUpdateTime = 0;

void pongInit(){
  scorePlayerLeft  = 0;
  scorePlayerRight = 0;
  positionPlayerLeft  = FIELD_HEIGHT/2;
  positionPlayerRight = FIELD_HEIGHT/2;
  ballx = FIELD_WIDTH/2;
  bally = FIELD_WIDTH/2;
  velocityx = 1; 
  velocityy = 0;
  ballBounces = 0;
//  gameSpeed = 180;
  lastAutoPlayerMoveTime = 0;
//  rumbleUntil = 0;
//  waitUntil = 0;
}

void checkBallHitByPlayer() {
  if(ballx == 1)
  {
    if(bally == positionPlayerLeft)  // middle of paddle
    {
      velocityx = 1;
//      ballx = 1;
      ++ballBounces;
//      rumbleUntil = curTime + 200;
    } 
    else if(bally < positionPlayerLeft && bally >= positionPlayerLeft - PLAYER_HEIGHT / 2) // upper part of paddle
    {
      velocityx = 1;
      velocityy = max(-1,velocityy-1); // or min ?
//      ballx = 1;
      bally = positionPlayerLeft - PLAYER_HEIGHT / 2-1;
      ++ballBounces;
//      rumbleUntil = curTime + 200;
    }    
    else if(bally > positionPlayerLeft && bally <= positionPlayerLeft + (PLAYER_HEIGHT-1) / 2) //lowe par of paddle
    {
      velocityx = 1;
      velocityy = min(1,velocityy+1); 
//      ballx = 1;
      bally = positionPlayerLeft + (PLAYER_HEIGHT-1) / 2+1;
      ++ballBounces;
//      rumbleUntil = curTime + 200;
    }    
  } 
  else if(ballx == FIELD_WIDTH-2)
  {
    if(bally == positionPlayerRight)
    {
      velocityx = -1;
//      ballx = FIELD_WIDTH-2;
      ++ballBounces;
    } 
    else if(bally < positionPlayerRight && bally >= positionPlayerRight - PLAYER_HEIGHT / 2) 
    {
      velocityx = -1;
      velocityy = max(-1,velocityy-1); 
//      ballx = FIELD_WIDTH-2;
      bally = positionPlayerRight - PLAYER_HEIGHT / 2-1;
      ++ballBounces;
    }    
    else if(bally > positionPlayerRight && bally <= positionPlayerRight + (PLAYER_HEIGHT-1) / 2) 
    {
      velocityx = -1;
      velocityy = min(1,velocityy+1); 
//      ballx = FIELD_WIDTH-2;
      bally = positionPlayerRight + (PLAYER_HEIGHT-1) / 2+1;
      ++ballBounces;
    }    
  } 
}

void checkBallOutOfBounds() {
  if(bally < 0) 
  {
    velocityy = - velocityy;
    //bally = 0;
    bally = 1;
  } 
  else if(bally > FIELD_HEIGHT-1) 
  {
    velocityy = - velocityy;
    bally = FIELD_HEIGHT-2;
    //bally = FIELD_HEIGHT-1;
  } 
  
  if(ballx < 0) 
  {
    velocityx = - velocityx;
    velocityy = 0;
    ballx = FIELD_WIDTH/2;
    bally = FIELD_HEIGHT/2;
    ++scorePlayerRight;
    ballBounces = 0;
//    waitUntil = curTime + 2000;
  } 
  else if(ballx > FIELD_WIDTH-1) 
  {
    velocityx = - velocityx;
    velocityy = 0;
    ballx = FIELD_WIDTH/2;
    bally = FIELD_HEIGHT/2;
    ++scorePlayerLeft;
    ballBounces = 0;
//    waitUntil = curTime + 2000;
  } 
}

boolean moveAutoPlayer()
{
  if(bally < positionPlayerRight)
  {
    if(positionPlayerRight - PLAYER_HEIGHT / 2>0) 
    {
      --positionPlayerRight;
      return true;
    }
  } 
  else if(bally > positionPlayerRight) 
  {
    if(positionPlayerRight + (PLAYER_HEIGHT-1) / 2 < FIELD_HEIGHT -1)
    {
      ++positionPlayerRight;
      return true;
    }      
  } 
  return false;
}

void setPosition(){
  switch(curControl){
    case BTN_DOWN:
    case BTN_RIGHT:
      if(positionPlayerLeft + (PLAYER_HEIGHT-1) / 2 < FIELD_HEIGHT-1){
        ++positionPlayerLeft;
      }
      break;
    case BTN_UP:     
    case BTN_LEFT: 
      if(positionPlayerLeft - PLAYER_HEIGHT / 2 > 0) {
        --positionPlayerLeft;
      }
      break;
  }
}

void runPong(){
  boolean dirChanged = false;
  pongInit();
  
  appRunning = true;
  while(appRunning)
  {    
    
    if (scorePlayerLeft == MAX_SCORE || scorePlayerRight == MAX_SCORE){
      appRunning = false;
      break;
    }
    
    checkBallHitByPlayer();
    
    if((curTime - lastAutoPlayerMoveTime) > AUTO_PLAYER_SPEED) 
    {
      if(moveAutoPlayer()) {
        lastAutoPlayerMoveTime = curTime;
      }
    }
    
    ballx += velocityx;
    bally += velocityy;

//    checkBallHitByPlayer();
    checkBallOutOfBounds();
    clearTablePixels();
    

    // Draw ball
    setTablePixel(ballx ,bally, WHITE);
    
    // Draw player left
    for (int y=positionPlayerLeft-PLAYER_HEIGHT/2; y<=positionPlayerLeft+PLAYER_HEIGHT/2; ++y){
      setTablePixel(0, y, BLUE);
    }
    // Draw player right
    for (int y=positionPlayerRight-PLAYER_HEIGHT/2; y<=positionPlayerRight+PLAYER_HEIGHT/2; ++y){
      setTablePixel(FIELD_WIDTH-1, y, YELLOW);
    }
    
    showPixels();
    
    
    curTime=millis();
    do{
      readInput();
      if (curControl == BTN_EXIT){
        appRunning = false;
        break;
      }
      if (curControl != BTN_NONE && !dirChanged){//Can only change direction once per loop
        dirChanged = true;
        setPosition();
      }
      curTime = millis();
    } 
    while ((curTime - prevUpdateTime) <250);  //Once enough time  has passed, proceed. The lower this number, the faster the game is
    prevUpdateTime = curTime;
  }
  
  fadeOut();
  displayLogo();
}


//////////////////////////////////////////////////////////////////////////////////////////////////

/* LedTable
 *
 * Written by: Patrick MERLIN
 * 
 * Simple animations
 */

/*
#define DELAY 60

const uint64_t  IMAGES[] = {
  0xf09090f00f09090f,
  0x00f0909ff9090f00,
  0x0000ff9999ff0000,
  0x000f09f99f90f000,
  0x0f09090ff09090f0,
  0x1e12121e78484878,
  0x3c24243c3c24243c,
  0x784848781e12121e
};
const uint8_t IMAGES_LEN = sizeof(IMAGES)/8;

const uint64_t IMAGES2[] = {
  0x0000000000000000,
  0x0000001818000000,
  0x00003c3c3c3c0000,
  0x007e7e7e7e7e7e00,
  0xffffffffffffffff,
  0xffffffe7e7ffffff,
  0xffffc3c3c3c3ffff,
  0xff818181818181ff
};

const uint64_t IMAGES3[] = {
  0x2020f824241f0404,
  0x10107c18183e0808,
  0x08083e18187c1010,
  0x04041f2424f82020,
  0x0004243ffc242000,
  0x002024fc3f240400
};

const uint64_t IMAGES5[] = {
  0xe0a0e00000070507,
  0x70507000000e0a0e,
  0x38283800001c141c,
  0x1c141c0000382838,
  0x0e0a0e0000705070,
  0x0705070000e0a0e0
};

const byte bytex[]={3,4,5, 6,7,8, 8,8,8, 8,7,6, 5,4,3, 2,1,0, 0,0,0, 0,1,2 }; //24
//const byte bytey[]={0,0,0, 0,1,2, 3,4,5, 6,7,8, 8,8,8, 8,7,6, 5,4,3, 2,1,0  };


void countDown(uint8_t nb)
{
  for (int8_t r=nb; r >=0 ; r--)
  {
     for (uint8_t i=0; i <5 ; i++)
     {
        byte row= (CHIFFRE[r] >>i *8) & 0xFF;
        for (uint8_t j = 0; j<3; j++)
        {
          if (bitRead(row,j))
            setTablePixel (j+3, i+2, RED);
          else
            setTablePixel (j+3, i+2, BLACK);
        }
         
      }
      for (uint8_t i=0; i <24 ; i++)
      {
      
///      ledtable.fill(bytec[r],bytec[ (r+17)%24] , color_red);
        for (uint8_t j = 0; j<8; j++)
        {
          setTablePixel(bytex[(i-j+24)%24],bytex[(i-j+18)%24] , WHITE);
          setTablePixel(bytex[(i-j+16)%24],bytex[(i-j+10)%24] , BLACK); 
        }      
        showPixels();
        delay(1000/24);
      }

//     clearTablePixels();
  }
}

void displayDoubleImage(const uint64_t image)
{
  for (uint8_t i=0; i <8 ; i++)
  {
    byte row= (image >>i *8) & 0xFF;
    for (uint8_t j = 0; j<8; j++)
    {
      if (bitRead(row,j)){
        setTablePixel(j, i, PURPLE);
        setTablePixel(j+7, i+2, RED);}
      else{
        setTablePixel(j, i, BLACK);
        setTablePixel(j+7, i+2, BLACK);}
    }
    showPixels();
  }
}

void DelayAndTestExit(uint8_t time){
  static unsigned long prevUpdateTime = 0;
  static unsigned long curTime = 0;

    //Check input keys
    do{
      readInput();
      if (curControl == BTN_EXIT){
        appRunning = false;
        break;
      }
      curTime = millis();
      delay(10);
    } 
    while ((curTime - prevUpdateTime) <80);//Once enough time  has passed, proceed. The lower this number, the faster the game is
    prevUpdateTime = curTime;  
}




void initTest(){
  appRunning = true;
  countDown(3);

  printNumber (15, 0, 0, RED);
  printNumber (99, 8,0, YELLOW);
  showPixels();
  delay (5000);
  clearTablePixels();
}

void runTest(){
  clearTablePixels();
  showPixels();
  initTest();  


  
  while(appRunning)
  {

  if (appRunning)  for (uint8_t d=0; d<5; d++)
  {
  for (uint8_t c=1; c<6; c++)
  {
    displayDoubleImage(IMAGES5[c]);
    DelayAndTestExit(DELAY);
  }
 
  if (appRunning) for (int8_t c=6-2; c>=0; c--)
  {
    displayDoubleImage(IMAGES5[c]);
    DelayAndTestExit(DELAY);
  }
  }

  if (appRunning) for (uint8_t d=0; d<5; d++)
  {
  for (uint8_t c=0; c<IMAGES_LEN; c++)
  {
    displayDoubleImage(IMAGES2[c]);
    DelayAndTestExit(DELAY);
  }
 
  if (appRunning) for (int8_t c=IMAGES_LEN-1; c>=0; c--)
  {
    displayDoubleImage(IMAGES2[c]);
    DelayAndTestExit(DELAY);
  }
  }


  if (appRunning) for (uint8_t d=0; d<10; d++)
  for (uint8_t c=0; c<6; c++)
  {
    displayDoubleImage(IMAGES3[c]);
    DelayAndTestExit(DELAY);
  }

  if (appRunning) for (uint8_t d=0; d<10; d++)
  for (uint8_t c=0; c<IMAGES_LEN; c++)
  {
    displayDoubleImage(IMAGES[c]);
    DelayAndTestExit(DELAY);
  }


  }
  displayLogo();
}

*/

///////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Snake game
 */

/*
#define SPEED 500

#define MAXSNAKEPLAYER 4
uint8_t length[MAXSNAKEPLAYER];  //Current length of snake + score (-3)
uint8_t xs[MAXSNAKEPLAYER][127]; //Array containing all snake segments,
uint8_t ys[MAXSNAKEPLAYER][127]; // max snake length is array length
uint8_t dir[MAXSNAKEPLAYER];      //Current Direction of snake
uint8_t isDead[MAXSNAKEPLAYER];   //Is snake alive
//uint8_t score[MAXSNAKEPLAYER];
#define SNAKEWIDTH  1 //Snake width


boolean snakeGameOver;

uint8_t ax = 0;//Apple x position
uint8_t ay = 0;//Apple y position
//uint8_t acolor = BLUE;

// Collision detection function
boolean checkCollision(int x1, int x2, int y1, int y2)
{
  if ( x1==x2 && y1==y2)
    return true;
  return false;
}

//TODO Check if 3 dead snake around apple.
void newApple()
{
  uint8_t i,j,collision; 
  do
  {
    collision=0;
    ax = random(FIELD_WIDTH-1);
    ay = random(FIELD_HEIGHT-1);
    for (j=0; j<nbPlayer; j++)
      for(i=0; i<length[j]; i++)
        if (checkCollision(ax, xs[j][i], ay, ys[j][i])) 
        {
          collision++;
          break;
        }
   }
   while (collision !=0);
}

void snakeInit(){
  //Snake start position and direction & initialise variables

  uint8_t i;

  nbPlayerDie=0;
  
  for (i=0; i<nbPlayer; i++)
  {
    length[i] = 3;
//    score[i] = 0;
    isDead[i] =0;
  }
  
  xs[0][0]=3; xs[0][1]=2; xs[0][2]=1;
  ys[0][0]=4; ys[0][1]=4; ys[0][2]=4;
  dir[0] = DIR_RIGHT;

  if(nbPlayer >1)
  {
    xs[1][0]=11; xs[1][1]=12; xs[1][2]=13;
    ys[1][0]=6; ys[1][1]=6; ys[1][2]=6;
    dir[1] = DIR_LEFT;
  }
  
  if(nbPlayer > 2)
  {
    ys[0][0]=2; ys[0][1]=2; ys[0][2]=2;
    ys[1][0]=3; ys[1][1]=3; ys[1][2]=3;

    xs[2][0]=3; xs[2][1]=2; xs[2][2]=1;
    ys[2][0]=7; ys[2][1]=7; ys[2][2]=7;
    dir[2] = DIR_RIGHT;
  }

  if(nbPlayer > 3)
  {
    xs[3][0]=11; xs[3][1]=12; xs[3][2]=13;
    ys[3][0]=8; ys[3][1]=8; ys[3][2]=8;
    dir[3] = DIR_LEFT;   
  }
  
  //Generate random apple position
  newApple();

  snakeGameOver = false;
}

void setDirection()
{
  if (curControl & BTN_LEFT || curControl & BTN_DOWN || curControl & BTN_RIGHT2 || curControl & BTN_UP2)
  {
     dir[0]--;
     if (dir[0]==0) dir[0]=4;    
  }
  if (curControl & BTN_RIGHT ||  curControl & BTN_UP || curControl & BTN_LEFT2 || curControl & BTN_DOWN2)
  {
     dir[0]++; 
     if (dir[0]==5) dir[0]=1;
  }
}

void setDirectionJ1_2()
{
  if (curControl & BTN_LEFT)
  {
     dir[0]--;
     if (dir[0]==0) dir[0]=4;    
  }
  if (curControl & BTN_RIGHT)
  {
     dir[0]++;
     if (dir[0]==5) dir[0]=1;
  }
}

void setDirectionJ2_2()
{
  if (curControl & BTN_RIGHT2)
  {
     dir[1]--;
     if (dir[1]==0) dir[1]=4;    
  }
  if (curControl & BTN_LEFT2)
  {
     dir[1]++;
     if (dir[1]==5) dir[1]=1;
  }
}

void setDirectionJ1()
{
  if (curControl & BTN_LEFT)
  {
     dir[0]--;
     if (dir[0]==0) dir[0]=4;    
  }
  if (curControl & BTN_UP)
  {
     dir[0]++;
     if (dir[0]==5) dir[0]=1;
  }
}

void setDirectionJ2()
{
  if (curControl & BTN_UP2)
  {
     dir[1]--;
     if (dir[1]==0) dir[1]=4;    
  }

  if (curControl & BTN_LEFT2)
  {
     dir[1]++;
     if (dir[1]==5) dir[1]=1;
  }
}

void setDirectionJ3()
{
  if (curControl & BTN_DOWN) 
  {
     dir[2]--;
     if (dir[2]==0) dir[2]=4;
  }
  if (curControl & BTN_RIGHT)
  {
     dir[2]++;
     if (dir[2]==5) dir[2]=1;
  }
}

void setDirectionJ4()
{
  if (curControl & BTN_RIGHT2) 
  {
      dir[3]--;
      if (dir[3]==0) dir[3]=4;
  }
  if (curControl & BTN_DOWN2)
  {
      dir[3]++;
      if (dir[3]==5) dir[3]=1;
  }
}


void die() //Ending, show score
{
  nbPlayerDie++;
  if( nbPlayerDie == nbPlayer )
  {
    snakeGameOver = true;
    #ifdef DEBUG
    Serial.println("die");
    #endif
  }
}

void runSnake(){
  boolean snakeRunning = true;

  //Check buttons and set snake movement direction while we are waiting to draw the next move
  unsigned long curTime, now;
  unsigned long dirChanged= 0;
  unsigned long dirChanged2=0;
  unsigned long dirChanged3=0;
  unsigned long dirChanged4=0;
  uint8_t i,j,len;

  CRGB snakecol[]= { PURPLE , BLUE, GREEN, WHITE };
  CRGB snakecolhead[MAXSNAKEPLAYER];

  snakecolhead[0]=LPURPLE; //CRGB(0xFF0000);
  snakecolhead[1]=LBLUE; //CRGB(0xFFFF00);
  snakecolhead[2]=LGREEN;
  snakecolhead[3]=LWHITE;
  
  if (nbPlayer>MAXSNAKEPLAYER) nbPlayer=MAXSNAKEPLAYER;
  
  snakeInit();
  
  while(snakeRunning)
  {    
    //Check collision with snake
    for (j=0; j<nbPlayer; j++)
    {
      if ( isDead[j] ) continue;   
      for (i=0; i<nbPlayer; i++)
      {
        if (i!=j) //not same snake
        {
          for(len=0; len < length[i]; len++)
          {
//        if (collide(xs[j][0], xs[j][len], ys[j][0], ys[j][len], SNAKEWIDTH, SNAKEWIDTH, SNAKEWIDTH, SNAKEWIDTH))     
            if (checkCollision(xs[j][0], xs[i][len], ys[j][0], ys[i][len]))
            {
              #ifdef DEBUG
              Serial.println("\ncollision with other\n");
              #endif
              isDead[j]++;
              die();
              break;
            }
          }
        }
        else // same snake i==j
        {
          len=length[j]-1;
          while (len>3) // need to be at least 4 for a self-collision
          {
            if (checkCollision(xs[j][0], xs[j][len], ys[j][0], ys[j][len]))
            {
              #ifdef DEBUG
              Serial.println("\nself collision\n");
              #endif
              isDead[j]++;
              die();
              break;
            }
            len--;            
          }
        }
      }     
    }

    if (snakeGameOver){
      snakeRunning = false;
      break;
    }

    //Check collision of snake head with apple
    for (j=0; j<nbPlayer; j++)
    {
      if ( isDead[j] ) continue;
      
      if (checkCollision(xs[j][0], ax, ys[j][0], ay))
      {
        length[j]++;
            
        //Generate new apple position
        newApple();
      }
    }
    
    //Shift snake position array by one   
    for (j=0; j<nbPlayer; j++)
    {
      if ( isDead[j] ) continue;
      i = length[j]-1;
      while (i>=1){
        xs[j][i] = xs[j][i-1];
        ys[j][i] = ys[j][i-1];
        i = i-1;
      }
 
      //Determine new position of head of snake
      if (dir[j] == DIR_RIGHT){
        xs[j][0]++;
        if ( xs[j][0] == LONG_SIDE )  xs[j][0]=0;
      } 
      else if (dir[j] == DIR_LEFT){
        if ( xs[j][0] == 0) xs[j][0]=LONG_SIDE;
        xs[j][0]--;
      } 
      else if (dir[j] == DIR_UP){
        if ( ys[j][0] == 0) ys[j][0]=SHORT_SIDE;
        ys[j][0]--;
      } 
      else {//DOWN
        ys[j][0]++;
        if ( ys[j][0] == SHORT_SIDE )  ys[j][0]=0;
      }
    }
    
    //Check if outside playing field
    for (j=0; j<nbPlayer; j++)
    {
      if ( isDead[j] ) continue; 
      if (xs[j][0]<0) {xs[j][0] =FIELD_WIDTH -1;}
      else if (xs[j][0]>=FIELD_WIDTH) {xs[j][0] = 0;}      
      else if (ys[j][0]<0) {ys[j][0] =FIELD_HEIGHT -1;}
      else if (ys[j][0]>=FIELD_HEIGHT) {ys[j][0] = 0;}    
    }
    clearTablePixels();
   
    //Draw apple
    setTablePixel(ax,ay,YELLOW);

    //Draw snakes
    for (j=0; j<nbPlayer; j++)
    {    
      for (i=0; i<length[j]; i++)
      {
        if ( isDead[j] ) setTablePixel(xs[j][i], ys[j][i], RED);
        else 
        {
          if(i==0) setTablePixel(xs[j][i], ys[j][i], snakecolhead[j]);
          else setTablePixel(xs[j][i], ys[j][i], snakecol[j]);
        }
      }
    }
    
    showPixels();


    curTime = millis();
    do{
      readInput();
      if (curControl == BTN_EXIT){
        snakeRunning = false;
        break;
      } 
      now=millis();

      if ( curControl != BTN_NONE )
      {       
        #ifdef DEBUG
        Serial.print(curControl);
        #endif

        switch(nbPlayer)
        {
        case 1:
        {
          if (  (now-dirChanged)>SPEED &&  (curControl>BTN_START)  )  //Can only change direction once per loop
          {
            #ifdef DEBUG
            Serial.print("\nP1");
            #endif
            dirChanged=now; 
            setDirection();
          }
          break;
        }
        
        case 2:
        {
          if ( (now-dirChanged )>SPEED && ( (curControl&BTN_LEFT) || (curControl&BTN_RIGHT) ) ) //Can only change direction once per loop
          {
//            Serial.print("P1");
            dirChanged=now; 
            setDirectionJ1_2();
          }
        
          if ( (now-dirChanged2)>SPEED  && ( (curControl&BTN_LEFT2) || (curControl&BTN_RIGHT2) ) ) //Can only change direction once per loop
          {
//            Serial.print("P2");
            dirChanged2=now;
            setDirectionJ2_2();
          }
          break;          
        }
        
        
        case 3:
        case 4:
        { 
          #ifdef DEBUG
          Serial.print(curControl);
          #endif
          if ( (now-dirChanged )>SPEED && ( (curControl&BTN_LEFT) || (curControl&BTN_UP) ) ) //Can only change direction once per loop
          {
//            Serial.print("P1");
            dirChanged=now; 
            setDirectionJ1();
          }
        
          if ( (now-dirChanged2)>SPEED  && ( (curControl&BTN_LEFT2) || (curControl&BTN_UP2) ) ) //Can only change direction once per loop
          {
//            Serial.print("P2");
            dirChanged2=now;
            setDirectionJ2();
          }

          if ( (nbPlayer >2) && (now-dirChanged3 )>SPEED &&( (curControl&BTN_DOWN) || (curControl&BTN_RIGHT) ) )
          {
//            Serial.print("P3");
            dirChanged3=now;
            setDirectionJ3();
          }

          if ( (nbPlayer >3) && (now-dirChanged4 )>SPEED && ( (curControl&BTN_DOWN2) || (curControl&BTN_RIGHT2) ) )
          {
//            Serial.print("P4");
            dirChanged4=now;
            setDirectionJ4();
          }
        }
        break;
        }
      }
    } 
    while ( (millis() - curTime ) <SPEED);//Once enough time  has passed, proceed. The lower this number, the faster the game is // 
  }
  
  fadeOut();

  //add offset for 1 or 2 player to center scores
  if(nbPlayer==1) {i=4; j=2;}
  else if(nbPlayer==2) {i=0; j=2;}
  else {i=j=0;}
  
  printNumber (length[0]-3, i, j, snakecol[0]);
  if (nbPlayer > 1) printNumber (length[1]-3, 8, j, snakecol[1]);
  if (nbPlayer > 2) printNumber (length[2]-3, 0, 5, snakecol[2]);
  if (nbPlayer > 3) printNumber (length[3]-3, 8, 5, snakecol[3]);
  
  showPixels();
  delay (4000);
  fadeOut();
  displayLogo();   
}

// Set direction from current button state 


*/
/////////////////////////////////////////////////////

/* LedTable
 *
 * Written by: Ing. David Hrbaty
 * 
 * 
 * Main code for Bricks game
 */
 
float ballX = 10;
float ballY = 6;
float xincrement = 1;
float yincrement = 1;

uint8_t rad = 1;
uint8_t scorePlayer = 0;
uint8_t blockWidth = 1;
uint8_t blockHeight = 1;
uint8_t maxAttempt = 1;

int8_t positionPlayer = 6;

uint8_t numBlocks = 30;
#define MAX_SCORE numBlocks
#define MAX_ATTEMPT 5
#define PADDLE_SIZE 3
boolean continueGame = true;

char bricks[30][3] = {
  // in play, xloc, yloc 
   {1,0,0}  ,
   {1,1,0}  ,
   {1,2,0}  ,
   {1,3,0}  ,
   {1,4,0}  ,
   {1,5,0}  ,
   {1,6,0}  ,
   {1,7,0}  ,
   {1,8,0}  ,
   {1,9,0}  ,
   {1,0,1}  ,
   {1,1,1}  ,
   {1,2,1}  ,
   {1,3,1}  ,
   {1,4,1}  ,
   {1,5,1}  ,
   {1,6,1}  ,
   {1,7,1}  ,
   {1,8,1}  ,
   {1,9,1}  , 
   {1,0,2}  ,
   {1,1,2}  ,
   {1,2,2}  ,
   {1,3,2}  ,
   {1,4,2}  ,
   {1,5,2}  ,
   {1,6,2}  ,
   {1,7,2}  ,
   {1,8,2}  ,   
   {1,9,2}  
};
/* Block shape */
//static uint8_t brick[] PROGMEM = {
static uint8_t brick[]  = {
  10,
  8,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

void bricksInit(){
  scorePlayer = 0;
  maxAttempt = 0;
  ballY = 6;
  ballX = 10;
  for (uint8_t i=0; i<numBlocks; i++){
    bricks[i][0] = 1;
  }
}

void setPositionPaddle(){
  switch(curControl){
    case BTN_RIGHT:
      if(positionPlayer + (PADDLE_SIZE-1) / 2 < SHORT_SIDE-1){
        ++positionPlayer;
      }
      break;
    case BTN_LEFT:
      if(positionPlayer - PADDLE_SIZE / 2 > 0) {
        --positionPlayer;
      }
      break;
    case BTN_START:
      break;
    case BTN_UP:
      break;
    case BTN_DOWN:
      break;
  }
}

void checkBallHitByPaddle() {
//  if(ballY == SHORT_SIDE-2)
  if(ballY == LONG_SIDE-2) // line above paddle
  {
    if(ballX == positionPlayer) // paddle center
    {
      yincrement = -1;
 //     ballY = SHORT_SIDE-2;
 //     ballY = LONG_SIDE-2;
    } 
    else if(ballX < positionPlayer && ballX >= positionPlayer - PADDLE_SIZE / 2) // left side of paddle
    {
      yincrement = -1;
      xincrement = max(-1,(int)xincrement-1); 
//      ballY = FIELD_WIDTH-2;
//     ballY = LONG_SIDE-2;
      ballX = positionPlayer - PADDLE_SIZE / 2-1;
    }    
    else if(ballX > positionPlayer && ballX <= positionPlayer + (PADDLE_SIZE-1) / 2) // right side of the paddle
    {
      yincrement = -1;
      xincrement = min(1,(int)xincrement+1); //right
 //     ballY = SHORT_SIDE-2;
      ballX = positionPlayer + (PADDLE_SIZE-1) / 2+1;
    }    
  } 
}

void removeBlock(uint8_t index) //  Removes a block from game play
{
      bricks[index][0] = 0;                                             // Mark it as out of play
      scorePlayer++;                                                          // Increment score
      yincrement = -yincrement;                                         // Flip the y increment
}

boolean checkBlockCollision(){
    uint8_t ballTop = ballY-rad;                                            // Values for easy reference
    uint8_t ballBottom = ballY+rad;
    uint8_t ballLeft = ballX-rad;
    uint8_t ballRight = ballX+rad;
    
    for(uint8_t i=0;i<numBlocks;i++){                                       // Loop through the blocks
        if(bricks[i][0] == 1){                                          // If the block hasn't been eliminated
         uint8_t blockX = bricks[i][1];                                     // Grab x and y location
         uint8_t blockY = bricks[i][2];
         if(ballBottom >= blockY && ballTop <= blockY+blockHeight){     // If hitting BLOCK
           if(ballRight >= blockX && ballLeft <= blockX+blockWidth){       
             removeBlock(i);                                            // Mark the block as out of play
             return true;
           }
         }
      }
    }
  return false;                                                         // No collision detected
}

void checkBallOutOfBoundsTable() {
  if(ballY < 0) 
  {
    yincrement = - yincrement;
    ballY = 1;
  } 
  else if(ballY > LONG_SIDE-1) 
  {
    yincrement = - yincrement;
    xincrement = 0;
    ballY = LONG_SIDE/2;
    ballX = SHORT_SIDE/2;
    maxAttempt++;   
  } 
  if(ballX < 0) 
  {
    xincrement = - xincrement;
    ballX = 1;
  } 
  else if(ballX > SHORT_SIDE-1) 
  {
    xincrement = - xincrement;
    ballX = SHORT_SIDE-2;
  } 
}

void runBricks(){
  bricksInit();
  unsigned long prevUpdateTime = 0;
  boolean bricksRunning = true;

  while(bricksRunning){    
    
    if (scorePlayer == MAX_SCORE || maxAttempt == MAX_ATTEMPT){
      bricksRunning = false;
      break;
    }
    
    checkBallHitByPaddle();
    checkBlockCollision();
    
    ballX += xincrement;
    ballY += yincrement;
    
    checkBallOutOfBoundsTable();
    clearTablePixels();
    

    // Draw ball
    setTablePixelv(ballX,ballY,WHITE);
    
    // Draw player paddle
    for (uint8_t x=positionPlayer-PADDLE_SIZE/2; x<=positionPlayer+PADDLE_SIZE/2; ++x){
      setTablePixelv(x, LONG_SIDE-1, BLUE);
//      setTablePixelv(x, SHORT_SIDE-1, BLUE);
    }
    // Draw bricks
    for (uint8_t i=0; i<numBlocks; i++){
      if(bricks[i][0] == 1) {
        setTablePixelv(bricks[i][1],bricks[i][2], GREEN);
      }
    }
    showPixels();

    unsigned long curTime;
    boolean dirChanged = false;
    do{
      readInput();
      if (curControl == BTN_EXIT){
        bricksRunning = false;
        break;
      }
      if (curControl != BTN_NONE && !dirChanged){//Can only change direction once per loop
        dirChanged = true;
        setPositionPaddle();
      }
      curTime = millis();
    } 
    while ((curTime - prevUpdateTime) <250);//Once enough time  has passed, proceed. The lower this number, the faster the game is
    prevUpdateTime = curTime;
  }
  
  fadeOut();
  char buf[4];
  uint8_t len = sprintf(buf, "%i ", scorePlayer);

//TODO 
//  scrollTextBlockedv(buf,len,WHITE);
  scrollText3 (buf, 7, 0, PrintCol);
  displayLogo();
}
//////





///////////////////////////////////////////////////////////////////////////////////////////////////
#define MINSELECTION  1
#define MAXSELECTION  13


uint8_t curSelection = MINSELECTION;

#define TEXTSPEED  140

void checkSelectionRange(){
  if (curSelection>MAXSELECTION){
    curSelection = MINSELECTION;
  } else if (curSelection<MINSELECTION){
    curSelection = MAXSELECTION;
  }
}

void mainLoop(void){
  char* curSelectionText;
  uint8_t curSelectionTextLength;
  unsigned long prevUpdateTime = 0;
  int8_t oldNbPlayer;

  char* SelectionText[]= { "", "1 Rainbow ", "2 Animation ", "3 Stars ", "4 Vu Meter ", "5 DaftPunk ", "6 Tetris ", "7 Snake ", 
  "8 Pong ", "9 Bricks ", "10 Test ", "11 GameOfLife ", "12 Nb Player ", "13 JinX ", "14 Cylon ", "15 Plasma " };

runPong();
//runBricks();
//runNbPlayer();
//runDP();
//runColorPalette();

  while(true){
    //Show menu system and wait for input
    clearTablePixels();
    curSelectionText= SelectionText[curSelection];
    curSelectionTextLength=strlen(curSelectionText);
    
    boolean selectionChanged = false;
    boolean runSelection = false;

    //Scroll current selection text from right to left;
//    for (uint8_t x=LONG_SIDE; x>-(curSelectionTextLength*8); x--){
    for (int8_t x=LONG_SIDE; x>-(curSelectionTextLength*3); x--){
//    for (uint8_t x=0; x>-(size); x--){

//      printText(curSelectionText, curSelectionTextLength, x, (SHORT_SIDE-8)/2, YELLOW);
      printText3(curSelectionText, x, 3, PrintCol );
//      printText4(curSelectionText, x, 3, PrintCol );
      
      //Read buttons
      unsigned long curTime;
      do{
        readInput();
        if (curControl != BTN_NONE){        
          if (curControl == BTN_LEFT){
            curSelection--;
            selectionChanged = true;
            delay(400);
          } 
          else if ( curControl & BTN_EXIT){
            #ifdef DEBUG
            Serial.print(curControl);
            #endif
            oldNbPlayer=nbPlayer;
            if  (curControl & BTN_LEFT) {nbPlayer=1;}
            else if  (curControl & BTN_UP) nbPlayer=2;
            else if  (curControl & BTN_DOWN) nbPlayer=3;
            else if  (curControl & BTN_RIGHT) nbPlayer=4;
            if (oldNbPlayer != nbPlayer) 
            {
              #ifdef DEBUG
              Serial.print("PRINT");
              #endif
              clearTablePixels();
 //             printText3 ("NbPla", 0, 0, PrintCol);
              printNumber (nbPlayer, 4, 5, RED);
              showPixels();
              delay (1000);
            }
          }
          else if (curControl == BTN_RIGHT){
            curSelection++;
            selectionChanged = true;
            delay(400);
          } 
          else if (curControl == BTN_START){
            runSelection = true;
          } 
          checkSelectionRange();
        }
        curTime = millis();
      } while (((curTime - prevUpdateTime) < TEXTSPEED) && (curControl == BTN_NONE));//Once enough time  has passed, proceed
      prevUpdateTime = curTime;
      
      if (selectionChanged || runSelection)
        break;
    }
    
    //If we are here, it means a selection was changed or a game started, or user did nothing
    if (selectionChanged){
      //For now, do nothing
    } else if (runSelection){//Start selected game
      switch (curSelection){
        case 1:
//          runRainbowPalette();
          break;
        case 2:
//          runColorPalette();
          break;
        case 3:
//          runStars();         
          break;
        case 4:
//          runVUmeter();
          break;
        case 5:
          runDP();
          break;
        case 6:
          runTetris();
          break;   
        case 7:
//         runSnake();
          break; 
        case 8:
          runPong();        
          break;  
        case 9:       
          runBricks();
          break;        
        case 10:
//          runTest();
          break;                           
        case 11:
//          runGameofLife();
          break;       
        case 12:
          runNbPlayer();
          break;                           
        case 13:
//          runJinx();
          break;                           
        case 14:
//          runCylon();
          break;                           
        case 15:
//          runPlasma();
          break;                           
      }
    } else {
      //If we are here, no action was taken by the user, so we will move to the next selection automatically
      curSelection++;
      checkSelectionRange();
    }
  }
}

/////////////////////////////////////////////////////
/*
 * Main code for the Tetris game
 */
//Maximum size of bricks. Individual bricks can still be smaller (eg 3x3)
#define  MAX_BRICK_SIZE    4
#define  BRICKOFFSET       -1 // Y offset for new bricks

#define  INIT_SPEED        1000//Initial delay in ms between brick drops
#define  SPEED_STEP        100  // Factor for speed increase between levels, default 10
#define  LEVELUP           2 //Number of rows before levelup, default 5

// Playing field
struct Field{
  uint8_t pix[SHORT_SIDE][LONG_SIDE+1];//Make field one larger so that collision detection with bottom of field can be done in a uniform way
  unsigned int color[SHORT_SIDE][LONG_SIDE];
};
Field field;

//Structure to represent active brick on screen
struct Brick{
  boolean enabled;//Brick is disabled when it has landed
  int xpos,ypos;
  int yOffset;//Y-offset to use when placing brick at top of field
  uint8_t siz;
  uint8_t pix[MAX_BRICK_SIZE][MAX_BRICK_SIZE];

  unsigned int color;
};
Brick activeBrick;

//Struct to contain the different choices of blocks
struct AbstractBrick{
  int yOffset;//Y-offset to use when placing brick at top of field
  uint8_t siz;
  uint8_t pix[MAX_BRICK_SIZE][MAX_BRICK_SIZE];
};

//Brick "library"
AbstractBrick brickLib[7] = {
  {
      1,//yoffset when adding brick to field
      4,
      { {0,0,0,0},
        {0,1,1,0},
        {0,1,1,0},
        {0,0,0,0}
      }
  },
  {
      0,
      4,
      { {0,1,0,0},
        {0,1,0,0},
        {0,1,0,0},
        {0,1,0,0}
      }
  },
  {
      1,
      3,
      { {0,0,0,0},
        {1,1,1,0},
        {0,0,1,0},
        {0,0,0,0}
      }
  },
  {
      1,
      3,
      { {0,0,1,0},
        {1,1,1,0},
        {0,0,0,0},
        {0,0,0,0}
      }
  },
  {
      1,
      3,
      { {0,0,0,0},
        {1,1,1,0},
        {0,1,0,0},
        {0,0,0,0}
      }
  },
  {
      1,
      3,
      { {0,1,1,0},
        {1,1,0,0},
        {0,0,0,0},
        {0,0,0,0}
      }
  },
  {
      1,
      3,
      { {1,1,0,0},
        {0,1,1,0},
        {0,0,0,0},
        {0,0,0,0}
      }
  }
};

uint16_t brickSpeed;
uint8_t nbRowsThisLevel;
uint16_t nbRowsTotal;

boolean tetrisGameOver;

void tetrisInit(){
  clearField();
  brickSpeed = INIT_SPEED;
  nbRowsThisLevel = 0;
  nbRowsTotal = 0;
  tetrisGameOver = false;
  
  newActiveBrick();
}

boolean tetrisRunning = false;
void runTetris(void){
  tetrisInit();
  
  unsigned long prevUpdateTime = 0;
  
  tetrisRunning = true;
  while(tetrisRunning){
    unsigned long curTime;
    do{
      readInput();
      if (curControl & BTN_UP) delay (400);
      if (curControl & BTN_DOWN) delay (100);
      if (curControl & BTN_LEFT || curControl & BTN_RIGHT) delay (75);
      if (curControl != BTN_NONE){
        playerControlActiveBrick();
        printField();
      }
      if (tetrisGameOver) break;
 
      curTime = millis();
    } while ((curTime - prevUpdateTime) < brickSpeed);//Once enough time  has passed, proceed. The lower this number, the faster the game is
    prevUpdateTime = curTime;
  
    if (tetrisGameOver){
      fadeOut();
      char buf[4];
      int len = sprintf(buf, "%i", nbRowsTotal);
      
//      scrollTextBlocked(buf,len,WHITE);
      scrollText3 (buf, 7, 0, PrintCol);
      
      //Disable loop and exit to main menu of led table
      tetrisRunning = false;
      break;
    }
  
    //If brick is still "on the loose", then move it down by one
    if (activeBrick.enabled){
      shiftActiveBrick(DIR_DOWN);
    } else {
      //Active brick has "crashed", check for full lines
      //and create new brick at top of field
      checkFullLines();
      newActiveBrick();
      prevUpdateTime = millis();//Reset update time to avoid brick dropping two spaces
    }
    printField();
  }
  
  fadeOut();
  displayLogo();
}

void playerControlActiveBrick(){
  switch(curControl){
    case BTN_LEFT:
      shiftActiveBrick(DIR_LEFT);
      break;
    case BTN_RIGHT:
      shiftActiveBrick(DIR_RIGHT);
      break;
    case BTN_DOWN:
      shiftActiveBrick(DIR_DOWN);
      break;
    case BTN_UP:
      rotateActiveBrick();
      break;
    case BTN_EXIT:
      tetrisRunning = false;
      break;
  }
}

void printField(){
  int x,y;
  for (x=0;x<SHORT_SIDE;x++){
    for (y=0;y<LONG_SIDE;y++){
      uint8_t activeBrickPix = 0;
      if (activeBrick.enabled){//Only draw brick if it is enabled
        //Now check if brick is "in view"
        if ((x>=activeBrick.xpos) && (x<(activeBrick.xpos+(activeBrick.siz)))
            && (y>=activeBrick.ypos) && (y<(activeBrick.ypos+(activeBrick.siz)))){
          activeBrickPix = (activeBrick.pix)[x-activeBrick.xpos][y-activeBrick.ypos];
        }
      }
      if (field.pix[x][y] == 1){
        setTablePixelv(x,y, field.color[x][y]);
      } else if (activeBrickPix == 1){
        setTablePixelv(x,y, activeBrick.color);
      } else {
        setTablePixelv(x,y, 0x000000);
      }
    }
  }
  showPixels();
}

/* *** Game functions *** */

void newActiveBrick(){
//  uint8_t selectedBrick = 3;
  uint8_t selectedBrick = random(7);
  uint8_t selectedColor = random(3);

  //Set properties of brick
  activeBrick.siz = brickLib[selectedBrick].siz;
  activeBrick.yOffset = brickLib[selectedBrick].yOffset;
  activeBrick.xpos = SHORT_SIDE/2 - activeBrick.siz/2;
  activeBrick.ypos = BRICKOFFSET-activeBrick.yOffset;
  activeBrick.enabled = true;
  
  //Set color of brick
  activeBrick.color = colorLib[selectedColor];
  //activeBrick.color = colorLib[1];
  
  //Copy pix array of selected Brick
  uint8_t x,y;
  for (y=0;y<MAX_BRICK_SIZE;y++){
    for (x=0;x<MAX_BRICK_SIZE;x++){
      activeBrick.pix[x][y] = (brickLib[selectedBrick]).pix[x][y];
    }
  }
  
  //Check collision, if already, then game is over
  if (checkFieldCollision(&activeBrick)){
    tetrisGameOver = true;
  }
}

//Check collision between bricks in the field and the specified brick
boolean checkFieldCollision(struct Brick* brick){
  uint8_t bx,by;
  uint8_t fx,fy;
  for (by=0;by<MAX_BRICK_SIZE;by++){
    for (bx=0;bx<MAX_BRICK_SIZE;bx++){
      fx = (*brick).xpos + bx;
      fy = (*brick).ypos + by;
      if (( (*brick).pix[bx][by] == 1) 
            && ( field.pix[fx][fy] == 1)){
        return true;
      }
    }
  }
  return false;
}

//Check collision between specified brick and all sides of the playing field
boolean checkSidesCollision(struct Brick* brick){
  //Check vertical collision with sides of field
  uint8_t bx,by;
  uint8_t fx,fy;
  for (by=0;by<MAX_BRICK_SIZE;by++){
    for (bx=0;bx<MAX_BRICK_SIZE;bx++){
      if ( (*brick).pix[bx][by] == 1){
        fx = (*brick).xpos + bx;//Determine actual position in the field of the current pix of the brick
        fy = (*brick).ypos + by;
        if (fx<0 || fx>=SHORT_SIDE){
          return true;
        }
      }
    }
  }
  return false;
}

Brick tmpBrick;

void rotateActiveBrick(){
  //Copy active brick pix array to temporary pix array
  uint8_t x,y;
  for (y=0;y<MAX_BRICK_SIZE;y++){
    for (x=0;x<MAX_BRICK_SIZE;x++){
      tmpBrick.pix[x][y] = activeBrick.pix[x][y];
    }
  }
  tmpBrick.xpos = activeBrick.xpos;
  tmpBrick.ypos = activeBrick.ypos;
  tmpBrick.siz = activeBrick.siz;
  
  //Depending on size of the active brick, we will rotate differently
  if (activeBrick.siz == 3){
    //Perform rotation around center pix
    tmpBrick.pix[0][0] = activeBrick.pix[0][2];
    tmpBrick.pix[0][1] = activeBrick.pix[1][2];
    tmpBrick.pix[0][2] = activeBrick.pix[2][2];
    tmpBrick.pix[1][0] = activeBrick.pix[0][1];
    tmpBrick.pix[1][1] = activeBrick.pix[1][1];
    tmpBrick.pix[1][2] = activeBrick.pix[2][1];
    tmpBrick.pix[2][0] = activeBrick.pix[0][0];
    tmpBrick.pix[2][1] = activeBrick.pix[1][0];
    tmpBrick.pix[2][2] = activeBrick.pix[2][0];
    //Keep other parts of temporary block clear
    tmpBrick.pix[0][3] = 0;
    tmpBrick.pix[1][3] = 0;
    tmpBrick.pix[2][3] = 0;
    tmpBrick.pix[3][3] = 0;
    tmpBrick.pix[3][2] = 0;
    tmpBrick.pix[3][1] = 0;
    tmpBrick.pix[3][0] = 0;
    
  } else if (activeBrick.siz == 4){
    //Perform rotation around center "cross"
    tmpBrick.pix[0][0] = activeBrick.pix[0][3];
    tmpBrick.pix[0][1] = activeBrick.pix[1][3];
    tmpBrick.pix[0][2] = activeBrick.pix[2][3];
    tmpBrick.pix[0][3] = activeBrick.pix[3][3];
    tmpBrick.pix[1][0] = activeBrick.pix[0][2];
    tmpBrick.pix[1][1] = activeBrick.pix[1][2];
    tmpBrick.pix[1][2] = activeBrick.pix[2][2];
    tmpBrick.pix[1][3] = activeBrick.pix[3][2];
    tmpBrick.pix[2][0] = activeBrick.pix[0][1];
    tmpBrick.pix[2][1] = activeBrick.pix[1][1];
    tmpBrick.pix[2][2] = activeBrick.pix[2][1];
    tmpBrick.pix[2][3] = activeBrick.pix[3][1];
    tmpBrick.pix[3][0] = activeBrick.pix[0][0];
    tmpBrick.pix[3][1] = activeBrick.pix[1][0];
    tmpBrick.pix[3][2] = activeBrick.pix[2][0];
    tmpBrick.pix[3][3] = activeBrick.pix[3][0];
  } else {
    Serial.println("Brick size error");
  }
  
  //Now validate by checking collision.
  //Collision possibilities:
  //      -Brick now sticks outside field
  //      -Brick now sticks inside fixed bricks of field
  //In case of collision, we just discard the rotated temporary brick
  if ((!checkSidesCollision(&tmpBrick)) && (!checkFieldCollision(&tmpBrick))){
    //Copy temporary brick pix array to active pix array
    for (y=0;y<MAX_BRICK_SIZE;y++){
      for (x=0;x<MAX_BRICK_SIZE;x++){
        activeBrick.pix[x][y] = tmpBrick.pix[x][y];
      }
    }
  }
}

//Shift brick left/right/down by one if possible
void shiftActiveBrick(int dir){
  //Change position of active brick (no copy to temporary needed)
  if (dir == DIR_LEFT){
    activeBrick.xpos--;
  } else if (dir == DIR_RIGHT){
    activeBrick.xpos++;
  } else if (dir == DIR_DOWN){
    activeBrick.ypos++;
  }
  
  //Check position of active brick
  //Two possibilities when collision is detected:
  //    -Direction was LEFT/RIGHT, just revert position back
  //    -Direction was DOWN, revert position and fix block to field on collision
  //When no collision, keep activeBrick coordinates
  if ((checkSidesCollision(&activeBrick)) || (checkFieldCollision(&activeBrick))){
    //Serial.println("coll");
    if (dir == DIR_LEFT){
      activeBrick.xpos++;
    } else if (dir == DIR_RIGHT){
      activeBrick.xpos--;
    } else if (dir == DIR_DOWN){
      activeBrick.ypos--;//Go back up one
      addActiveBrickToField();
      activeBrick.enabled = false;//Disable brick, it is no longer moving
    }
  }
}

//Copy active pixels to field, including color
void addActiveBrickToField(){
  uint8_t bx,by;
  uint8_t fx,fy;
  for (by=0;by<MAX_BRICK_SIZE;by++){
    for (bx=0;bx<MAX_BRICK_SIZE;bx++){
      fx = activeBrick.xpos + bx;
      fy = activeBrick.ypos + by;
      
      if (fx>=0 && fy>=0 && fx<SHORT_SIDE && fy<LONG_SIDE && activeBrick.pix[bx][by]){//Check if inside playing field
        //field.pix[fx][fy] = field.pix[fx][fy] || activeBrick.pix[bx][by];
        field.pix[fx][fy] = activeBrick.pix[bx][by];
        field.color[fx][fy] = activeBrick.color;
      }
    }
  }
}

//Move all pix from te field above startRow down by one. startRow is overwritten
void moveFieldDownOne(uint8_t startRow){
  if (startRow == 0){//Topmost row has nothing on top to move...
    return;
  }
  uint8_t x,y;
  for (y=startRow-1; y>0; y--){
    for (x=0;x<SHORT_SIDE; x++){
      field.pix[x][y+1] = field.pix[x][y];
      field.color[x][y+1] = field.color[x][y];
    }
  }
}

void checkFullLines(){
  int x,y;
  int minY = 0;
  for (y=(LONG_SIDE-1); y>=minY; y--){
    uint8_t rowSum = 0;
    for (x=0; x<SHORT_SIDE; x++){
      rowSum = rowSum + (field.pix[x][y]);
    }
    if (rowSum>=SHORT_SIDE){
      //Found full row, animate its removal
      for (x=0;x<SHORT_SIDE; x++){
        field.pix[x][y] = 0;
        printField();
        delay(100);
      }
      //Move all upper rows down by one
      moveFieldDownOne(y);
      y++; minY++;
      printField();
      delay(100);
      
      nbRowsThisLevel++; nbRowsTotal++;
      if (nbRowsThisLevel >= LEVELUP){
        nbRowsThisLevel = 0;
        brickSpeed = brickSpeed - SPEED_STEP;
        if (brickSpeed<200){
          brickSpeed = 200;
        }
      }
    }
  }
}

void clearField(){
  uint8_t x,y;
  for (y=0;y<LONG_SIDE;y++){
    for (x=0;x<SHORT_SIDE;x++){
      field.pix[x][y] = 0;
      field.color[x][y] = 0;
    }
  }
  for (x=0;x<SHORT_SIDE;x++){//This last row is invisible to the player and only used for the collision detection routine
    field.pix[x][LONG_SIDE] = 1;
  }
}

/////////////////////////////////////////////////////

void loop() {
  // put your main code here, to run repeatedly:
//  runDP();
  mainLoop();
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

/*
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
*/
/*
  void     begin(uint16_t id = 0x9325);
  void     drawPixel(int16_t x, int16_t y, uint16_t color);
  void     drawFastHLine(int16_t x0, int16_t y0, int16_t w, uint16_t color);
  void     drawFastVLine(int16_t x0, int16_t y0, int16_t h, uint16_t color);
  void     fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t c);
  void     fillScreen(uint16_t color);
  void     reset(void);
  void     setRegisters8(uint8_t *ptr, uint8_t n);
  void     setRegisters16(uint16_t *ptr, uint8_t n);
  void     setRotation(uint8_t x);
       // These methods are public in order for BMP examples to work:
  void     setAddrWindow(int x1, int y1, int x2, int y2);
  void     pushColors(uint16_t *data, uint8_t len, boolean first);

  uint16_t color565(uint8_t r, uint8_t g, uint8_t b),
           readPixel(int16_t x, int16_t y),
           readID(void);
  uint32_t readReg(uint8_t r);

 * 
 */
