/*
 *  TeensyZ80 by Colin "Domipheus" Riley
 *  
 *  This implements a Clock, RAM/ROM, Serial Device, Console Output,
 *  and Mode 2 Interrupts. There is a 16KB global array which should
 *  be initialized to the 'ROM', but in essence this is all ram and
 *  the whole 16KB is read/writable. The Serial device and Console 
 *  is implemented via I/O ports.
 *
 *  This is intended for educational purposes - it is fully in-sync,
 *  in that everything occurs in a clock. The Z80 is clocked fully
 *  in step with operations in the loop() functions and as such the 
 *  Z80 WAIT line is not required. This has the side effect of being 
 *  fairly slow, but at ~60KHz is responsive enough to run some 
 *  programs.
 *
 *  This code is in flux, so may have unfinished features etc.
 *
 *  ================================================================
 */ 
#include <Adafruit_GFX.h>
#include <ILI9341_t3.h>

/*
 *  SPI pin definitions
 *  Don't change! First block is the TFT pins. 
 *  Second block is for SD card chip select. Bus shared.
 *  =======================================================
 */ 
#include <SPI.h>
#define sclk 13
#define mosi 11
#define miso 12
#define cs   10
#define dc   9


#include <SD.h>
#define sdcs 8

/*
 *  Z80 pin definitions
 *  Some analog pins exist here, so the read functions are different
 *  ================================================================
 */ 
#define Z_CLK 29

#define AD0 23
#define AD1 22
#define AD2 21
#define AD3 20
#define AD4 19
#define AD5 18
#define AD6 17
#define AD7 16
#define AD8 15
#define AD9 14
#define AD10 30
#define AD11 A12
#define AD12 A11
#define AD13 A13
#define AD14 
#define AD15 

#define D0 1
#define D1 0
#define D2 3
#define D3 6
#define D4 7
#define D5 5
#define D6 4
#define D7 2

#define WR 28
#define RD 25
#define M1 32
#define MREQ 33
#define IOREQ 24
#define RFSH 27
#define RESET 26
#define INT 31

/*
 *  SPI TFT LCD 2.2" 320x240
 *  ============================
 */ 
ILI9341_t3 tft = ILI9341_t3(cs, dc); 

/*
 *  16KB RAM (there is no rom)
 *  ============================
 */ 
short ROM_LENGTH = 16384;
unsigned char TEST_ROM[16384]=
{
	0xF3,0x31,0xFF,0x3F,0xED,0x5E,0x3E,0x01,0xED,0x47,0x11,
	0x4F,0x00,0xCD,0x14,0x00,0xFB,0x76,0x18,0xFD,0xF5,0x1A,
	0xB7,0x28,0x05,0xD3,0x03,0x13,0x18,0xF7,0xF1,0xC9,0xF3,
	0xD5,0x11,0x36,0x00,0xCD,0x14,0x00,0xD1,0xFB,0xED,0x4D,
	0xF3,0xF5,0x3E,0x2E,0xD3,0x03,0xF1,0xFB,0xED,0x4D,0x55,
	0x6E,0x6B,0x6E,0x6F,0x77,0x6E,0x20,0x49,0x6E,0x74,0x65,
	0x72,0x72,0x75,0x70,0x74,0x20,0x56,0x65,0x63,0x74,0x6F,
	0x72,0x00,0x54,0x65,0x65,0x6E,0x73,0x79,0x5A,0x38,0x30,
	0x3E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x20,0x00,0x2C,0x00,0x20,0x00,0x20,0x00,
	0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,
	0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,
	0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,
	0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,
	0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,
	0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,
	0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,
	0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,
	0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,
	0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,
	0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,
	0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,
	0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,
	0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,
	0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,
	0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,
	0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,
	0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,
	0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,
	0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,
	0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,
	0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,
	0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,
};


/*
 *  Signals/Buses from CPU
 *  ======================
 */ 
