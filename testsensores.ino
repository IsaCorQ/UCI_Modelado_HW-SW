// Incluimos librerías
#include <DHT.h>
#include <Adafruit_VEML7700.h>
#include <Wire.h>

// Definimos el pin digital donde se conecta el sensor DHT
#define DHTPIN 2
#define DHTTYPE DHT22   // DHT 22  (AM2302)

// Pin definitions
#define WATER_LEVEL_PIN A0 // Water level sensor
#define LUX_RELAY_PIN 4    // Digital output to activate relay/device

// Inicializamos el sensor DHT11
DHT dht(DHTPIN, DHTTYPE);
Adafruit_VEML7700 veml = Adafruit_VEML7700();

// Sensor readings
float humidity = 0;
float temperature = 0;
int waterLevel = 0;
float lux = 0;

// Lux threshold (adjust as needed)
float LUX_THRESHOLD = 500.0;

void setup() {
  // Inicializamos comunicación serie
  Serial.begin(9600);
  delay(1000);
  
  Serial.println("Inicializando sensores...");
  
  // Comenzamos el sensor DHT
  dht.begin();
  
  // Initialize VEML7700 lux sensor
  if (!veml.begin()) {
    Serial.println("VEML7700 sensor not found!");
    while (1);
  }
  
  // Set lux sensor gain and integration time
  veml.setGain(VEML7700_GAIN_1);
  veml.setIntegrationTime(VEML7700_IT_100MS);
  
  // Initialize digital output pin
  pinMode(LUX_RELAY_PIN, OUTPUT);
  digitalWrite(LUX_RELAY_PIN, LOW);
  
  Serial.println("Todos los sensores inicializados!");
  Serial.println("Iniciando lecturas...\n");
}

void loop() {
  // Read data and store it to variables hum and temp
  float hum = dht.readHumidity();
  float temp = dht.readTemperature();
  
  // Read water level (0-1023)
  waterLevel = analogRead(WATER_LEVEL_PIN);
  
  // Read lux value from VEML7700
  lux = veml.readLux();
  
  // Activate relay/output if lux exceeds threshold
  if (lux > LUX_THRESHOLD) {
    digitalWrite(LUX_RELAY_PIN, HIGH);
  } else {
    digitalWrite(LUX_RELAY_PIN, LOW);
  }
  
  // Print all sensor readings
  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.print(" %, Temp: ");
  Serial.print(temp);
  Serial.print(" Celsius | Water: ");
  Serial.print(waterLevel);
  Serial.print(" | Lux: ");
  Serial.print(lux);
  Serial.print(" | Relay: ");
  Serial.println(digitalRead(LUX_RELAY_PIN) ? "ON" : "OFF");
  
  delay(2000); // Delay 2 sec.
}
