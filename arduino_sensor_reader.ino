

#include <DHT.h>
#include <ArduinoJson.h>

// ========== PIN CONFIGURATION ==========
#define DHTPIN 2           // DHT22 data pin
#define DHTTYPE DHT22
#define SOIL_HUMIDITY_PIN A0  // Soil moisture sensor (analog pin)
#define MOTOR_PIN 5        // Motor control pin (HIGH = motor on)
#define LED_PIN 6          // Status LED

// ========== SENSOR INITIALIZATION ==========
DHT dht(DHTPIN, DHTTYPE);

// ========== VARIABLES ==========
float temperature = 0.0;
float humidity = 0.0;
int soilMoisture = 0;
bool motorActive = false;
unsigned long lastSendTime = 0;
const unsigned long SEND_INTERVAL = 2000;  // Send data every 2 seconds

// ========== SETUP ==========
void setup() {
  // Initialize Serial communication (9600 baud for ESP8266 compatibility)
  Serial.begin(9600);
  delay(1000);
  
  Serial.println("\n\n===== ARDUINO SENSOR READER =====");
  Serial.println("Initializing sensors...");
  
  // Initialize DHT sensor
  dht.begin();
  
  // Initialize pins
  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, LOW);  // Motor off initially
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  pinMode(SOIL_HUMIDITY_PIN, INPUT);
  
  Serial.println("Ready to communicate with ESP8266");
  Serial.println("Sending sensor data every 2 seconds...");
}

// ========== MAIN LOOP ==========
void loop() {
  // Check for commands from ESP8266
  if (Serial.available()) {
    handleCommand();
  }
  
  // Read sensors and send data periodically
  if (millis() - lastSendTime >= SEND_INTERVAL) {
    readSensors();
    sendToESP8266();
    lastSendTime = millis();
  }
  
  delay(10);
}

// ========== READ SENSORS ==========
void readSensors() {
  // Read DHT22 (temperature and humidity)
  float newTemp = dht.readTemperature();
  float newHumidity = dht.readHumidity();
  
  // Validate DHT readings
  if (!isnan(newTemp)) {
    temperature = newTemp;
  }
  if (!isnan(newHumidity)) {
    humidity = newHumidity;
  }
  
  // Read soil moisture (analog sensor)
  soilMoisture = analogRead(SOIL_HUMIDITY_PIN);
  
  // Local debug output
  Serial.print("[ARDUINO] Temp: ");
  Serial.print(temperature);
  Serial.print("°C | Humidity: ");
  Serial.print(humidity);
  Serial.print("% | Soil: ");
  Serial.print(soilMoisture);
  Serial.print(" | Motor: ");
  Serial.println(motorActive ? "ON" : "OFF");
}

// ========== SEND DATA TO ESP8266 ==========
void sendToESP8266() {
  // Create JSON document with sensor data
  DynamicJsonDocument doc(256);
  doc["temp"] = temperature;
  doc["humidity"] = humidity;
  doc["soil"] = soilMoisture;
  doc["motor"] = motorActive ? "ON" : "OFF";
  
  // Serialize and send to ESP8266
  serializeJson(doc, Serial);
  Serial.println();  // Add newline for proper parsing
}

// ========== HANDLE COMMANDS FROM ESP8266 ==========
void handleCommand() {
  String command = Serial.readStringUntil('\n');
  command.trim();
  
  if (command.length() > 0) {
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, command);
    
    if (!error && doc.containsKey("action")) {
      String action = doc["action"];
      
      if (action == "motor_on") {
        digitalWrite(MOTOR_PIN, HIGH);
        motorActive = true;
        digitalWrite(LED_PIN, HIGH);
        Serial.println("[ARDUINO] Motor ON (commanded by ESP8266)");
      } 
      else if (action == "motor_off") {
        digitalWrite(MOTOR_PIN, LOW);
        motorActive = false;
        digitalWrite(LED_PIN, LOW);
        Serial.println("[ARDUINO] Motor OFF (commanded by ESP8266)");
      }
    }
  }
}
