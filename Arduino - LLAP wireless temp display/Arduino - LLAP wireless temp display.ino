/*
 Name:		Arduino___LLAP_wireless_temp_display.ino
 Created:	7/28/2018 9:22:21 PM
 Author:	James
*/

// This will catch the LLAP packets and display it

#include <SPI.h>
#include <gfxfont.h>
#include <Adafruit_SPITFT_Macros.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <LLAPSerial.h>

// Software SPI (slower updates, more flexible pin options):
// pin 7 - Serial clock out (SCLK)
// pin 6 - Serial data out (DIN)
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
//Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);

// Hardware SPI (faster, but must use certain hardware pins):
// SCK is LCD serial clock (SCLK) - this is pin 13 on Arduino Uno
// MOSI is LCD DIN - this is pin 11 on an Arduino Uno
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(5, 4, 3);
// Note with hardware SPI MISO and SS pins aren't used but will still be read
// and written to during SPI transfer.  Be careful sharing these pins!

const byte numChars = 13; //12 bytes for LLAP message and 1 more byte for the termination of the string
char LLAPmessage[numChars];
boolean FullPacket_Recieved = false;

void setup()
{
	Serial.begin(9600);
	pinMode(8, OUTPUT); // switch on the radio
	digitalWrite(8, HIGH);
	delay(1000); // allow the radio to startup
	
	display.begin();
	display.setContrast(60);
	

	Serial.println("<Arduino is ready>");
}

void MonitorSerialForLLAP()
{
	static boolean recvInProgress = false;
	static byte ndx = 0;
	char startMarker = 'a'; //Marks the beginning of an LLAP packet
	char rc;

	while (Serial.available() > 0 && FullPacket_Recieved == false)
	{
		rc = Serial.read();

		if (recvInProgress == true)
		{
			if (ndx <= 11)
			{
				LLAPmessage[ndx] = rc;
				ndx++;
				if (ndx == 12)
				{
					recvInProgress = false;
					LLAPmessage[ndx] = '\0';
					ndx = 0;
					FullPacket_Recieved = true;
				}
			}
			else
			{
				LLAPmessage[ndx] = '\0'; // terminate the string
				recvInProgress = false;
				ndx = 0;
				FullPacket_Recieved = true;
			}
		}

		else if (rc == startMarker)
		{
			recvInProgress = true;
			LLAPmessage[ndx] = rc;
			ndx++;
		}
	}
}

void SendLLAPpacket()
{
	if (FullPacket_Recieved == true)
	{
		//Serial.print("This just in ... ");
		Serial.println(LLAPmessage);

		FullPacket_Recieved = false;
	}
}

void loop()
{
	MonitorSerialForLLAP();
	SendLLAPpacket();
}
