#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <HTTPClient.h>

// Definições do Wi-Fi
const char* ssid = "trea";
const char* password = "banana123";

// Definições do MQTT
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* mqtt_topic = "trabalho-iot-unoesc";

// Definições do HTTP
const char* serverName = "http://192.168.184.38:5000/data"; // Altere para o IP do seu PC na rede

WiFiClient espClient;
PubSubClient client(espClient);

// Configurações do DHT
#define DHTPIN 15           // GPIO15
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

void setup_wifi() {
  delay(10);
  Serial.println("Conectando-se ao Wi-Fi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Wi-Fi conectado");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conexão com MQTT...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("conectado");
    } else {
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Falha ao ler o sensor DHT11!");
    return;
  }

  String payload = "{";
  payload += "\"Temperatura\":";
  payload += t;
  payload += ",\"Umidade\":";
  payload += h;
  payload += "}";

  Serial.print("Enviando payload MQTT: ");
  Serial.println(payload);

  client.publish(mqtt_topic, payload.c_str());

  // Envia dados para o servidor Flask via HTTP POST
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.print("Resposta HTTP: ");
      Serial.println(response);
    } else {
      Serial.print("Erro no POST: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi desconectado, não foi possível enviar via HTTP");
  }

  delay(5000); // Aguarda 5 segundos para nova leitura
}