byte M1_val = 0;
byte RFSH_val = 0;
byte RD_val = 0;
byte WR_val = 0;
byte MEMREQ_val = 0;
byte IOREQ_val = 0;
unsigned short addressBus = 0;
unsigned char dataBus = 0;


/*
 *  Console/Terminal Definitions
 *  ============================
 */ 
#define CONSOLE_FONTY 9
#define CONSOLE_FONTX 7
#define CONSOLE_START_Y 100
#define CONSOLE_START_X 9
#define CONSOLE_ROWS 24
#define CONSOLE_COLUMNS 32

byte           console_current_row = 0;
byte           console_current_col = 0;
unsigned short console_current_color = 0xFFFF;
byte           console_current_color_state = 0;
  
/*
 *  Vram Emulation Definitions
 *  ============================
 */ 
unsigned short ioVramBuffer = 0;
static unsigned char last_disp[CONSOLE_COLUMNS*CONSOLE_ROWS] = {0};



/*
 *  I/O Port Definitions
 *  ============================
 */ 
#define PORT_VRAM_BUFFER_LOC 0
#define PORT_SERIAL_CMD 1
#define PORT_SERIAL_DATA 2
#define PORT_DISP_PUTCHAR 3
#define PORT_DISP_SETROW 4
#define PORT_DISP_SETCOL 5
#define PORT_DISP_SETCOLOUR 6


/*
 *  Serial Communications Device
 *  ============================
 */ 
#define SERIAL_CMD_SET_INTVECTOR 0xF1
#define SERIAL_CMD_SET_RATE 0xF2
#define SERIAL_CMD_INIT 0xFA

// The set rate command takes a 16 bit value (For now) which means it 
// needs multiple cycles/io writes to complete
#define SERIAL_CMD_SET_RATE_2 0x02
#define SERIAL_CMD_READY 0

//State info needs to understand the IO timings as the commands can remain on the buses
// for many cycles
int            ioDebounce = 0;

unsigned short ioSerialIntVector = 0;
int            ioSerialInitialized = 0;
unsigned short ioSerialRate = 0;
int            ioSerialCurrentMode = SERIAL_CMD_READY;

// We dont want to constantly be flagging interupts. Only flag it if weve completed the
// previous one
int ioSerialINTDebounce = 0;


byte currentInterruptVector = 0;
int currentInterrupt = 0;



/*
 * Debug Display Options
 * =====================
 */
#define MEMORY_DISPLAY 0
#define VRAM_DISPLAY 0
#define DBG_DRAW_DATA 0
#define DBG_DRAW_ADDR 0
#define DBG_DRAW_CYCLE 1
#define DBG_DRAW_LOG 0

/*
 *  Debug Log definitions
 *  ============================
 */ 
#define GFX_ADDR_START 15
#define GFX_DATA_START 35
#define GFX_CONTROL_START 55
#define GFX_LOG_START 160
#define GFX_LOG_LINES 16
int current_line = 0;


#define DELAY 20
unsigned int cycle = 0;


// Taken from examples on Arduino site for now
Sd2Card card;
SdVolume volume;
SdFile root;

// Helper function
char* fmtstring(char *fmt, ... )
{
  static char tmp[256]; 
  va_list args; 
  va_start (args, fmt ); 
  vsnprintf(tmp, 256, fmt, args); 
  va_end (args); 
  return &tmp[0];
} 

void performZ80Reset()
{
  //Perform a reset for 4 cycles (manual says 3 should be enough).
  digitalWrite(RESET, LOW);  
  for (int i = 0; i<8; i++)
  {  
    digitalWrite(Z_CLK, HIGH);  
    delay(1);
    digitalWrite(Z_CLK, LOW);  
    delay(1);
  }
  //bring CPU out of reset state
  digitalWrite(RESET, HIGH); 
}

