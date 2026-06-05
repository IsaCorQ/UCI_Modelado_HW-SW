#define SENSOR_PIN A0

int sensorMin = 0;
int sensorMax = 800;
int lecturaActual = 0;
float humedad = 0;

void setup() {
  Serial.begin(9600);
  delay(500);
  Serial.println("Sensor de humedad iniciado");
}

void loop() {
  lecturaActual = analogRead(SENSOR_PIN);
  humedad = map(lecturaActual, sensorMin, sensorMax, 0, 100);
  humedad = constrain(humedad, 0, 100);
  
  Serial.print(lecturaActual);
  Serial.print(" | ");
  Serial.print(humedad);
  Serial.println("%");
  
  delay(1000);
}
