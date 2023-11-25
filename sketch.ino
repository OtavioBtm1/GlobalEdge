#include "ArduinoJson.h"
#include "EspMQTTClient.h"
#include <DHT.h>

#define DHTPIN 2
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

DHT dht(DHTPIN, DHTTYPE);

// MQTT Configurações
EspMQTTClient client{
  "Wokwi-GUEST", // SSID do WiFi
  "",             // Senha do WiFi
  "mqtt.tago.io", // Endereço do servidor MQTT
  "Default",      // Usuario MQTT
  "7cd72f18-6a99-42c3-917b-9867934902df",   // Token do device
  "esp",          // Nome do device
  1883            // Porta de comunicação MQTT
};

char bufferJson[100];

void sendEmail(const char* topic) {
  client.publish(topic, "EnviarEmail"); // Envie um comando MQTT para acionar o envio de e-mail
}

void setup() {
  Serial.begin(9600);
  dht.begin();

  Serial.println("Conectando WiFi");
  while (!client.isWifiConnected()) {
    Serial.print('.');
    client.loop();
    delay(1000);
  }

  Serial.println("WiFi Conectado");

  Serial.println("Conectando com Servidor MQTT");
  while (!client.isMqttConnected()) {
    Serial.print('.');
    client.loop();
    delay(1000);
  }

  Serial.println("MQTT Conectado");
}

// Callback da EspMQTTClient
void onConnectionEstablished() {}

void loop() {
  float temperatura = dht.readTemperature();
  float umidade = dht.readHumidity();

  // Create JSON document for temperatura
  StaticJsonDocument<300> documentoJsonTemperatura;
  documentoJsonTemperatura["variable"] = "temperatura";
  documentoJsonTemperatura["value"] = temperatura;
  documentoJsonTemperatura["unit"] = "°C";

  serializeJson(documentoJsonTemperatura, bufferJson);
  Serial.println(bufferJson);

  // Publish temperatura data to MQTT
  client.publish("SeuTopicoTemperatura", bufferJson);

  // Check if temperature is above 37.5 or below 35
  if (temperatura > 37.5) {
    // If yes, send email for fever
    Serial.println("Payload MQTT para Envio de Email (Febre):");
    Serial.println(bufferJson);
    sendEmail("AvisarFebreEmail");
  } else if (temperatura < 35) {
    // If yes, send email for hypothermia
    Serial.println("Payload MQTT para Envio de Email (Hipotermia):");
    Serial.println(bufferJson);
    sendEmail("AvisarHipotermiaEmail");
  }

  delay(5000);
  client.loop();
}