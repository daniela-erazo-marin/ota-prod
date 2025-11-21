// ESP32 OTA Update via HTTPS and MQTT with TLS
// Basado en el repositorio: https://github.com/alvaro-salazar/iot-mqtt-tls.git

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <esp_https_ota.h>
#include <ArduinoJson.h>

// Configuración WiFi
const char* ssid = "MARIN_G";
const char* password = "Jacobo20035";

// Configuración MQTT
const char* mqtt_broker = "mqtt.daniela.freeddns.org";  // Broker MQTT con TLS
const int mqtt_port = 18083;  //8883                               // Puerto para MQTT con TLS
const char* mqtt_topic = "dispositivo/device1/ota";
const char* mqtt_client_id = "ESP32Client";
const char* mqtt_username = "DaniMarin";  // Opcional: usuario MQTT
const char* mqtt_password = "Uceva2025*"; // Opcional: contraseña MQTT


// URL para OTA
const char* ota_url = "https://ota.daniela.freeddns.org/firmware.bin";

// Certificado de Let's Encrypt para uceva-iot-core.freeddns.org
const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"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\n" \
"-----END CERTIFICATE-----\n";

// Versión actual del firmware
const char* CURRENT_VERSION = "1.0.0";

// Cliente WiFi seguro para MQTT
WiFiClientSecure wifiClient;
// Cliente MQTT
PubSubClient mqttClient(wifiClient);

// Función para iniciar la actualización OTA
void startOTA(const char* url) {
  Serial.println("[OTA] Iniciando actualización...");

  esp_http_client_config_t config;
  memset(&config, 0, sizeof(esp_http_client_config_t));
  config.url = url;
  config.cert_pem = root_ca;  // Usar el certificado de Let's Encrypt
  config.timeout_ms = 10000;

  esp_err_t ret = esp_https_ota(&config);

  if (ret == ESP_OK) {
    Serial.println("[OTA] Actualización exitosa, reiniciando...");
    delay(1000);
    ESP.restart();
  } else {
    Serial.printf("[OTA] Falló la actualización: %d\n", ret);
  }
}

// Función para manejar mensajes MQTT
void onMqttMessage(char* topic, byte* payload, unsigned int length) {
  Serial.printf("[MQTT] Mensaje en topic %s\n", topic);

  // Convertir el payload a una cadena terminada en null
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';

  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, message);
  if (error) {
    Serial.println("[MQTT] Error al parsear JSON");
    return;
  }

  const char* version = doc["version"];
  const char* url = doc["url"];

  if (strcmp(version, CURRENT_VERSION) != 0) {
    Serial.printf("[MQTT] Nueva versión detectada: %s\n", version);
    startOTA(url);
  } else {
    Serial.println("[MQTT] Ya tienes la última versión");
  }
}

// Función para conectar a WiFi
void connectToWifi() {
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado. IP: " + WiFi.localIP().toString());
}

// Función para conectar a MQTT
void connectToMqtt() {
  // Configurar el certificado para TLS
  wifiClient.setCACert(root_ca);
  
  // Configurar el cliente MQTT
  mqttClient.setServer(mqtt_broker, mqtt_port);
  mqttClient.setCallback(onMqttMessage);
  
  // Intentar conectar al broker MQTT
  Serial.println("Conectando al broker MQTT...");
  while (!mqttClient.connected()) {
    if (mqttClient.connect(mqtt_client_id, mqtt_username, mqtt_password)) {
      Serial.println("[MQTT] Conectado al broker");
      mqttClient.subscribe(mqtt_topic);
    } else {
      Serial.print(".");
      delay(2000);
    }
  }
}

// Función para publicar un mensaje MQTT
void publishMqttMessage(const char* topic, const char* message) {
  if (mqttClient.connected()) {
    mqttClient.publish(topic, message);
  }
}

// Función para publicar el estado del dispositivo
void publishDeviceStatus() {
  StaticJsonDocument<200> doc;
  doc["device_id"] = mqtt_client_id;
  doc["version"] = CURRENT_VERSION;
  doc["ip"] = WiFi.localIP().toString();
  doc["rssi"] = WiFi.RSSI();
  
  char buffer[200];
  serializeJson(doc, buffer);
  
  publishMqttMessage("dispositivo/device1/status", buffer);
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n[INICIO] ESP32 OTA Update via HTTPS and MQTT with TLS");
  Serial.printf("[INFO] Versión actual: %s\n", CURRENT_VERSION);
  
  connectToWifi();
  connectToMqtt();
  
  // Publicar estado inicial
  publishDeviceStatus();
}

void loop() {
  // PubSubClient necesita que llamemos a loop() regularmente
  mqttClient.loop();
  
  // Reconectar a MQTT si se perdió la conexión
  if (!mqttClient.connected()) {
    Serial.println("[MQTT] Conexión perdida, reconectando...");
    connectToMqtt();
  }
  
  // Publicar estado cada 5 minutos
  static unsigned long lastStatusPublish = 0;
  if (millis() - lastStatusPublish > 300000) {
    publishDeviceStatus();
    lastStatusPublish = millis();
  }
} 