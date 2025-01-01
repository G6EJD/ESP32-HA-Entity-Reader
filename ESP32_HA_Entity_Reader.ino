#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>  // https://github.com/bblanchon/ArduinoJson
#include "credentials.h"  // WiFi and HA API Details

// credentiasl.h contains the following 4 lines
// char ssid[]            = "yourSSID";                // your network SSID
// char password[]        = "yourPASSWORD";            // your network PASSWORD
// String API_Key         = "yourHomeAssistantAPIKey"; // From HA See readme
// String HomeAssistantIP = "192.168.0.1"              // your Home Asssistant IP Address (example here)

void setup() {
  Start_Serial();
  Start_WiFi();
}

void loop() {
  Serial.println("\nUpdating... --------------------------------------");
  GetEntityData("sensor.windspeed"); // parse the entity name to be read as a value
  GetEntityData("sensor.winddirection");
  GetEntityData("sensor.temp");
  GetEntityData("sensor.hum");
  delay(30000);
}

void Start_Serial() {
  Serial.begin(115200);
  while (!Serial); delay(100);
  Serial.println("Serial started...");
  Serial.println(__FILE__);
}

void Start_WiFi() {
  Serial.println("Starting WiFi...\nConnecting to: " + String(ssid));
  WiFi.mode(WIFI_STA);  // switch off AP
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, password);
  unsigned long start = millis();
  uint8_t connectionStatus;
  bool AttemptConnection = true;
  while (AttemptConnection) {
    connectionStatus = WiFi.status();
    if (millis() > start + 15000) {  // Wait 15-secs maximum
      AttemptConnection = false;
    }
    if (connectionStatus == WL_CONNECTED || connectionStatus == WL_CONNECT_FAILED) {
      AttemptConnection = false;
    }
    delay(50);
  }
  if (connectionStatus == WL_CONNECTED) Serial.println("WiFi connected at: " + WiFi.localIP().toString());
  else Serial.println("WiFi connection *** FAILED ***");
}

// ###################### Get Entity Data #######################
void GetEntityData(String EntityName) {
  HTTPClient http;
  //Serial.println("Getting Entity Data...");
  String url     = "http://" + HomeAssistantIP + ":8123/api/states/" + EntityName;
  //Serial.println(url);
  String Auth    = "Bearer " + API_Key;
  String Content = "application/json";
  String Accept  = "application/json";
  String Payload = "";
  http.begin(url);  //Specify destination for HTTP request
  http.addHeader("Authorization", Auth);
  http.addHeader("Content-type", Content);
  http.addHeader("Accept", Accept);
  int httpResponseCode = http.GET();
  if (httpResponseCode >= 200) {
    //Serial.println("Received: " + String(httpResponseCode) + " as response Code");
    String response = http.getString();  //Get the response to the request
    //Serial.println(response);
    DecodeEntity(EntityName, response);
  } else Serial.println(httpResponseCode);
  http.end();
}

void DecodeEntity(String EntityName, String input) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, input);
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }
  const char* entity_id           = doc["entity_id"]; // "sensor.windspeed"
  const char* state               = doc["state"]; // "5.0"  Serial.println(value);
  const char* unit_of_measurement = doc["attributes"]["unit_of_measurement"];
  const char* friendlyName        = doc["attributes"]["friendly_name"];
  
  Serial.println(String(friendlyName) + " = " + String(state) + String(unit_of_measurement) + " (" + EntityName + ")");
}  

/*
	object		{7}
    entity_id	:	sensor.hum
    state	:	96.7
	attributes		{3}
    unit_of_measurement	:	%
    device_class	:	humidity
    friendly_name	:	Humidity
    last_changed	:	2025-01-01T15:30:33.916545+00:00
    last_reported	:	2025-01-01T15:30:33.916545+00:00
    last_updated	:	2025-01-01T15:30:33.916545+00:00
	context		{3}
    id	:	01JGH7TW9V0QSSM1Q7VF1M758R
    parent_id	:	null
    user_id	:	null

{"entity_id":"sensor.hum","state":"96.7","attributes":{"unit_of_measurement":"%","device_class":"humidity","friendly_name":"Humidity"},"last_changed":"2025-01-01T15:30:33.916545+00:00","last_reported":"2025-01-01T15:30:33.916545+00:00","last_updated":"2025-01-01T15:30:33.916545+00:00","context":{"id":"01JGH7TW9V0QSSM1Q7VF1M758R","parent_id":null,"user_id":null}}
*/
