String strHTML;

void setupWebServer()
{
	// attach AsyncEventSource
	//server.addHandler(&events);
	server.addHandler(&debugEvent);

	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {sendStatusPage(request); });
	server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {sendStatusPage(request); });
	server.on("/debug", HTTP_GET, [](AsyncWebServerRequest *request) {request->send(SPIFFS, "/debug.html"); });

	server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request) {
		DEBUG_PRINTLN("settings");
		buildSettingsXML();
		request->send(200, "text/xml", strHTML);
	});

	server.on("/devReboot", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(200, "text/plain", "Rebooting...");
		shouldReboot = true;
	});

	server.serveStatic("/css/style.css", SPIFFS, "/css/style.css");
	server.serveStatic("/css/table-column-options.css", SPIFFS, "/css/table-column-options.css");
	server.serveStatic("/images/bg.png", SPIFFS, "/images/bg.png");

	// Catch-All Handlers
	// Any request that can not find a Handler that canHandle it ends in the callbacks below.
	server.onNotFound(onRequest);
	server.onFileUpload(onUpload);
	server.onRequestBody(onBody);
	server.begin();
}

void sendStatusPage(AsyncWebServerRequest *request) {
	request->send(SPIFFS, "/status.html");
}

void onRequest(AsyncWebServerRequest *request) {
	//Handle Unknown Request
	request->send(404);
}

void onBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
	//Handle body
}

void onUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
	//Handle upload
}

void onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
	//Handle WebSocket event
}

// Build the XML file with settings for setup page
void buildSettingsXML()
{
	strHTML = "<?xml version = \"1.0\" ?><inputs><degc>";
	strHTML += celsiusTemp;
	strHTML += "</degc><dew>";
	strHTML += dewPointTemp;
	strHTML += "</dew><humid>";
	strHTML += humidityTemp;
	strHTML += "</humid><fresh>";
	strHTML += freshData;
	strHTML += "</fresh></inputs>";
}
