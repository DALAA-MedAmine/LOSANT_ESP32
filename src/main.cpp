#include <config.h>

// initiate the the wifi client
WiFiClientSecure wifiClient;
LosantDevice device(LOSANT_DEVICE_ID);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

float tempC = 0.0;

void handleCommand(LosantCommand *command) {
  Serial.print("Command received: ");
  Serial.println(command->name);

  // Perform action specific to the command received.
  if(strcmp(command->name, "print") == 0) 
  {
    Serial.println("I received a command !");
  }
}

void connect() {

  // Connect to Wifi.
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  wifiClient.setCACert(rootCABuff);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    //WiFi.begin(WIFI_SSID, WIFI_PASS);
  }  

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Connect to Losant.
  Serial.println();
  Serial.print("Connecting to Losant...");

  device.connectSecure(wifiClient, LOSANT_ACCESS_KEY, LOSANT_ACCESS_SECRET);
  while(!device.connected()) {
    delay(500);
    Serial.print(".");
      //device.connect(wifiClient, LOSANT_ACCESS_KEY, LOSANT_ACCESS_SECRET);
  }

  Serial.println("Connected!");
}

void setup() {
  Serial.begin(115200);

  // Register the command handler to be called when a command is received
  // from the Losant platform.
  device.onCommand(&handleCommand);

  connect();
  millis_p = millis();
  sensors.begin();
}

void sendCommande() {
  Serial.println("Sending msg to server");

  // Losant uses a JSON protocol. Construct the simple state object.
  // { "button" : true }
  StaticJsonDocument<200> jsonBuffer;
  JsonObject root = jsonBuffer.to<JsonObject>();
  root["temp"] = tempC;

  // Send the state to Losant.
  device.sendState(root);
}


void loop() 
{
  sensors.requestTemperatures();
  bool toReconnect = false;

  tempC = sensors.getTempCByIndex(0);
  
  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("Disconnected from WiFi");
    toReconnect = true;
  }

  if(!device.connected()) {
    Serial.println("Disconnected from Losant");
    toReconnect = true;
  }

  if(toReconnect) {
    connect();
  }

  device.loop();

  if(millis() - millis_p > millis_delay)
  {
    sendCommande();
    millis_p = millis();
  }
}