void updateZ80Control()
{
 M1_val = digitalRead(M1)==LOW?1:0; 
 RFSH_val = digitalRead(RFSH)==LOW?1:0; 
 RD_val = digitalRead(RD)==LOW?1:0; 
 WR_val = digitalRead(WR)==LOW?1:0; 
 MEMREQ_val = digitalRead(MREQ)==LOW?1:0; 
 IOREQ_val = digitalRead(IOREQ)==LOW?1:0; 
}

void setup(void) {
  
  Serial.begin(9600);
  tft.begin();
  
  tft.setRotation(0);	// 0 - Portrait, 1 - Lanscape
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextWrap(true);

  delay(10);
  
  // Test starting the SD card
  // We dont do anything else with it just now. May need 
  // to ensure the SPI commands are working after the tft
  // is used after this code.
  pinMode(sdcs, OUTPUT);
  //if (!SD.begin(sdcs)) {
  if (!card.init(SPI_HALF_SPEED, sdcs))
  {
    tft.setCursor (20, 20);
    tft.println("Card failed, or not present");
    // don't do anything more:
  } 
  else
  {
    
    tft.setCursor (20, 20);
    tft.println("SD Card OK");
    
    // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
    if (!volume.init(card)) 
    {
      tft.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
    }
    else 
    {
      // print the type and size of the first FAT-type volume
      uint32_t volumesize;
      tft.print("\nVolume type is FAT");
      tft.println(volume.fatType(), DEC);
      tft.println();
     
      volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
      volumesize *= volume.clusterCount();       // we'll have a lot of clusters
      volumesize *= 512;                            // SD card blocks are always 512 bytes
      tft.print("Volume size (bytes): ");
      tft.println(volumesize);
      tft.print("Volume size (Kbytes): ");
      volumesize /= 1024;
      tft.println(volumesize);
      tft.print("Volume size (Mbytes): ");
      volumesize /= 1024;
      tft.println(volumesize);
    }
  }

  //Address bus always input  
  pinMode(AD0, INPUT);   
  pinMode(AD1, INPUT); 
  pinMode(AD2, INPUT); 
  pinMode(AD3, INPUT); 
  pinMode(AD4, INPUT); 
  pinMode(AD5, INPUT); 
  pinMode(AD6, INPUT); 
  pinMode(AD7, INPUT); 
  pinMode(AD8, INPUT); 
  pinMode(AD9, INPUT);
  pinMode(AD10, INPUT);
  pinMode(AD11, INPUT);
  pinMode(AD12, INPUT);
  pinMode(AD13, INPUT);
  
  pinMode(WR, INPUT); 
  pinMode(RD, INPUT);
  pinMode(MREQ, INPUT); 
  pinMode(RFSH, INPUT); 
  pinMode(IOREQ, INPUT); 
  pinMode(M1, INPUT);
    
  // Databus can be input or output, start as an output
  pinMode(D0, OUTPUT);   
  pinMode(D1, OUTPUT); 
  pinMode(D2, OUTPUT); 
  pinMode(D3, OUTPUT); 
  pinMode(D4, OUTPUT); 
  pinMode(D5, OUTPUT); 
  pinMode(D6, OUTPUT); 
  pinMode(D7, OUTPUT); 

  pinMode(RESET, OUTPUT);
  pinMode(Z_CLK, OUTPUT);   
  pinMode(INT, OUTPUT);
  
  digitalWrite(INT, HIGH);

  // initial values
  digitalWrite(RESET, LOW);  
  digitalWrite(Z_CLK, HIGH);  
  
  tft.setCursor (1, 1);
  tft.print("Z80 TEST");

  performZ80Reset();
  
  tft.fillRect (0, CONSOLE_START_Y-8, 240, 2, ILI9341_BLUE);
}

elapsedMillis timer_seconds;
byte timer_seconds_intvector = 2;

