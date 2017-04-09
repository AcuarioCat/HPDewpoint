/*
 Name:		HPDewpoint.ino
 Created:	1/21/2017 10:40:26 AM
 Author:	Nigel
*/
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <DHT_U.h>
#include <Adafruit_Sensor.h>
#include <math.h>
#include "ESPAsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include "DHT.h"
#include "debugnm.h"

#define version "1.0"
#define WIFIPORT 8081
AsyncWebServer server(WIFIPORT);
//AsyncWebServer server = AsyncWebServer(0);
//AsyncWebSocket ws("/ws"); // access at ws://[esp ip]/ws
//AsyncEventSource events("/updates"); // event source (Server-Sent events)
AsyncEventSource debugEvent("/debugMsg"); // event source (Server-Sent events)

const char* ssid = "Terrapico";
const char* password = "5432167890";
//const char* hostName = "DewPoint";
const char* hostName = "HPController";
bool shouldReboot = false;

#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
// DHT Sensor
const int DHTPin = 2;			//GPIO2
// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);

// Temporary variables
static char celsiusTemp[7];
static char dewPointTemp[7];
static char humidityTemp[7];
float dewPoint = 0;
float humidity = 0;
float temperature = 0;
bool freshData;				//Flag if data is fresh or old (old if dht read failure)
int failCount = 0;

Ticker tmrGetSensor, tmrLED;
bool bReadSensor = true;
void doGetSensor() { bReadSensor = true; }

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(115200);
	delay(1000);
	DEBUG_PRINTLN("Startup wifi");
	WiFi.hostname(hostName);
	setupWifi();
	
	//WiFi.mode(WIFI_STA);					//WIFI_STA
	//WiFi.begin(ssid, password);
	//if (WiFi.waitForConnectResult() != WL_CONNECTED) {
	//	DEBUG_PRINTLN("WiFi Failed! Rebooting...");
	//	delay(100);
	//	ESP.restart();
	//	return;
	//}
	DEBUG_PRINTLN("WiFi connected!");
	DEBUG_PRINTST("%d.%d.%d.%d:%d %s\n", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3], WIFIPORT, version);
	sprintf(celsiusTemp, "0");
	sprintf(dewPointTemp, "0");
	sprintf(humidityTemp, "0");
	
	SPIFFS.begin();
	DEBUG_PRINTLN("SPIFFS started");

	setupWebServer();
	setupUDP();
	dht.begin();
	tmrGetSensor.attach(20, doGetSensor);

}

// the loop function runs over and over again until power down or reset
void loop() {
	if (bReadSensor == true)
	{
		readDHT();
		bReadSensor = false;
	}
	delay(100);
	checkUDP();
	if (shouldReboot) {
		DEBUG_PRINTLN("Rebooting...");
		delay(100);
		ESP.restart();
	}
}

void readDHT() {
	DEBUG_PRINTLN("Reading DHT");
	// Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
	humidity = dht.readHumidity();
	// Read temperature as Celsius (the default)
	temperature = dht.readTemperature();
	// Check if any reads failed and exit early (to try again).
	freshData = true;
	if (isnan(humidity) || isnan(temperature)) {
		Serial.printf("Failed to read from DHT sensor! (%d)\n", failCount);
		freshData = false;
		failCount++;
		if(failCount>10)
			tmrLED.attach(1, flashLED);
	}
	else {
		// Computes temperature values in Celsius and Humidity
		float hic = dht.computeHeatIndex(temperature, humidity, false);
		dewPoint = calcDewPoint(temperature, humidity);
		dtostrf(hic, 6, 2, celsiusTemp);
		dtostrf(humidity, 6, 2, humidityTemp);
		dtostrf(dewPoint, 6, 2, dewPointTemp);
		DEBUG_PRINTST("Temp=%s, H=%s, Dew=%s\n", celsiusTemp, humidityTemp, dewPointTemp);
		failCount = 0;
	}
}

// dewPoint function NOAA
// reference (1) : http://wahiduddin.net/calc/density_algorithms.htm
// reference (2) : http://www.colorado.edu/geography/weather_station/Geog_site/about.htm
//
double calcDewPoint(double celsius, double humidity)
{
	// (1) Saturation Vapor Pressure = ESGG(T)
	double RATIO = 373.15 / (273.15 + celsius);
	double RHS = -7.90298 * (RATIO - 1);
	RHS += 5.02808 * log10(RATIO);
	RHS += -1.3816e-7 * (pow(10, (11.344 * (1 - 1 / RATIO))) - 1);
	RHS += 8.1328e-3 * (pow(10, (-3.49149 * (RATIO - 1))) - 1);
	RHS += log10(1013.246);

	// factor -3 is to adjust units - Vapor Pressure SVP * humidity
	double VP = pow(10, RHS - 3) * humidity;

	// (2) DEWPOINT = F(Vapor Pressure)
	double T = log(VP / 0.61078);   // temp var
	return (241.88 * T) / (17.558 - T);
}

