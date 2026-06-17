#include <DHT.h>
#include <Adafruit_VEML7700.h>
#include <Wire.h>

#define DHTPIN 2
#define DHTTYPE DHT22

#define SOIL_HUMIDITY_PIN A0
#define HUMIDIFIER_PIN 3
#define MOTOR_PIN 5
#define LED_PIN 6

DHT dht(DHTPIN, DHTTYPE);
Adafruit_VEML7700 veml = Adafruit_VEML7700();

float dhtHumidity = 0;
float temperature = 0;
int soilHumidity = 0;
float lux = 0;

int soilMin = 0, soilMax = 800;
float TEMP_HIGH = 28.0;
float LUX_LOW = 200.0;
int SOIL_DRY_ADC = 300;

int chipState = 3;

void setup() {
  Serial.begin(9600);
  delay(1000);
  
  Serial.println("Inicializando sensores...");
  
  dht.begin();
  
  if (!veml.begin()) {
    Serial.println("VEML7700 sensor not found!");
    while (1);
  }
  
  veml.setGain(VEML7700_GAIN_1);
  veml.setIntegrationTime(VEML7700_IT_100MS);
  
  pinMode(HUMIDIFIER_PIN, OUTPUT);
  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  
  digitalWrite(HUMIDIFIER_PIN, LOW);
  digitalWrite(MOTOR_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  
  Serial.println("Todos los sensores inicializados!");
  Serial.println("Iniciando lecturas...\n");
}

void loop() {
  float hum = dht.readHumidity();
  float temp = dht.readTemperature();
  
  soilHumidity = analogRead(SOIL_HUMIDITY_PIN);
  int soilPercent = map(soilHumidity, soilMin, soilMax, 0, 100);
  soilPercent = constrain(soilPercent, 0, 100);
  
  lux = veml.readLux();
  
  int targetState = 3;
  if (soilHumidity < 400) {
    targetState = 2;
  }
  
  if (targetState != chipState) {
    int pulsesNeeded = (targetState - chipState + 3) % 3;
    if (pulsesNeeded == 0) pulsesNeeded = 3;
    
    for (int i = 0; i < pulsesNeeded; i++) {
      delay(100);
      digitalWrite(HUMIDIFIER_PIN, HIGH);
      delay(500);
      digitalWrite(HUMIDIFIER_PIN, LOW);
    }
    chipState = targetState;
  }
  
  if (temp > TEMP_HIGH) {
    digitalWrite(MOTOR_PIN, HIGH);
  } else {
    digitalWrite(MOTOR_PIN, LOW);
  }
  
  if (lux < LUX_LOW) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
  
  Serial.print("ADC: ");
  Serial.print(soilHumidity);
  Serial.print(" | Soil%: ");
  Serial.print(soilPercent);
  Serial.print(" | DHT Hum: ");
  Serial.print(hum);
  Serial.print("% | Temp: ");
  Serial.print(temp);
  Serial.print("C | Lux: ");
  Serial.print(lux);
  Serial.print(" | Humid State: ");
  Serial.print(chipState);
  Serial.print(" | Motor: ");
  Serial.print(digitalRead(MOTOR_PIN) ? "ON" : "OFF");
  Serial.print(" | LED: ");
  Serial.println(digitalRead(LED_PIN) ? "ON" : "OFF");
  
  delay(2000);
}
