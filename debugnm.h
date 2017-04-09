#pragma once
#include <Arduino.h>
//#define DEBUG
#define WEB
//#define TELNET

#ifdef DEBUG
#define DEBUG_PRINT(x)  Serial.print (x)
#ifdef ESP8266
#define DEBUG_PRINTST(x,...)  Serial.printf (x,__VA_ARGS__)
#else
#define DEBUG_PRINTST(x,...)  SerialPrint2 (x,__VA_ARGS__)
#endif
#define DEBUG_PRINTLN(x)  Serial.println (x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTST(x,...)
#define DEBUG_PRINTLN(x)
#endif

#ifdef WEB
extern AsyncEventSource debugEvent;
#define DEBUG_PRINT(x)  sendWeb (x)
#define DEBUG_PRINTST(x,...)  sendWebF (x,__VA_ARGS__)
#define DEBUG_PRINTST(x,...)  {char buf[128]; sprintf(buf, x,__VA_ARGS__); sendWeb(buf);}
#define DEBUG_PRINTLN(x)  sendWebln (x)

#ifdef ESP8266
#define __builtin_va_start
#define __builtin_va_end
#endif

void sendWeb(const char * data)
{
	Serial.print(data);
	debugEvent.send(data, "debug");
}
void sendWebln(const char * data)
{
	Serial.println(data);
	debugEvent.send(data, "debug");
}
#endif

#ifdef TELNET
#define MAX_SRV_CLIENTS 1
#define DEBUG_PRINT(x)  sendTelnet (x)
#define DEBUG_PRINTST(x,...)  sendTelnetF (x,__VA_ARGS__)
#define DEBUG_PRINTLN(x)  sendTelnetln (x)

// start telnet server (do NOT put in setup())
const uint16_t aport = 23; // standard port
WiFiServer TelnetServer(aport);
WiFiClient TelnetClient[MAX_SRV_CLIENTS];
void setupTelnet()
{
	TelnetServer.begin();
	TelnetServer.setNoDelay(true);
}

void sendTelnet(const char * data)
{
	Serial.print(data);
	TelnetClient[0].print(data);
}
void sendTelnetln(const char * data)
{
	Serial.println(data);
	TelnetClient[0].println(data);
}
void sendTelnetF(const char * format, ...)//  __attribute__((format(printf, 2, 3)))
{
	Serial.printf(format);
	TelnetClient[0].printf(format);
}
#endif

//Format and output string to serial debug port
void SerialPrint2(char *format, ...)
{
	char buff[128];
	va_list args;
	va_start(args, format);

	vsnprintf(buff, sizeof(buff), format, args);
	va_end(args);
	buff[sizeof(buff) / sizeof(buff[0]) - 1] = '\0';
	Serial.print(buff);
}

