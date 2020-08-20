//=============================================================================
// "Arduino" example program for Crystalfontz ePaper. 
//
// This project is for the CFAP400300C0-0420 :
//
//   https://www.crystalfontz.com/product/cfap400300c00420
//
// It was written against a Seeduino v4.2 @3.3v. An Arduino UNO modified to
// operate at 3.3v should also work.
//-----------------------------------------------------------------------------
// This is free and unencumbered software released into the public domain.
// 
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
// 
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
// 
// For more information, please refer to <http://unlicense.org/>
//=============================================================================
// Connecting the Arduino to the display
//
// ARDUINO |Adapter |Wire Color |Function
// --------+--------+-----------+--------------------
// D2      |19      |Yellow     |BS1 Not Used
// D3      |17      |Green      |Busy Line
// D4      |18      |Brown      |Reset Line
// D5      |15      |Purple     |Data/Command Line
// D10     |16      |Blue       |Chip Select Line
// D11     |14      |White      |MOSI
// D13     |13      |Orange     |Clock
// 3.3V    |5       |Red        |Power
// GND     |3       |Black      |Ground
//
// Short the following pins on the adapter board:
// GND  -> BS2
// RESE -> 3ohms
//=============================================================================
//Connecting the Arduino to the SD card
//
// ARDUINO  |Wire Color |Function
// ---------+-----------+--------------------
// D2       |Blue       |CS
// D3       |Green      |MOSI
// D4       |Brown      |CLK
// D5       |Purple     |MISO
//
//=============================================================================
// Creating image data arrays
//
// Bmp_to_epaper is code that will aid in creating bitmaps necessary from .bmp files.
// The code can be downloaded from the Crystalfontz website: https://www.Crystalfontz.com
// or it can be downloaded from github: https://github.com/crystalfontz/bmp_to_epaper
//=============================================================================

// The display is SPI, include the library header.
#include <SPI.h>
#include <SD.h>
#include <avr/io.h>

// Include LUTs
#include "LUTs_for_CFAP400300C00420.h"
#include "Images_for_CFAP400300C00420.h"

#define ePaper_RST_0  (digitalWrite(EPD_RESET, LOW))
#define ePaper_RST_1  (digitalWrite(EPD_RESET, HIGH))
#define ePaper_CS_0   (digitalWrite(EPD_CS, LOW))
#define ePaper_CS_1   (digitalWrite(EPD_CS, HIGH))
#define ePaper_DC_0   (digitalWrite(EPD_DC, LOW))
#define ePaper_DC_1   (digitalWrite(EPD_DC, HIGH))

#define EPD_READY   3
#define EPD_RESET   4
#define EPD_DC      5
#define EPD_CS      10
#define SD_CS       8

#define HRES  300
#define VRES  400

//=============================================================================
//this function will take in a byte and send it to the display with the 
//command bit low for command transmission
void writeCMD(uint8_t command)
{
	ePaper_DC_0;
	ePaper_CS_0;
	SPI.transfer(command);
	ePaper_CS_1;
}

//this function will take in a byte and send it to the display with the 
//command bit high for data transmission
void writeData(uint8_t data)
{
	ePaper_DC_1;
	ePaper_CS_0;
	SPI.transfer(data);
	ePaper_CS_1;
}

//===========================================================================
void setup(void)
{
	//delay(50);
	//Debug port / Arduino Serial Monitor (optional)
	Serial.begin(9600);
  Serial.println("setup started");
	// Configure the pin directions   
	pinMode(EPD_CS, OUTPUT);
	pinMode(EPD_RESET, OUTPUT);
	pinMode(EPD_DC, OUTPUT);
	pinMode(EPD_READY, INPUT);
  pinMode(SD_CS, OUTPUT);

  digitalWrite(SD_CS, LOW);

  if (!SD.begin(SD_CS))
  {
    Serial.println("SD could not initialize");
  }
	//Set up SPI interface
	SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
	SPI.begin();


	//reset driver
	ePaper_RST_0;
	delay(200);
	ePaper_RST_1;
	delay(200);

  initEPD();
	Serial.println("setup complete");
}

