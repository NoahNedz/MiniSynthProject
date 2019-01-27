#include <SPI.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789

//tft screen pinout -> SCK-clk, SI->Data,TCS-CS,RST-RST,DC-DC
#define TFT_CS     A1
#define TFT_DC     A0
#define TFT_RST    A2  // you can also connect this to the Arduino reset
                       // in which case, set this #define pin to -1!

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

#define G   8
#define F   7
#define E   6
#define D   5
#define C   4
#define B   3
#define A   2


float noteStatus1[] = {0,0,0,0,0,0,0};
float noteStatus2[] = {0,0,0,0,0,0,0};
float noteStatus3[] = {0,0,0,0,0,0,0};
float noteArray[] = {G,F,E,D,C,B,A};
float hertzArray[] = {392,349,330,294,262,247,220};


#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define GREEN    0x07E0
#define CYAN     0x07FF
#define MAGENTA  0xF81F
#define YELLOW   0xFFE0 
// For 1.44" and 1.8" TFT with ST7735 use

const int SINE = 0x2000;                    // Define AD9833's waveform register value.
const int SQUARE = 0x2028;                  // When we update the frequency, we need to
const int TRIANGLE = 0x2002;
                     // Standard SPI pins for the AD9833 waveform generator.
const int CLK = 13;                         // CLK and DATA pins are shared with the TFT display.
const int DATA = 11;
const int FSYNC0 = 10;                       // Standard SPI pins for the AD9833 waveform generator.
const int FSYNC1 = 9;
const int FSYNC2 = A4;

//SPI: 10 (SS), 11 (MOSI), 12 (MISO), 13 (SCK)
const float refFreq = 25000000.0;
int waveType = 0;
int wave = 0;

int selection = 0;

int j = 0;
int osc0 = 0;
int osc1 = 0;
int osc2 = 0;



int used[] = {0,0,0};
void setup() {
  // put your setup code here, to run once:

pinMode(G,INPUT);
pinMode(F,INPUT);
pinMode(E,INPUT);
pinMode(D,INPUT);
pinMode(C,INPUT);
pinMode(B,INPUT);
pinMode(A,INPUT);
pinMode(A7,INPUT);
pinMode(A6,OUTPUT);
pinMode(FSYNC0,OUTPUT);
pinMode(FSYNC1,OUTPUT);
pinMode(FSYNC2,OUTPUT);

SPI.begin();
delay(50); 
tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab
tft.fillScreen(BLACK);
delay(50);
AD9833reset(FSYNC0);                                   // Reset AD9833 module after power-up.
AD9833reset(FSYNC1);
AD9833reset(FSYNC2);
delay(50);


}

void loop() {
  wave = analogRead(A7);
  if(wave <= 50){
    waveType = SQUARE;
  }
  else if(wave <= 250){
    waveType = TRIANGLE;
  }
  else
  waveType = SINE;

 
  
  for(int i = 0; i < 7; i++){
  int currentNoteNew1 = digitalRead(noteArray[i]);
 
    if(currentNoteNew1 != noteStatus1[i]){
      
      noteStatus1[i] = currentNoteNew1;

      if(noteStatus1[i] == 1){
        if(used[0] == 0){
          selection = FSYNC0;
          osc0 = i;
          used[0] = 1;
        }
        else if(used[1] == 0){
          selection = FSYNC1;
          osc1 = i;
          used[1] = 1;
        }
        else if(used[2] == 0){
          selection = FSYNC2;
          osc2 = i;
          used[2] = 1;
        }
        AD9833setFrequency(hertzArray[i],waveType,selection);
        popScreen(hertzArray[i],waveType,selection);
      }
      
      else{
        if(i == osc0){
          selection = FSYNC0;
          used[0] = 0;
        }
        else if(i == osc1){
          selection = FSYNC1;
          used[1] = 0;
        }
        else if(i == osc2){
          selection = FSYNC2;
          used[2] = 0;
        }
        
        AD9833reset(selection);
        
      }
    }
  }
}
void AD9833reset(int selection) {
 
  WriteRegister(0x100, selection);   // Write '1' to AD9833 Control register bit D8.
  delay(10);
}

// Set the frequency and waveform registers in the AD9833.
void AD9833setFrequency(long frequency, int Waveform, int selection) {

  long FreqWord = (frequency * pow(2, 28)) / refFreq;

  int MSB = (int)((FreqWord & 0xFFFC000) >> 14);    //Only lower 14 bits are used for data
  int LSB = (int)(FreqWord & 0x3FFF);
  
  //Set control bits 15 ande 14 to 0 and 1, respectively, for frequency register 0
  LSB |= 0x4000;
  MSB |= 0x4000; 
  
  WriteRegister(0x2100,selection);   
  WriteRegister(LSB,selection);                  // Write lower 16 bits to AD9833 registers
  WriteRegister(MSB,selection);                  // Write upper 16 bits to AD9833 registers.
  WriteRegister(0xC000,selection);               // Phase register
  WriteRegister(Waveform,selection);             // Exit & Reset to SINE, SQUARE or TRIANGLE

}

void WriteRegister(int dat, int selection) { 
  
  // Display and AD9833 use different SPI MODES so it has to be set for the AD9833 here.
        
  SPI.setDataMode(SPI_MODE2); 
  digitalWrite(selection, LOW);           // Set FSYNC low before writing to AD9833 registers
  
  delayMicroseconds(10);              // Give AD9833 time to get ready to receive data.
  
  SPI.transfer(highByte(dat));        // Each AD9833 register is 32 bits wide and each 16
  SPI.transfer(lowByte(dat));         // bits has to be transferred as 2 x 8-bit bytes.

  digitalWrite(selection, HIGH);          //Write done. Set FSYNC high
}

void popScreen(unsigned long note, int waveType, int selection) {
  SPI.setDataMode(SPI_MODE0);   
  tft.setTextWrap(false);
    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(10, 30);
    tft.setTextColor(ST77XX_GREEN);
    tft.setTextSize(4);
    tft.println(String(note));
    tft.setCursor(10, 80);
    tft.setTextColor(ST77XX_BLUE);
    tft.setTextSize(2);
    
    if(waveType == SINE)
    tft.println("SINE");
    else if(waveType == TRIANGLE)
    tft.println("TRIANGLE");
    else
    tft.println("SQUARE");
    tft.setCursor(10, 100);
    tft.println(String(selection));
}