void loop() {
  cycle++;
  
  if (ioSerialInitialized && ioSerialIntVector)
  {
    
    if (!ioSerialINTDebounce && (Serial.available() > 0)) {
      digitalWrite(INT, LOW);
      
      currentInterruptVector = ioSerialIntVector;
      currentInterrupt = 1;
      ioSerialINTDebounce = 1;
    }
    else if (Serial.available() ==0)
    {
      ioSerialINTDebounce = 0;
    }
  }
  
  if ((timer_seconds > 1000) && !currentInterrupt)
  {
    timer_seconds = 0;
    digitalWrite(INT, LOW);
    currentInterruptVector = timer_seconds_intvector;
    currentInterrupt = 1;
  }
 
  digitalWrite(Z_CLK, HIGH);
  
  updateZ80Control();
  
  if (RFSH_val==1) {
    // skip cycles that are just refshing dram
    digitalWrite(Z_CLK, LOW);
    return;
  }
  
  //Use this for debugging if need be
  //delay(DELAY);
  
  if (currentInterrupt>0) {
    
    //digitalWrite(INT, HIGH);
    if (IOREQ_val && M1_val) // interrupt ack
    {
      digitalWrite(INT, HIGH);
      currentInterrupt = 0;
      dataBus = currentInterruptVector;
      writeDataBus();
     /* digitalWrite(Z_CLK, LOW);
      digitalWrite(Z_CLK, HIGH); // first wait state
      digitalWrite(Z_CLK, LOW);
      digitalWrite(Z_CLK, HIGH); // second wait state
      digitalWrite(Z_CLK, LOW);
      return;*/
    }
  }
  
  readAddressBus();
  
  if (MEMREQ_val) {
    if (RD_val) {
      if (addressBus < ROM_LENGTH) {
        dataBus = TEST_ROM[addressBus];
      } else {
        dataBus = 0x0;
      }
      writeDataBus();
    } else if (WR_val) {
      readDataBus();
      if (addressBus < ROM_LENGTH) {
        TEST_ROM[addressBus] = dataBus;
      }
    }
  }
  if (IOREQ_val && (ioDebounce == 0) && !M1_val)
  {
    ioDebounce = 1;
    unsigned short portAddress = addressBus & 0x00FF;
    if (RD_val) {
      if (portAddress == PORT_SERIAL_CMD)
      {
        dataBus = Serial.available();
      }
      else if (portAddress == PORT_SERIAL_DATA)
      {
        dataBus = Serial.read();
      } 
      else if (portAddress == PORT_DISP_SETROW)
      {
        dataBus = console_current_row;
      } 
      else if (portAddress == PORT_DISP_SETCOL)
      {
        dataBus = console_current_col;
      }
      writeDataBus();
      
    } else if (WR_val) {
      readDataBus();
      if (portAddress == PORT_VRAM_BUFFER_LOC)
      {
        ioVramBuffer = ((unsigned short)dataBus) << 8U;
      } 
      else if (portAddress == PORT_SERIAL_CMD)
      {
        if (ioSerialCurrentMode == SERIAL_CMD_READY)
        {
          //ready for commands, so put us in the right mode
          if (dataBus == SERIAL_CMD_INIT)
          {
            //Serial.begin(ioSerialRate);
            //while (!Serial);
            ioSerialInitialized = 1;
          } 
          else 
          {
            ioSerialCurrentMode = dataBus;
          }
        } else {
          //take data for the given mode
          if (ioSerialCurrentMode == SERIAL_CMD_SET_INTVECTOR)
          {
            ioSerialIntVector = dataBus;
            ioSerialCurrentMode = SERIAL_CMD_READY;
          } 
          else if (ioSerialCurrentMode == SERIAL_CMD_SET_RATE)
          {
            ioSerialCurrentMode = SERIAL_CMD_SET_RATE_2;
            ioSerialRate = dataBus;
          }
          else if (ioSerialCurrentMode == SERIAL_CMD_SET_RATE_2)
          {
            ioSerialCurrentMode = SERIAL_CMD_READY;
            ioSerialRate |= (((unsigned short)dataBus)<<8U);
          }
        }
      } 
      else if (portAddress == PORT_SERIAL_DATA)
      {
        Serial.write(dataBus);
      } 
      else if (portAddress == PORT_DISP_PUTCHAR)
      {
        char c = dataBus;
        //maybe add this to all debug routines and then only call it on a setcolour i/o
        tft.setTextColor( console_current_color, ILI9341_BLACK);
        tft.setCursor (CONSOLE_START_X + (console_current_col*CONSOLE_FONTX), CONSOLE_START_Y + (console_current_row*CONSOLE_FONTY));
        tft.print(fmtstring("%c", c));
        
        console_current_col++;
        if (console_current_col >= CONSOLE_COLUMNS) {
          console_current_col = 0;
          console_current_row++;
        }
        if (console_current_row >= CONSOLE_ROWS) {
          console_current_row = 0;
        }
      } 
      else if (portAddress == PORT_DISP_SETROW)
      {
        console_current_row = dataBus;
        if (console_current_row >= CONSOLE_ROWS) {
          console_current_row = 0;
        }
      } 
      else if (portAddress == PORT_DISP_SETCOL)
      {
        console_current_col = dataBus;
        
        if (console_current_col >= CONSOLE_COLUMNS) {
          console_current_col = 0;
        }
      }
      else if (portAddress == PORT_DISP_SETCOLOUR)
      {
        if ((console_current_color_state&0x1)==0x1) 
        {
          console_current_color |= dataBus<<8U;
        } 
        else
        {
          console_current_color = dataBus;
        }
        console_current_color_state++;
      }
    }
  } 
  else
  {
    // to deal with cycle timings figure 7, timing, z80 manual
    if (ioDebounce > 0)
    {
      ioDebounce++;
      if (ioDebounce >= 3)
      {
        ioDebounce = 0;
      }
    }
  }
  
  
#if DBG_DRAW_CYCLE
  debugDrawCycle();
#endif

#if DBG_DRAW_ADDR
  debugDrawAddressBus();
#endif

#if DBG_DRAW_DATA
  debugDrawDataBus();
#endif

#if MEMORY_DISPLAY
  memoryDisplayUpdate();
#endif
  
#if VRAM_DISPLAY
  vramDisplayUpdate();
#endif

  digitalWrite(Z_CLK, LOW); 

#if DBG_DRAW_LOG
  debugDrawLog();
#endif
}