void initEPD()
{

  //-----------------------------------------------------------------------------
  //more detail on the following commands and additional commands not used here	
  //can be found on the CFAP400300C0-0420 datasheet on the Crystalfontz website	
  //-----------------------------------------------------------------------------

  //Power Setting
  writeCMD(0x01);
  writeData(0x03);
  writeData(0x00);
  writeData(0x2B);
  writeData(0x2B);
  writeData(0x09);

  //Booster Soft Start
  writeCMD(0x06);
  writeData(0x17);
  writeData(0x17);
  writeData(0x17);

  //Power On
  writeCMD(0x04);
  Serial.println("before wait");
  //wait until powered on
  while (0 == digitalRead(EPD_READY));
  Serial.println("after wait");

  //Panel Setting 
  writeCMD(0x00);
  writeData(0x83);

  //PLL Control
  writeCMD(0x30);
  writeData(0x3a);

  //Resolution
  writeCMD(0x61);
  writeData(0x01);	//first half of 2 bytes:  256		
  writeData(0x90);	//second half of 2 bytes: 144 -> first half plus second half = 400
  writeData(0x01);	//first half of 2 bytes:  256
  writeData(0x2c);	//second half of 2 bytes: 44  -> first half plus second half = 300


  //Vcom and data interval setting
  writeCMD(0x50);
  writeData(0x07);

}

void setRegisterLUT()
{
	//set LUTs
	//The following block allows the LUTs to be changed.
	//In order for these LUTs to take effect, command 0x00 must have bit 5 set to "1"
	//set panel setting to call LUTs from the register
	writeCMD(0x00);
	writeData(0x2b); //11101111

	//VCOM_LUT_LUTC
	writeCMD(0x20);
	for (int i = 0; i < 44; i++)
	{
		writeData(pgm_read_byte(&VCOM_LUT_LUTC[i]));
	}
	//W2W_LUT_LUTWW
	writeCMD(0x21);
	for (int i = 0; i < 42; i++)
	{
		writeData(pgm_read_byte(&W2W_LUT_LUTWW[i]));
	}
	//B2W_LUT_LUTBW_LUTR
	writeCMD(0x22);
	for (int i = 0; i < 42; i++)
	{
		writeData(pgm_read_byte(&B2W_LUT_LUTBW_LUTR[i]));
	}
	//W2B_LUT_LUTWB_LUTW
	writeCMD(0x23);
	for (int i = 0; i < 42; i++)
	{
		writeData(pgm_read_byte(&W2B_LUT_LUTWB_LUTW[i]));
	}
	//B2B_LUT_LUTBB_LUTB
	writeCMD(0x24);
	for (int i = 0; i < 42; i++)
	{
		writeData(pgm_read_byte(&B2B_LUT_LUTBB_LUTB[i]));
	}
}

void setOTPLUT()
{
	//set panel setting to call LUTs from OTP
	writeCMD(0x00);
	writeData(0x0b); //00001011
}

void partialUpdateSolid(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color1, uint8_t color2)
{
  //Please note, this is demonstration code on how to use the Partial Updating on the controller but this display itself
  //  does not support partial updating. Using this code may cause ghosting issues over time.


  //turn on partial update mode
  writeCMD(0x91);

  writeCMD(0x90);
  writeData((x1 / 8) << 1);	  //1st half x1
  writeData(((x1 / 8) << 1) & 0xf8);	//2nd half x1
  writeData((x2 / 8) << 1);	  //1st half x2
  writeData(((x2 / 8) << 1) & 0xf8 | 0x07);	//2nd half x2
  writeData(y1 >> 8);	  //1st half y1
  writeData(y1 & 0xff);	//2nd half y1
  writeData(y2 >> 8);	  //1st half y2
  writeData(y2 & 0xff);	//2nd half y2
  writeData(0x01);

  writeCMD(0x10);
  int i;
  int h;
  for (h = 0; h < y2 - y1; h++)
  {
    for (i = 0; i <  (x2 - x1) / 8; i++)
    {
      writeData(color1);
    }
  }
  writeCMD(0x13);
  for (h = 0; h < y2 - y1; h++)
  {
    for (i = 0; i < (x2 - x1) / 8; i++)
    {
      writeData(color2);
    }
  }

  //partial refresh of the same area as the partial update
  writeCMD(0x12);
  while (0 == digitalRead(EPD_READY));

  //turn off partial update mode
  writeCMD(0x92);
  //initEPD();
}


