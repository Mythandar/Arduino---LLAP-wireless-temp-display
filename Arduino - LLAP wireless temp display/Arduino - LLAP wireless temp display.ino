/*
 Name:		Arduino___LLAP_wireless_temp_display.ino
 Created:	7/28/2018 9:22:21 PM
 Author:	James
*/

// This will catch the LLAP packets and display it

//#include <gfxfont.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>


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
char LLAPpacketID[3];
char LLAPpacketMessage[10];
char LLAPtempMessage[5];
char LLAPtempCelcius[6];
char LLAPbattVoltage[5];
char Message[9];
String batteryvoltage = "";
int batteryInfo = 0;

boolean FullPacket_Recieved = false;

void setup()
{
	Serial.begin(9600);
	pinMode(8, OUTPUT); // switch on the radio
	digitalWrite(8, HIGH);
	delay(300); // allow the radio to startup

	display.begin();
	display.setContrast(60);
	display.display(); // show splashscreen
	delay(200);
	display.clearDisplay(); // clears the screen and buffer
	display.display();

	//pinMode(10, INPUT_PULLUP);
	//delay(30);
	/* if (digitalRead(10) == LOW)
	{
		Serial.begin(9600);
		pinMode(8, OUTPUT); // switch on the radio
		digitalWrite(8, HIGH);
		delay(300); // allow the radio to startup
		Serial.write("+++\r\n");
		delay(30);
		Serial.write("ATBD 2580\r\n");
		delay(30);
		Serial.write("ATAC\r\n");
		Serial.println("Button pressed and set to 9600 for programming");
	}
	else if (digitalRead(10) == HIGH)
	{
		Serial.begin(115200);
		pinMode(8, OUTPUT); // switch on the radio
		digitalWrite(8, HIGH);
		delay(300); // allow the radio to startup
		Serial.write("+++\r\n");
		delay(30);
		Serial.write("ATBD 1C200\r\n");
		delay(30);
		Serial.write("ATAC\r\n");
	}
	
	pinMode(8, OUTPUT); // switch on the radio
	digitalWrite(8, HIGH);
	delay(300); // allow the radio to startup
	*/
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
		//Serial.println(LLAPmessage);
		//Serial.println(LLAPtempCelcius);

		// Break the LLAP into parts

		// Device ID
		for (int i = 1; i <= 2; i++)
		{
			LLAPpacketID[i - 1] = LLAPmessage[i];
			//LLAPpacketID[0] = LLAPmessage[1];
			//LLAPpacketID[1] = LLAPmessage[2]; this will do the same, but messier
		}
		
		// Message
		for (int i =1; i <=9; i++)
		{
			LLAPpacketMessage[i - 1] = LLAPmessage[i + 2];
			/*LLAPpacketMessage[0] = LLAPmessage[3];
			LLAPpacketMessage[1] = LLAPmessage[4];
			LLAPpacketMessage[2] = LLAPmessage[5];
			LLAPpacketMessage[3] = LLAPmessage[6];
			LLAPpacketMessage[4] = LLAPmessage[7];
			LLAPpacketMessage[5] = LLAPmessage[8];
			LLAPpacketMessage[6] = LLAPmessage[9];
			LLAPpacketMessage[7] = LLAPmessage[10];
			LLAPpacketMessage[8] = LLAPmessage[11];
			LLAPpacketMessage[9] = LLAPmessage[12];
			*/
		}
		
		//temp signal
		for (int i = 0; i <= 3; i++)
		{
			LLAPtempMessage[i] = LLAPmessage[i +3];
			/*LLAPtempMessage[0] = LLAPmessage[3];
			LLAPtempMessage[1] = LLAPmessage[4];
			LLAPtempMessage[2] = LLAPmessage[5];
			LLAPtempMessage[3] = LLAPmessage[6];
			*/
		}

		//Temp Celcius
		for (int i=0; i <=4; i++)
		{
			LLAPtempCelcius[i] = LLAPmessage[i + 7];
		}
		//also convert to farenheit

		float Celcius = atof(LLAPtempCelcius);
		float fahrenheit = (Celcius * 9.0) / 5.0 + 32;

		//Battery voltage
		for (int i = 0; i <= 3; i++)
		{
			LLAPbattVoltage[i] = LLAPmessage[i + 7];
		}
		

		
		String compare(LLAPtempMessage);
		if (compare == "BATT")
		{
			batteryInfo = 1;
			batteryvoltage = LLAPbattVoltage;
		}

		if (compare == "TMPA")
		{
			display.clearDisplay();
			display.setTextSize(1);
			display.setTextColor(BLACK);
			display.setCursor(0, 2);
			display.print("  ");
			display.print(LLAPtempCelcius);
			display.print(" C");
			display.setCursor(0, 15);
			display.print("  ");
			display.print(fahrenheit);
			display.print(" F");
			if (batteryInfo == 1)
			{
				display.setCursor(0, 30);
				display.print("BATT: ");
				display.print(batteryvoltage);
			}

		}
		
		//display.println(LLAPpacketMessage);
		//display.setCursor(0,20);
		//display.println(LLAPmessage);
		display.display();
		FullPacket_Recieved = false;
	}
}


void loop()
{
	MonitorSerialForLLAP();
	SendLLAPpacket();
}