void readAddressBus() {
  addressBus = 0;
  addressBus |= ((digitalRead(AD0)==HIGH)?1:0)<<0;
  addressBus |= ((digitalRead(AD1)==HIGH)?1:0)<<1;
  addressBus |= ((digitalRead(AD2)==HIGH)?1:0)<<2;
  addressBus |= ((digitalRead(AD3)==HIGH)?1:0)<<3;
  addressBus |= ((digitalRead(AD4)==HIGH)?1:0)<<4;
  addressBus |= ((digitalRead(AD5)==HIGH)?1:0)<<5;
  addressBus |= ((digitalRead(AD6)==HIGH)?1:0)<<6;
  addressBus |= ((digitalRead(AD7)==HIGH)?1:0)<<7;
  addressBus |= ((digitalRead(AD8)==HIGH)?1:0)<<8;
  addressBus |= ((digitalRead(AD9)==HIGH)?1:0)<<9;
  addressBus |= ((digitalRead(AD10)==HIGH)?1:0)<<10;
  addressBus |= ((analogRead(AD11)>512)?1:0)<<11;
  addressBus |= ((analogRead(AD11)>512)?1:0)<<12;
  addressBus |= ((analogRead(AD11)>512)?1:0)<<13;
}

void readDataBus() {
   pinMode(D0, INPUT); 
   pinMode(D1, INPUT); 
   pinMode(D2, INPUT); 
   pinMode(D3, INPUT); 
   pinMode(D4, INPUT); 
   pinMode(D5, INPUT); 
   pinMode(D6, INPUT); 
   pinMode(D7, INPUT); 
   
  dataBus = 0;
  dataBus |= ((digitalRead(D0)==HIGH)?1:0)<<0;
  dataBus |= ((digitalRead(D1)==HIGH)?1:0)<<1;
  dataBus |= ((digitalRead(D2)==HIGH)?1:0)<<2;
  dataBus |= ((digitalRead(D3)==HIGH)?1:0)<<3;
  dataBus |= ((digitalRead(D4)==HIGH)?1:0)<<4;
  dataBus |= ((digitalRead(D5)==HIGH)?1:0)<<5;
  dataBus |= ((digitalRead(D6)==HIGH)?1:0)<<6;
  dataBus |= ((digitalRead(D7)==HIGH)?1:0)<<7;
}