//================================================================================
void show_BMPs_in_root(void)
{
  File
    root_dir;
  root_dir = SD.open("/");
  if (0 == root_dir)
  {
    Serial.println("show_BMPs_in_root: Can't open \"root\"");
    return;
  }


  File
    bmp_file;

  while (1)
  {
    bmp_file = root_dir.openNextFile();
    if (0 == bmp_file)
    {
      // no more files, break out of while()
      // root_dir will be closed below.
      break;
    }
    //Skip directories (what about volume name?)
    if (0 == bmp_file.isDirectory())
    {
      //The file name must include ".BMP"
      if (0 != strstr(bmp_file.name(), ".BMP"))
      {
        //The BMP must be exactly 36918 long
        //(this is correct for182x96, 24-bit)
        uint32_t size = bmp_file.size();

        Serial.println(size);
        if ( 360054 <= size)
        {
          Serial.println("made it");


          writeCMD(0x10);
          //Jump over BMP header
          bmp_file.seek(54);
          //grab one row of pixels from the SD card at a time
          static uint8_t half_line[200 * 3];
          for (int line = 0; line < 300*2; line++)
          {

            //Set the LCD to the left of this line. BMPs store data
            //to have the image drawn from the other end, uncomment the line below

            //read a line from the SD card
            bmp_file.read(half_line, 200 * 3);

            //send the line to the display
            send_pixels_BW(200 * 3, half_line);

          }
          Serial.println("halfway");


          writeCMD(0x13);
          //Jump over BMP header
          bmp_file.seek(54);
          //grab one row of pixels from the SD card at a time
          for (int line = 0; line < 300 * 2; line++)
          {
            //Set the LCD to the left of this line. BMPs store data
            //to have the image drawn from the other end, uncomment the line below

            //read a line from the SD card
            bmp_file.read(half_line, 200 * 3);

            //send the line to the display
            send_pixels_Y(200 * 3, half_line);
          }
          Serial.println("done");
          writeCMD(0x12);
          while (0 == digitalRead(EPD_READY));
        }
      }
    }
    //Release the BMP file handle
    bmp_file.close();
    //Give a bit to let them see it
    delay(3000);
  }
  //Release the root directory file handle
  root_dir.close();
  //SPI.beginTransaction(SPISettings(12000000, MSBFIRST, SPI_MODE0));
}

