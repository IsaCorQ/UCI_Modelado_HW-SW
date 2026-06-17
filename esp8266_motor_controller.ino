/*
ESP8266 Temperature Motor Controller (Arduino Bridge)
- Receives sensor data from Arduino via Serial
- Receives commands from Python app via HTTP
- Controls motor based on temperature threshold
- Hosts a web server on local network
- Communicates with Arduino via TX/RX pins
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

// ========== WIFI CONFIGURATION ==========
const char* ssid = "YOUR_WIFI_SSID";           // Change to your WiFi name
const char* password = "YOUR_WIFI_PASSWORD";    // Change to your WiFi password

// ========== PIN CONFIGURATION ==========
// ESP8266 TX (GPIO1) and RX (GPIO3) are used for Serial communication with Arduino
#define MOTOR_PIN D8       // GPIO15 - Motor control pin
#define LED_PIN D0         // GPIO16 - Status LED

// ========== SENSOR AND VARIABLES ==========
ESP8266WebServer server(80);

float temperature = 0.0;
float humidity = 0.0;
float tempThreshold = 28.0;  // Default temperature threshold
bool motorActive = false;
unsigned long lastReadTime = 0;
const unsigned long READ_INTERVAL = 2000;  // Check for Arduino data every 2 seconds

// ========== SETUP ==========
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\nESP8266 Motor Controller Starting...");
  
  // Initialize pins
  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, LOW);  // Motor off initially
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // Initialize Serial communication with Arduino
  Serial.begin(9600);  // Standard Arduino baud rate
  delay(1000);
  Serial.println("ESP8266 ready to receive data from Arduino");
  
  // Connect to WiFi
  connectToWiFi();
  
  // Setup web server routes
  setupWebServer();
  
  Serial.println("Setup complete!");
}

// ========== MAIN LOOP ==========
void loop() {
  server.handleClient();
  
  // Read data from Arduino and check threshold
  if (millis() - lastReadTime >= READ_INTERVAL) {
    readFromArduino();
    checkTemperatureThreshold();
    lastReadTime = millis();
  }
  
  delay(10);
}

// ========== WIFI CONNECTION ==========
void connectToWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    digitalWrite(LED_PIN, HIGH);  // LED on when connected
  } else {
    Serial.println("\nFailed to connect to WiFi");
    digitalWrite(LED_PIN, LOW);
  }
}

// ========== READ DATA FROM ARDUINO ==========
void readFromArduino() {
  // Arduino sends JSON: {"temp":25.5,"humidity":60}
  if (Serial.available()) {
    String data = Serial.readStringUntil('\n');
    data.trim();
    
    if (data.length() > 0) {
      DynamicJsonDocument doc(256);
      DeserializationError error = deserializeJson(doc, data);
      
      if (!error) {
        if (doc.containsKey("temp")) {
          temperature = doc["temp"];
        }
        if (doc.containsKey("humidity")) {
          humidity = doc["humidity"];
        }
        
        Serial.print("[ESP] Received - Temp: ");
        Serial.print(temperature);
        Serial.print("°C | Humidity: ");
        Serial.print(humidity);
        Serial.print("% | Motor: ");
        Serial.println(motorActive ? "ON" : "OFF");
      }
    }
  }
}

// ========== TEMPERATURE THRESHOLD CHECK ==========
void checkTemperatureThreshold() {
  if (temperature >= tempThreshold && !motorActive) {
    activateMotor();
  } else if (temperature < (tempThreshold - 2.0) && motorActive) {
    // Hysteresis: motor stays on until temp drops 2 degrees below threshold
    deactivateMotor();
  }
}

// ========== MOTOR CONTROL ==========
void activateMotor() {
  digitalWrite(MOTOR_PIN, HIGH);
  motorActive = true;
  Serial.println("⚡ Motor activated (temperature threshold reached)");
}

void deactivateMotor() {
  digitalWrite(MOTOR_PIN, LOW);
  motorActive = false;
  Serial.println("🛑 Motor deactivated (temperature below threshold)");
}

// ========== WEB SERVER SETUP ==========
void setupWebServer() {
  // GET current sensor data
  server.on("/get_data", HTTP_POST, handleGetData);
  
  // POST set temperature threshold
  server.on("/set_threshold", HTTP_POST, handleSetThreshold);
  
  // POST motor control
  server.on("/motor_control", HTTP_POST, handleMotorControl);
  
  // GET status page (for testing in browser)
  server.on("/", HTTP_GET, handleStatus);
  
  // Handle not found
  server.onNotFound(handleNotFound);
  
  server.begin();
  Serial.println("Web server started on port 80");
}

// ========== WEB SERVER HANDLERS ==========
void handleGetData() {
  DynamicJsonDocument doc(256);
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["motor_status"] = motorActive ? "ON" : "OFF";
  doc["threshold"] = tempThreshold;
  doc["ip"] = WiFi.localIP().toString();
  
  String response;
  serializeJson(doc, response);
  
  server.sendHeader("Content-Type", "application/json");
  server.send(200, "application/json", response);
}

void handleSetThreshold() {
  if (server.hasArg("plain")) {
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    
    if (!error) {
      if (doc.containsKey("temperature")) {
        tempThreshold = doc["temperature"];
        Serial.print("Temperature threshold set to: ");
        Serial.println(tempThreshold);
        
        DynamicJsonDocument response(256);
        response["success"] = true;
        response["threshold"] = tempThreshold;
        response["message"] = "Threshold updated";
        
        String jsonResponse;
        serializeJson(response, jsonResponse);
        server.sendHeader("Content-Type", "application/json");
        server.send(200, "application/json", jsonResponse);
      } else {
        server.send(400, "application/json", "{\"success\": false, \"error\": \"Missing temperature field\"}");
      }
    } else {
      server.send(400, "application/json", "{\"success\": false, \"error\": \"Invalid JSON\"}");
    }
  } else {
    server.send(400, "application/json", "{\"success\": false, \"error\": \"No data provided\"}");
  }
}

void handleMotorControl() {
  if (server.hasArg("plain")) {
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    
    if (!error) {
      if (doc.containsKey("action")) {
        String action = doc["action"];
        if (action == "on") {
          activateMotor();
        } else if (action == "off") {
          deactivateMotor();
        }
        
        DynamicJsonDocument response(256);
        response["success"] = true;
        response["motor_status"] = motorActive ? "ON" : "OFF";
        
        String jsonResponse;
        serializeJson(response, jsonResponse);
        server.sendHeader("Content-Type", "application/json");
        server.send(200, "application/json", jsonResponse);
      } else {
        server.send(400, "application/json", "{\"success\": false, \"error\": \"Missing action field\"}");
      }
    } else {
      server.send(400, "application/json", "{\"success\": false, \"error\": \"Invalid JSON\"}");
    }
  } else {
    server.send(400, "application/json", "{\"success\": false, \"error\": \"No data provided\"}");
  }
}

void handleStatus() {
  String html = "<html><head><title>ESP8266 Motor Controller</title>";
  html += "<meta http-equiv=\"refresh\" content=\"2\">";
  html += "<style>body{font-family:Arial;margin:20px;} .status{border:1px solid #ccc;padding:10px;margin:10px 0;}</style>";
  html += "</head><body>";
  html += "<h1>🌡️ ESP8266 Motor Controller Status</h1>";
  html += "<div class='status'>";
  html += "<p><strong>Temperature:</strong> " + String(temperature) + "°C</p>";
  html += "<p><strong>Humidity:</strong> " + String(humidity) + "%</p>";
  html += "<p><strong>Temperature Threshold:</strong> " + String(tempThreshold) + "°C</p>";
  html += "<p><strong>Motor Status:</strong> <span style='color:" + String(motorActive ? "green" : "red") + ";'><strong>" + String(motorActive ? "ON" : "OFF") + "</strong></span></p>";
  html += "<p><strong>IP Address:</strong> " + WiFi.localIP().toString() + "</p>";
  html += "</div>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

void handleNotFound() {
  server.send(404, "application/json", "{\"error\": \"Endpoint not found\"}");
}
