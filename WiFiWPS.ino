// program WiFiTelnetToSerialWPS.ino, Arduino 1.6.12
/*
WiFiTelnetToSerial - Example Transparent UART to Telnet Server for esp8266
*/

/* 2016-10-17 Rudolf Reuter, push button switch for WPS function added.
*  http://www.wi-fi.org/knowledge-center/faq/how-does-wi-fi-protected-setup-work
*
*  Once the WiFi credentials are WPS fetched, they are stored,
*  for following use (firmware function).
*
*  Works on ESP-01 module - Arduino Tools:
*      Board: "Generic ESP8266 Module"
*      Flash Mode: "DIO"
*      Flash Fequency: "40 MHz"
*      CPU Frequency: "80 MHz"
*      Flash Size: "512K (64K SPIFFS)"
*      Debug port: "Disabled"
*      Reset Method: "ck"
*      Upload Speed: "115200"
*
*  Connection of ESP8266 GPIO0 and GPIO2 for WPS button and LED indicator:
*         Vcc  Vcc       Vcc = 3.3 V
*          |    |         |
*         4K7  4K7       1k0  pull up resistors (3K3 to 10K Ohm)
*          |    |   +-|<|-+   LED red
*          |    |   |
* GPIO0  - +--------+---o |
*               |         | -> | push button switch for WPS function
* GPIO2  -------+-------o |
*
* from http://www.forward.com.au/pfod/ESP8266/GPIOpins/index.html
*
* /!\ There is a '''5 minute''' timeout in the ESP8266 '''!WiFi stack'''.
* That means, if you have after the first WLAN connection a data traffic pause
* longer than '''5 minutes''', you to have to resync the connection
* by sending some dummy characters.
*/

#include <ESP8266WiFi.h>

bool startWPSPBC() {
	// from https://gist.github.com/copa2/fcc718c6549721c210d614a325271389
	DEBUG_PRINTLN("WPS config start");
	bool wpsSuccess = WiFi.beginWPSConfig();
	if (wpsSuccess) {
		// Well this means not always success :-/ in case of a timeout we have an empty ssid
		String newSSID = WiFi.SSID();
		if (newSSID.length() > 0) {
			// WPSConfig has already connected in STA mode successfully to the new station. 
			DEBUG_PRINTST("WPS finished. Connected successfull to SSID '%s'\n", newSSID.c_str());
		}
		else {
			wpsSuccess = false;
		}
	}
	return wpsSuccess;
}

void setupWifi() {
	wl_status_t status;
	//Serial.begin(115200);
	DEBUG_PRINTLN("\n WPS with push button on GPIO2 input, LOW = active");
	DEBUG_PRINTST("\nTry connecting to WiFi with SSID '%s'\n", WiFi.SSID().c_str());
	DEBUG_PRINTST("\nPassword '%s'\n", WiFi.psk().c_str());
	pinMode(0, OUTPUT);         // Use GPIO0
	digitalWrite(0, LOW);       // for hardware safe operation.

	WiFi.mode(WIFI_STA);

	for (int retry = 0; retry < 5; retry++)
	{
		WiFi.begin(WiFi.SSID().c_str(), WiFi.psk().c_str()); // reading data from EPROM, 

		while (WiFi.status() == WL_DISCONNECTED) {          // last saved credentials
			delay(500);
			DEBUG_PRINT(".");
		}

		status = WiFi.status();
		if ((status == WL_NO_SSID_AVAIL)|| (status == WL_CONNECTED))
			break;
	}

	if (status == WL_CONNECTED) {
		DEBUG_PRINTST("\nConnected successful to SSID '%s'\n", WiFi.SSID().c_str());
		digitalWrite(0, HIGH);
	}
	else {
		// WPS button I/O setup
		pinMode(2, INPUT_PULLUP);  // Push Button for GPIO2 active LOW
		DEBUG_PRINTST("\nCould not connect to WiFi. state='%d'\n", status);
		DEBUG_PRINTLN("Please press WPS button on your router, until mode is indicated.");
		DEBUG_PRINTLN("next press the ESP module WPS button, router WPS timeout = 2 minutes");

		tmrLED.attach_ms(250, flashLED);
		while (digitalRead(2) == HIGH)  // wait for WPS Button active
			yield(); // do nothing, allow background work (WiFi) in while loops
		tmrLED.detach();
		tmrLED.attach_ms(500, flashLED);
		DEBUG_PRINTLN("WPS button pressed");

		if (!startWPSPBC()) {
			DEBUG_PRINTLN("Failed to connect with WPS :-(");
		}
		else {
			WiFi.begin(WiFi.SSID().c_str(), WiFi.psk().c_str()); // reading data from EPROM, 
			while (WiFi.status() == WL_DISCONNECTED) {          // last saved credentials
				delay(500);
				DEBUG_PRINT("."); // show wait for connect to AP
			}
			//pinMode(0, INPUT);    // GPIO0, LED OFF, show WPS & connect OK
			tmrLED.detach();
			digitalWrite(0, HIGH);
		}
	}
}

bool ledState;
void flashLED() {
	digitalWrite(0, ledState);
	ledState = !ledState;
}