//================================================================================
void Load_Flash_Image_To_Display_RAM_RLE(uint16_t width_pixels,
  uint16_t height_pixels,
  const uint8_t *BW_image,
  const uint8_t *R_image)
{
  //Index into *image, that works with pgm_read_byte()
  uint8_t count = 0;

  //Get width_bytes from width_pixel, rounding up
  uint8_t
    width_bytes;
  width_bytes = (width_pixels + 7) >> 3;

  //Make sure the display is not busy before starting a new command.
  while (0 == digitalRead(EPD_READY));
  //Select the controller   
  ePaper_CS_0;

  //Aim at the command register
  ePaper_DC_0;
  //Write the command: DATA START TRANSMISSION 1 (DTM2) (R13H)
  //  Display Start transmission 1
  //  (DTM1, BW Data)
  //
  // This command starts transmitting data and write them into SRAM. To complete
  // data transmission, command DSP (Data transmission Stop) must be issued. Then
  // the chip will start to send data/VCOM for panel.
  //  * In B/W mode, this command writes �OLD� data to SRAM.
  //  * In B/W/Red mode, this command writes �BW� data to SRAM.
  SPI.transfer(0x10);
  //Pump out the BW data.
  ePaper_DC_1;
  count = 0;
  for (int i = 0; i < MONO_ARRAY_SIZE; i = i + 2)
  {
    count = pgm_read_byte(&BW_image[i]);
    for (uint8_t j = 0; j < count; j++)
    {
      SPI.transfer(pgm_read_byte(&BW_image[i + 1]));
    }
  }



  //Aim at the command register
  ePaper_DC_0;
  //Write the command: DATA START TRANSMISSION 2 (DTM2) (R13H)
  //  Display Start transmission 2
  //  (DTM2, Red Data)
  //
  // This command starts transmitting data and write them into SRAM. To complete
  // data transmission, command DSP (Data transmission Stop) must be issued. Then
  // the chip will start to send data/VCOM for panel.
  //  * In B/W mode, this command writes �NEW� data to SRAM.
  //  * In B/W/Red mode, this command writes �Red� data to SRAM.
  SPI.transfer(0x13);
  //Pump out the Red data.
  ePaper_DC_1;
  count = 0;
  for (int i = 0; i < YELLOW_ARRAY_SIZE; i = i + 2)
  {
    count = pgm_read_byte(&R_image[i]);
    for (uint8_t j = 0; j < count; j++)
      SPI.transfer(pgm_read_byte(&R_image[i + 1]));
  }

  //Aim back at the command register
  ePaper_DC_0;
  //Write the command: DATA STOP (DSP) (R11H)
  SPI.transfer(0x11);
  //Write the command: Display Refresh (DRF)   
  SPI.transfer(0x12);
  //Deslect the controller   
  ePaper_CS_1;
}


//================================================================================
void send_pixels_BW(uint16_t byteCount, uint8_t *dataPtr)
{
  uint8_t data;
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  while (byteCount != 0)
  {
    uint8_t data = 0;
    red = *dataPtr;
    dataPtr++;
    byteCount--;
    green = *dataPtr;
    dataPtr++;
    byteCount--;
    blue = *dataPtr;
    dataPtr++;
    byteCount--;

    //check to see if pixel should be set as black or white
    if (127 > ((red*.21) + (green*.72) + (blue*.07)))
    {
      data = data | 0x01;
    }

    for (uint8_t i = 0; i < 7; i++)
    {
      red = *dataPtr;
      dataPtr++;
      byteCount--;
      green = *dataPtr;
      dataPtr++;
      byteCount--;
      blue = *dataPtr;
      dataPtr++;
      byteCount--;
      data = data << 1;

      //check to see if pixel should be set as black or white
      if (127 > ((red*.21) + (green*.72) + (blue*.07)))
      {
        data = data | 0x01;
      }
      else
      {
        data = data & 0xFE;
      }
    }
      writeData(data);
  }
}

//================================================================================
void send_pixels_Y(uint8_t byteCount, uint8_t *dataPtr)
{
  uint8_t data;
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  while (byteCount != 0)
  {
    uint8_t data = 0;
    blue = *dataPtr;
    dataPtr++;
    byteCount--;
    green = *dataPtr;
    dataPtr++;
    byteCount--;
    red = *dataPtr;
    dataPtr++;
    byteCount--;

    //check to see if pixel should be set as yellow
    if ((228 < red) && ( 208 < green) && (blue < 250))
    {
      data |= 0x01;
    }

    for (uint8_t i = 0; i < 7; i++)
    {
      blue = *dataPtr;
      dataPtr++;
      byteCount--;
      green = *dataPtr;
      dataPtr++;
      byteCount--;
      red = *dataPtr;
      dataPtr++;
      byteCount--;
      data = data << 1;

      //check to see if pixel should be set as yellow
      if ((228 < red) && (208 < green) && (blue < 250))
      {
        data |= 0x01;
      }
      else
      {
        data &= 0xFE;
      }
    }
      writeData(data);
  }
}


void powerON()
{
  writeCMD(0x04);
}

void powerOff()
{
  writeCMD(0x02);
  writeCMD(0x03);
  writeData(0x00);
}

