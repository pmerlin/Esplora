 
#define SD_CS    8

#include <Esplora.h>    
#include <SD.h>  // include before TFT.h
#include <TFT.h>  

int pcmandirection = 0;
int oldx = 1;
int oldy = 1;
int px = 7; 
int py = 7;
int dottoeat = 105;
unsigned long tm = millis();

int monsters[3][5]{{7,5,7,5,1},{6,5,6,5,1},{7,6,7,6,1}}; //x,y,oldx,oldy values for three monsters

int positionsY[29][4] = {{1,6,49,2},{1,126,49,2},
                      {13,42,12,2},{13,66,12,2},{13,90,12,2},
                      {25,30,12,2},{25,102,12,2},
                      {37,66,12,2},
                      {49,30,24,2},{49,42,24,2},{49,90,24,2},{49,102,24,2},
                      {61,54,36,2},{61,78,36,2},
                      {85,30,24,2},{85,42,36,2},{85,90,36,2},{85,102,24,2},
                      {109,6,48,2},{109,66,12,2},{109,126,48,2},
                      {121,102,12,2},
                      {133,18,12,2},{133,30,12,2},{133,42,12,2},{133,54,12,2},{133,66,24,2},{133,78,12,2},{133,90,12,2}
                      };
int positionsX[35][4] = {{1,6,2,126},
                      {13,18,2,36},{13,78,2,36},
                      {25,6,2,12},{25,54,2,24},{25,114,2,12},
                      {37,18,2,14},{37,42,2,12},{37,78,2,12},{37,102,2,12},
                      {49,6,2,24},{49,54,2,24},{49,102,2,26},
                      {61,54,2,24},
                      {73,6,2,26},{73,102,2,26},
                      {85,6,2,26},{85,102,2,26},
                      {97,54,2,6},{97,72,2,8},
                      {109,6,2,26},{109,42,2,12},{109,78,2,12},{109,102,2,24},
                      {121,18,2,12},{121,54,2,24},{121,102,2,12},
                      {133,18,2,12},{133,42,2,12},{133,78,2,12},{133,114,2,12},
                      {145,18,2,14},{145,42,2,14},{145,78,2,36},
                      {157,6,2,126}         
                      };
 int matrix[10][13]  {{B10011,B11010,B10011,B11010,B00000,B00000,B10110,B00000,B00000,B10011,B11011,B11001,B11010},
                      {B10111,B10111,B11100,B10110,B00000,B00000,B10110,B00000,B00000,B10110,B10110,B00000,B10110},
                      {B10110,B10101,B11011,B11111,B11001,B11001,B11111,B11001,B11001,B11101,B11111,B11001,B11110},
                      {B10110,B10011,B11110,B10111,B11011,B11001,B11101,B11001,B11010,B10011,B11110,B00000,B10110},
                      {B10111,B11100,B10111,B11100,B10110,B00011,B00011,B00011,B10111,B11100,B10111,B11001,B11100},
                      {B10111,B11010,B10111,B11010,B10110,B00101,B00101,B00101,B10111,B11010,B10111,B11001,B11010},
                      {B10110,B10101,B11110,B10111,B11101,B11001,B11011,B11001,B11100,B10101,B11110,B00000,B10110},
                      {B10110,B10011,B11101,B11111,B11001,B11001,B11111,B11001,B11001,B11011,B11101,B11010,B10110},
                      {B10110,B10111,B11010,B10110,B00000,B00000,B10110,B00000,B00000,B10110,B10011,B11110,B10110},
                      {B10101,B11100,B10101,B11100,B00000,B00000,B10110,B00000,B00000,B10101,B11100,B10101,B11100}
                     };

void Reset() 
{
  asm volatile ("  jmp 0");  
}  

void platform()
{
  EsploraTFT.fill(255, 255, 255);
  for(int i=0;i<35;i++)
  {
    EsploraTFT.rect(positionsX[i][0], positionsX[i][1], positionsX[i][2],positionsX[i][3]);
  }

  for(int i=0;i<29;i++)
  {
    EsploraTFT.rect(positionsY[i][0], positionsY[i][1], positionsY[i][2],positionsY[i][3]);
  }
}
void drawdot(int ix, int iy)
{
 
 if ((matrix[iy][ix]&B10000)==B10000)
 {
    EsploraTFT.fill(255, 255, 255);
    ix++;
    iy++;
    int x = ix*12+2-6;
    int y = iy*12;
    EsploraTFT.circle(x,y,1);
  }
 
}

void drawdots()
{
  
  for(int n=0;n<13;n++)
    for(int k = 0;k<10;k++)
    {
      drawdot(n,k); 
    }
}