void writeDataBus() {
  pinMode(D0, OUTPUT); 
  pinMode(D1, OUTPUT); 
  pinMode(D2, OUTPUT); 
  pinMode(D3, OUTPUT); 
  pinMode(D4, OUTPUT); 
  pinMode(D5, OUTPUT); 
  pinMode(D6, OUTPUT); 
  pinMode(D7, OUTPUT);
     
  digitalWrite(D0, (dataBus&(1<<0))?HIGH:LOW);
  digitalWrite(D1, (dataBus&(1<<1))?HIGH:LOW);
  digitalWrite(D2, (dataBus&(1<<2))?HIGH:LOW);
  digitalWrite(D3, (dataBus&(1<<3))?HIGH:LOW);
  digitalWrite(D4, (dataBus&(1<<4))?HIGH:LOW);
  digitalWrite(D5, (dataBus&(1<<5))?HIGH:LOW);
  digitalWrite(D6, (dataBus&(1<<6))?HIGH:LOW);
  digitalWrite(D7, (dataBus&(1<<7))?HIGH:LOW);
}

void debugDrawCycle() {
  tft.setTextColor( ILI9341_WHITE, ILI9341_BLACK);
  tft.setCursor (76, 1);
  tft.print(cycle);
}

void debugDrawStatusLines() {
  tft.setTextColor( ILI9341_WHITE, ILI9341_BLACK);
  tft.setCursor (76, 160);
  tft.print(fmtstring("[%c %c %c %c] ", (M1_val?'F':' '), (RFSH_val?'H':' '), (MEMREQ_val?'M':' '), (IOREQ_val?'I':' ')));
}

void debugDrawLog() {

  if (GFX_LOG_LINES > 0)
  {
    
    tft.setTextColor( ILI9341_WHITE, ILI9341_BLACK);
    if (current_line >= GFX_LOG_LINES)
    {
      current_line = 0;
    }
    char* str = fmtstring("%02x: ? [%c %c %c %c] ", cycle, (M1_val?'F':' '), (RFSH_val?'H':' '), (MEMREQ_val?'M':' '), (IOREQ_val?'I':' '));
    
    int log_offset = GFX_LOG_START + (current_line*10);
    if (IOREQ_val && M1_val) {
      str = fmtstring("%02x: INT ACK DB:0x%02x", cycle, dataBus);
    } else if (M1_val && !MEMREQ_val && !RD_val) {
      str = fmtstring("%02x: I Fetch", cycle);
    } else if (M1_val && MEMREQ_val && RD_val) {
      str = fmtstring("%02x: I Read 0x%04x", cycle, addressBus);
    } else if (MEMREQ_val && RD_val) {
      str = fmtstring("%02x: Mem Rd 0x%04x=0x%02x", cycle, addressBus, dataBus);
    } else if (MEMREQ_val && WR_val) {
      str = fmtstring("%02x: Mem Wr 0x%04x=0x%02x", cycle, addressBus, dataBus);
    } else if (IOREQ_val && RD_val) {
      str = fmtstring("%02x: IO Rd 0x%04x=0x%02x", cycle, addressBus, dataBus);
    } else if (IOREQ_val && WR_val) {
      str = fmtstring("%02x: IO Wr 0x%04x=0x%02x", cycle, addressBus, dataBus);
    } else if (RFSH_val && RD_val) {
      str = fmtstring("%02x: RFSH 0x%04x", cycle, addressBus);
    } else if (RFSH_val && !M1_val) {
      str = fmtstring("%02x: RFSH 0x%04x", cycle, addressBus);
    } 
    tft.fillRect (0, log_offset, 196, 10, ILI9341_BLACK);
    tft.setCursor (2, log_offset );
    tft.print(str);
    
    tft.fillRect (0, GFX_LOG_START, 1, GFX_LOG_LINES*10, ILI9341_BLACK);
    
    tft.fillRect (0, log_offset, 1, 10, ILI9341_YELLOW);
    
    current_line++;
  }
skip:
  return;
}