//=============================================================================
#define SHUTDOWN_BETWEEN_UPDATES (0)
#define waittime          18000
#define splashscreenRLE   1
#define white             0
#define black             0
#define yellow            1
#define checkerboard      1
#define partialUpdate     0
#define showBMPs          0
void loop()
{
  Serial.println("top of loop");


#if splashscreenRLE
  //on the Seeeduino, there is not enough flash memory to store this data 
  //but if another uP with more flash is used, this function can be utilized
  //power on the display
  powerON();
  //load an image to the display
  Load_Flash_Image_To_Display_RAM_RLE(HRES, VRES, Mono_1BPP, Yellow_1BPP);


  Serial.print("refreshing . . . ");
  while (0 == digitalRead(EPD_READY));
  Serial.println("refresh complete");
  //for maximum power conservation, power off the EPD
  powerOff();
  delay(waittime);
#endif


#if white
  powerON();
	//Display the splash screen
	writeCMD(0x10);
	for (int i = 0; i < 15000; i++)
	{
		writeData(0x00);
	}

	//start data transmission 2: red data
	writeCMD(0x13);
	for (int i = 0; i < 15000; i++)
	{
		writeData(0x00);
	}
  Serial.println("before refresh wait");
	//refresh the display
	writeCMD(0x12);
	while (0 == digitalRead(EPD_READY));
  Serial.println("after refresh wait");
  powerOff();
	delay(waittime);
#endif


#if black
  powerON();
  //Display the splash screen
  writeCMD(0x10);
  for (int i = 0; i < 15000; i++)
  {
    writeData(0xff);
  }

  //start data transmission 2: red data
  writeCMD(0x13);
  for (int i = 0; i < 15000; i++)
  {
    writeData(0x00);
  }
  Serial.println("before refresh wait");
  //refresh the display
  writeCMD(0x12);
  while (0 == digitalRead(EPD_READY));
  Serial.println("after refresh wait");
  powerOff();
  delay(waittime);
#endif


#if yellow
  powerON();
  //Display the splash screen
  writeCMD(0x10);
  for (int i = 0; i < 15000; i++)
  {
    writeData(0x00);
  }

  //start data transmission 2: red data
  writeCMD(0x13);
  for (int i = 0; i < 15000; i++)
  {
    writeData(0xff);
  }
  Serial.println("before refresh wait");
  //refresh the display
  writeCMD(0x12);
  while (0 == digitalRead(EPD_READY));
  Serial.println("after refresh wait");
  powerOff();
  delay(waittime);
#endif

#if checkerboard
  uint8_t color1 = 0x00;
  uint8_t color2 = 0xff;

  powerON();

  writeCMD(0x10);
  for (int i = 0; i < 300; i++)
  {
   // color1 = ~color1;
    if (i % 6 == 0)
    {
      color1 = ~color1;
    }
    for (uint8_t j = 0; j < 50; j++)
    {
      writeData(color1);
      color1 = ~color1;
    }
  }

  writeCMD(0x13);
  for (int i = 0; i < 300; i++)
  {
    //color2 = ~color2;
    if (i % 6 == 0)
    {
      color2 = ~color2;
    }
    for (uint8_t j = 0; j < 50; j++)
    {
      writeData(color2);
      color2 = ~color2;
    }
  }

  Serial.println("before refresh wait");
  //refresh the display
  writeCMD(0x12);
  while (0 == digitalRead(EPD_READY));
  Serial.println("after refresh wait");
  powerOff();
  delay(waittime);
#endif

#if partialUpdate
  //Please note, this is demonstration code on how to use the Partial Updating on the controller but this display itself
  //  does not support partial updating. Using this code may cause ghosting issues over time.


  powerON();
  partialUpdateSolid(100, 100, 200, 200, 0xff, 0x00);
  while (0 == digitalRead(EPD_READY));
  delay(1000);

  partialUpdateSolid(100, 100, 200, 200, 0x00, 0xff);
  while (0 == digitalRead(EPD_READY));
  delay(1000);

  partialUpdateSolid(100, 100, 200, 200, 0x00, 0x00);
  while (0 == digitalRead(EPD_READY));
  powerOff();
  delay(waittime);
#endif

#if showBMPs
  powerON();
  show_BMPs_in_root();
  powerOff();
#endif

}
//=============================================================================
