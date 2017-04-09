#include <WiFiUdp.h>

unsigned int localPort = 8888;      // local port to listen on

char packetBuffer[UDP_TX_PACKET_MAX_SIZE];  //buffer to hold incoming packet,
WiFiUDP Udp;								// An EthernetUDP instance to let us send and receive packets over UDP

void setupUDP() {
	Udp.begin(localPort);
	DEBUG_PRINTST("UDP started on %d\n", localPort);
}

void checkUDP() {
	char s[3];
	char buf[16] = { 0 };

	// if there's data available, read a packet
	int packetSize = Udp.parsePacket();
	if (packetSize) {
		packetBuffer[packetSize + 1] = 0;
		DEBUG_PRINTST("Received packet of size %d\n", packetSize);
		IPAddress remote = Udp.remoteIP();
		//for (int i = 0; i < 4; i++) {
		//	sprintf(s, "%d", remote[i]);
		//	strcat(buf, s);
		//	if (i < 3) { strcat(buf, ".");}
		//}
		DEBUG_PRINTST("From %d.%d.%d.%d, port %d\n", remote[0], remote[1], remote[2], remote[3], Udp.remotePort());

		// read the packet into packetBufffer
		Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
		DEBUG_PRINTST("Contents: %s\n", packetBuffer);

		// send a reply to the IP address and port that sent us the packet we received
		char reply[128];
		sprintf(reply, "%s,%s,%s,%d", celsiusTemp, dewPointTemp, humidityTemp, freshData);
		DEBUG_PRINTST("Sending %s\n", reply);
		Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
		Udp.write(reply, strlen(reply));
		Udp.endPacket();
	}
	delay(10);
}