void drawpack(int ix, int iy, int R, int G, int B)
{
  if  ((matrix[iy][ix] & B10000)== B10000)
  {
    matrix[iy][ix] = matrix[iy][ix] & B1111;
    dottoeat--;
    Serial.println(dottoeat);
    if (dottoeat == 0)
    {
        Esplora.writeGreen(75);
        delay(2000);
        Esplora.writeGreen(0);
        Reset();
    }
  }
  ix++;
  iy++;
  EsploraTFT.fill(R,G,B);
  int x = ix*12+2-6;
  int y = iy*12;
  EsploraTFT.circle(x,y,4);
}
void drawmonster(int ix, int iy, int R, int G, int B)
{
  if ((ix==px) &&(iy==py))
  {
    Esplora.writeRed(75);
    delay(2000);
    Esplora.writeRed(0);
    Reset();
  }
  ix++;
  iy++;
  EsploraTFT.fill(R,G,B);
  int x = ix*12-8;
  int y = iy*12-4;
  EsploraTFT.rect(x,y,8,8);
}
void monsterposition()
{

     for(int i=0;i<3;i++)
     {
      int d = 0;
      int cn = 0;
      do
      {
        int lx = monsters[i][0];
        int ly = monsters[i][1];
        monsters[i][2]=lx;
        monsters[i][3]=ly;
        d = monsters[i][4];

        if ((matrix[ly][lx]&B0001)==B0001 && d==1){lx++;d=0;}
        if ((matrix[ly][lx]&B1000)==B1000 && d==4){lx--;d=0;}
        if ((matrix[ly][lx]&B0010)==B0010 && d==2){ly++;d=0;}
        if ((matrix[ly][lx]&B0100)==B0100 && d==3) {ly--;d=0;}
        lx=lx%13;
        ly=ly%13;
        monsters[i][0]=lx;
        monsters[i][1]=ly;
     
        if (d!=0) monsters[i][4] = random(1, 5);
        cn++;
      }
      while ((d!=0) && (cn < 4));

      int R=0,G=0,B=0;
      if (i==0) R=255;
      if (i==1) G=255;
      if (i==2) B=255;
      drawmonster(monsters[i][2],monsters[i][3],0,0,0);
      drawdot(monsters[i][2],monsters[i][3]);
      drawmonster(monsters[i][0],monsters[i][1],R,G,B);
     }
}
 

void setup() {
  Serial.begin (115200);
  Serial.println("init");
  EsploraTFT.begin();
  EsploraTFT.background(0,0,0);
  drawdots();
  platform();
  drawpack(px,py, 0,200,200);
  oldx = px;
  oldy = py;
  Esplora.writeRed(0);  
  Esplora.writeGreen(0);  
  Esplora.writeBlue(0); 
 
}

int ReadButton()
{
  if (Esplora.readButton(SWITCH_DOWN) == LOW) return 2;
  if (Esplora.readButton(SWITCH_LEFT) == LOW) return 4;
  if (Esplora.readButton(SWITCH_UP) == LOW) return 3;
  if (Esplora.readButton(SWITCH_RIGHT) == LOW) return 1;
  int jx = Esplora.readJoystickX();
  int jy = Esplora.readJoystickY();
  if (abs(jx) > abs(jy))
  {
    if ((abs(jx)>50) && (jx > 0)) return 4;
    if ((abs(jx)>50) && (jx < 0)) return 1;
  }
  else
  {
    if ((abs(jy)>50) && (jy > 0)) return 2;
    if ((abs(jy)>50) && (jy < 0)) return 3;
  }
  
  return pcmandirection;
}
void loop() {

  if ((px != oldx) || (py != oldy)) 
  {
    drawpack(oldx,oldy, 0,0,0);
    drawpack(px,py, 0,200,200);  
    oldx=px;
    oldy=py;    
  }
  int df = millis() - tm;
  if (df < 0) df +=1000; 
  if (df > 300)
  {
    tm = millis();
    monsterposition();
  }
  int d = ReadButton();
  if ((d == 1) && ((matrix[oldy][oldx]&B0001) == B0001))px++;
  if ((d == 4) && ((matrix[oldy][oldx]&B1000) == B1000)) px--;
  
  if ((d == 2) && ((matrix[oldy][oldx]&B0010) == B0010)) py++;
  if ((d == 3) && ((matrix[oldy][oldx]&B0100) == B0100)) py--;
  if (py <0) py = 9;
  px = px%13;
  py = py%10;

 
  if ((px != oldx) || (py != oldy)) 
  {
    pcmandirection = d;
    delay(300); 
  }
}
