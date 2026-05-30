#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#else
  #include <WiFi.h>   
#endif

const char* SSID = "TU_WIFI";
const char* PASS = "TU_PASSWORD";

WiFiServer server(5000);

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("Conectando a WiFi...");
  WiFi.begin(SSID, PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi conectado");
  Serial.print("IP del dispositivo: ");
  Serial.println(WiFi.localIP());

  server.begin();
  Serial.println("Servidor TCP iniciado en puerto 5000");
}

void loop() {
  WiFiClient client = server.available();

  if (client) {
    Serial.println("Cliente conectado");

    String msg = "";
    unsigned long t0 = millis();

    while (client.connected() && (millis() - t0 < 5000)) {
      while (client.available()) {
        char c = (char)client.read();
        if (c == '\n') {
          msg.trim();
          Serial.print("Recibido: ");
          Serial.println(msg);

          if (msg == "PING") {
            client.println("PONG");
          } else {
            client.println("UNKNOWN");
          }

          msg = "";
          t0 = millis();
        } else {
          msg += c;
        }
      }
    }

    client.stop();
    Serial.println("Cliente desconectado");
  }
}