void memoryDisplayUpdate() {
  tft.setCursor (0, GFX_CONTROL_START-8);
   
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  for (int x = 0; x < ROM_LENGTH; x++) 
  {
    if ((x%10)==0) 
    {
     tft.println(" ");
     tft.print(fmtstring("0x%04X: ", x)); 
    }
    
    byte data = TEST_ROM[x];
    tft.print(fmtstring("%02X ", data));
   
  }
}

void vramDisplayUpdate() {
  short vscr_addr = ioVramBuffer;
  
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  
  for (int y=0; y < CONSOLE_ROWS; y++)
  {
    for (int x = 0; x < CONSOLE_COLUMNS; x++)
    {
      short vscr_cur = (y*CONSOLE_COLUMNS) + x;
      unsigned char current = TEST_ROM[vscr_addr + vscr_cur];
      if (current != last_disp[vscr_cur])
      {
          last_disp[vscr_cur] = current;
          
          tft.setCursor (CONSOLE_START_X + (x*CONSOLE_FONTX), CONSOLE_START_Y + (y*CONSOLE_FONTY));
          tft.print(fmtstring("%c", current));
      }
    }
  } 
}

void debugDrawAddressBus() {
  
  tft.setTextColor( ILI9341_WHITE, ILI9341_BLACK);
  tft.setCursor (1, GFX_ADDR_START);
  tft.print("ADDR: 0x");
  tft.println(addressBus, HEX);
  tft.print("    B");
  tft.print('X');
  tft.print('X');
  tft.print((addressBus&(1<<13))?'1':'0');
  tft.print((addressBus&(1<<12))?'1':'0');
  tft.print((addressBus&(1<<11))?'1':'0');
  tft.print((addressBus&(1<<10))?'1':'0');
  tft.print((addressBus&(1<<9))?'1':'0');
  tft.print((addressBus&(1<<8))?'1':'0');
  tft.print((addressBus&(1<<7))?'1':'0');
  tft.print((addressBus&(1<<6))?'1':'0');
  tft.print((addressBus&(1<<5))?'1':'0');
  tft.print((addressBus&(1<<4))?'1':'0');
  tft.print((addressBus&(1<<3))?'1':'0');
  tft.print((addressBus&(1<<2))?'1':'0');
  tft.print((addressBus&(1<<1))?'1':'0');
  tft.print((addressBus&(1<<0))?'1':'0');
}

void debugDrawDataBus() {
  tft.setTextColor( ILI9341_WHITE, ILI9341_BLACK);
  tft.setCursor (1, GFX_DATA_START);
  tft.print("DATA: 0x");
  tft.println(dataBus, HEX);
  tft.print("    B");
  tft.print((dataBus&(1<<7))?'1':'0');
  tft.print((dataBus&(1<<6))?'1':'0');
  tft.print((dataBus&(1<<5))?'1':'0');
  tft.print((dataBus&(1<<4))?'1':'0');
  tft.print((dataBus&(1<<3))?'1':'0');
  tft.print((dataBus&(1<<2))?'1':'0');
  tft.print((dataBus&(1<<1))?'1':'0');
  tft.print((dataBus&(1<<0))?'1':'0');
}
