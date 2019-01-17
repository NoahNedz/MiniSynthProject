#include <SPI.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#define G   8
#define F   7
#define E   6
#define D   5
#define C   4
#define B   3
#define A   2


float noteStatus[] = {0,0,0,0,0,0,0};
float noteArray[] = {G,F,E,D,C,B,A};

#define TFT_CS     A1
#define TFT_DC     A0
#define TFT_RST    A2  // you can also connect this to the Arduino reset
                       // in which case, set this #define pin to -1!

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

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
const int FSYNC = 10;                       // Standard SPI pins for the AD9833 waveform generator.



//SPI: 10 (SS), 11 (MOSI), 12 (MISO), 13 (SCK)
const float refFreq = 25000000.0;
int waveType = SQUARE;



void setup() {
  // put your setup code here, to run once:

pinMode(G,INPUT);
pinMode(F,INPUT);
pinMode(E,INPUT);
pinMode(D,INPUT);
pinMode(C,INPUT);
pinMode(B,INPUT);
pinMode(A,INPUT);
SPI.begin();
delay(50); 
tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab
tft.fillScreen(BLACK);
delay(50);
AD9833reset();                                   // Reset AD9833 module after power-up.
delay(50);
//AD9833setFrequency(420, SINE); 

}

void loop() {
  for(int i = 0; i < 7; i++){
  int currentNoteNew = digitalRead(noteArray[i]);
  if(currentNoteNew != noteStatus[i]){
    noteStatus[i] = currentNoteNew;
  
    switch(i){
      case 0:
      if(noteStatus[i] == 1)
        AD9833setFrequency(587, waveType);
        else
          AD9833reset();
        delay(10);
        popScreen(587);
        break;
      case 1:
      if(noteStatus[i] == 1)
        AD9833setFrequency(523, waveType);
        else
          AD9833reset();
        delay(10);
        popScreen(523);
        break;
      case 2:
      if(noteStatus[i] == 1)
        AD9833setFrequency(494, waveType);
        else
        AD9833reset();
        delay(10);
        popScreen(494);
        break;
      case 3:
      if(noteStatus[i] == 1)
        AD9833setFrequency(440, waveType);
        else
          AD9833reset();
        delay(10);
        popScreen(440);
        break;
      case 4:
      if(noteStatus[i] == 1)
        AD9833setFrequency(392, waveType);
        else
        AD9833reset();
        delay(10);
        popScreen(392);
        break;
      case 5:
      if(noteStatus[i] == 1)
        AD9833setFrequency(370, waveType);
        else
        AD9833reset();
        delay(10);
        popScreen(370);
        break;
      case 6:
      if(noteStatus[i] == 1)
        AD9833setFrequency(330, waveType);
        else
        AD9833reset();
        delay(10);
        popScreen(330);
        break;
      default:
      break;
  
      }
    }
  }
}
void AD9833reset() {
 
  WriteRegister(0x100);   // Write '1' to AD9833 Control register bit D8.
  delay(10);
}

// Set the frequency and waveform registers in the AD9833.
void AD9833setFrequency(long frequency, int Waveform) {

  long FreqWord = (frequency * pow(2, 28)) / refFreq;

  int MSB = (int)((FreqWord & 0xFFFC000) >> 14);    //Only lower 14 bits are used for data
  int LSB = (int)(FreqWord & 0x3FFF);
  
  //Set control bits 15 ande 14 to 0 and 1, respectively, for frequency register 0
  LSB |= 0x4000;
  MSB |= 0x4000; 
  
  WriteRegister(0x2100);   
  WriteRegister(LSB);                  // Write lower 16 bits to AD9833 registers
  WriteRegister(MSB);                  // Write upper 16 bits to AD9833 registers.
  WriteRegister(0xC000);               // Phase register
  WriteRegister(Waveform);             // Exit & Reset to SINE, SQUARE or TRIANGLE

}

void WriteRegister(int dat) { 
  
  // Display and AD9833 use different SPI MODES so it has to be set for the AD9833 here.
        
  SPI.setDataMode(SPI_MODE2); 
  digitalWrite(FSYNC, LOW);           // Set FSYNC low before writing to AD9833 registers
  delayMicroseconds(10);              // Give AD9833 time to get ready to receive data.
  
  SPI.transfer(highByte(dat));        // Each AD9833 register is 32 bits wide and each 16
  SPI.transfer(lowByte(dat));         // bits has to be transferred as 2 x 8-bit bytes.

  digitalWrite(FSYNC, HIGH);          //Write done. Set FSYNC high
}

void popScreen(unsigned long note) {
  SPI.setDataMode(SPI_MODE0);   
  tft.setTextWrap(false);
    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(10, 30);
    tft.setTextColor(ST77XX_GREEN);
    tft.setTextSize(4);
    tft.println(String(note));
    
}
