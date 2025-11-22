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
const int mqtt_port = 8883;                                // Puerto para MQTT con TLS
const char* mqtt_topic = "dispositivo/device1/ota";
const char* mqtt_client_id = "ESP32Client";
const char* mqtt_username = "admin";  // Opcional: usuario MQTT
const char* mqtt_password = "admin*"; // Opcional: contraseña MQTT


// URL para OTA
const char* ota_url = "http://ota.daniela.freeddns.org/firmware.bin";

// Certificado de Let's Encrypt para uceva-iot-core.freeddns.org
//const char* root_ca = "-----BEGIN CERTIFICATE-----MIIEVzCCAj+gAwIBAgIRAKp18eYrjwoiCWbTi7/UuqEwDQYJKoZIhvcNAQELBQAwTzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2VhcmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMjQwMzEzMDAwMDAwWhcNMjcwMzEyMjM1OTU5WjAyMQswCQYDVQQGEwJVUzEWMBQGA1UEChMNTGV0J3MgRW5jcnlwdDELMAkGA1UEAxMCRTcwdjAQBgcqhkjOPQIBBgUrgQQAIgNiAARB6ASTCFh/vjcwDMCgQer+VtqEkz7JANurZxLP+U9TCeioL6sp5Z8VRvRbYk4P1INBmbefQHJFHCxcSjKmwtvGBWpl/9ra8HW0QDsUaJW2qOJqceJ0ZVFT3hbUHifBM/2jgfgwgfUwDgYDVR0PAQH/BAQDAgGGMB0GA1UdJQQWMBQGCCsGAQUFBwMCBggrBgEFBQcDATASBgNVHRMBAf8ECDAGAQH/AgEAMB0GA1UdDgQWBBSuSJ7chx1EoG/aouVgdAR4wpwAgDAfBgNVHSMEGDAWgBR5tFnme7bl5AFzgAiIyBpY9umbbjAyBggrBgEFBQcBAQQmMCQwIgYIKwYBBQUHMAKGFmh0dHA6Ly94MS5pLmxlbmNyLm9yZy8wEwYDVR0gBAwwCjAIBgZngQwBAgEwJwYDVR0fBCAwHjAcoBqgGIYWaHR0cDovL3gxLmMubGVuY3Iub3JnLzANBgkqhkiG9w0BAQsFAAOCAgEAjx66fDdLk5ywFn3CzA1w1qfylHUDaEf0QZpXcJseddJGSfbUUOvbNR9N/QQ16K1lXl4VFyhmGXDT5Kdfcr0RvIIVrNxFh4lqHtRRCP6RBRstqbZ2zURgqakn/Xip0iaQL0IdfHBZr396FgknniRYFckKORPGyM3QKnd66gtMst8I5nkRQlAg/Jb+Gc3egIvuGKWboE1G89NTsN9LTDD3PLj0dUMrOIuqVjLB8pEC6yk9enrlrqjXQgkLEYhXzq7dLafv5Vkig6Gl0nuuqjqfp0Q1bi1oyVNAlXe6aUXw92CcghC9bNsKEO1+M52YY5+ofIXlS/SEQbvVYYBLZ5yeiglV6t3SM6H+vTG0aP9YHzLn/KVOHzGQfXDP7qM5tkf+7diZe7o2fw6O7IvN6fsQXEQQj8TJUXJxv2/uJhcuy/tSDgXwHM8Uk34WNbRT7zGTGkQRX0gsbjAea/jYAoWv0ZvQRwpqPe79D/i7Cep8qWnA+7AE/3B3S/3dEEYmc0lpe1366A/6GEgk3ktr9PEoQrLChs6Itu3wnNLB2euC8IKGLQFpGtOO/2/hiAKjyajaBP25w1jF0Wl8Bbqne3uZ2q1GyPFJYRmT7/OXpmOH/FVLtwS+8ng1cAmpCujPwteJZNcDG0sF2n/sc0+SQf49fdyUK0ty+VUwFj9tmWxyR/M=-----END CERTIFICATE-----";
// Certificado de Let's Encrypt para uceva-iot-core.freeddns.org

const char* root_ca = "-----BEGIN CERTIFICATE-----MIIDnzCCAyWgAwIBAgISBaenrZXmRt9jw0c5zp/lgXLLMAoGCCqGSM49BAMDMDIxCzAJBgNVBAYTAlVTMRYwFAYDVQQKEw1MZXQncyBFbmNyeXB0MQswCQYDVQQDEwJFNzAeFw0yNTExMjEyMjA0MDlaFw0yNjAyMTkyMjA0MDhaMCQxIjAgBgNVBAMTGW1xdHQuZGFuaWVsYS5mcmVlZGRucy5vcmcwWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAAQ+QjlcbDx5B0mSe7psuwebOhVLIIRarat2OkC/dThYbgj1JMeoCQP89BZxNuSI191SLIiU5BfEUsCWh0yRRrFLo4ICJzCCAiMwDgYDVR0PAQH/BAQDAgeAMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjAMBgNVHRMBAf8EAjAAMB0GA1UdDgQWBBTRmA9XZeA1rYs1IP35Keb4BmOedDAfBgNVHSMEGDAWgBSuSJ7chx1EoG/aouVgdAR4wpwAgDAyBggrBgEFBQcBAQQmMCQwIgYIKwYBBQUHMAKGFmh0dHA6Ly9lNy5pLmxlbmNyLm9yZy8wJAYDVR0RBB0wG4IZbXF0dC5kYW5pZWxhLmZyZWVkZG5zLm9yZzATBgNVHSAEDDAKMAgGBmeBDAECATAtBgNVHR8EJjAkMCKgIKAehhxodHRwOi8vZTcuYy5sZW5jci5vcmcvNzYuY3JsMIIBBAYKKwYBBAHWeQIEAgSB9QSB8gDwAHYAyzj3FYl8hKFEX1vB3fvJbvKaWc1HCmkFhbDLFMMUWOcAAAGaqKfJaAAABAMARzBFAiEA2VmxrENtA0576gBAADaaG499tdyqtSCm21tMGhOJNPkCIBhmugre3BlBNwPGo5sWjfzjUNy3NkrzQfZQfr0se9BnAHYADleUvPOuqT4zGyyZB7P3kN+bwj1xMiXdIaklrGHFTiEAAAGaqKfJKAAABAMARzBFAiAJuSA0hl5nkFxdqFyoFLlBIsmWiGduaQbCU2pIRp7pcQIhAO4TDte1KkpYb+TnHVoGayeoYw8U1rr/HCOrylXA04TGMAoGCCqGSM49BAMDA2gAMGUCME/CMMo+dhYXCFt5gLxM2yjZTfAOOpDxJwdYe8ECZJtTypPui0LS8R1BqVbu6tCyUAIxANybOhY8XFaECwga7m5OF0W7xl5OLMMLL+7wGQ54ISFrwx1ryPa60Et7G8WLlmGlWg==-----END CERTIFICATE----------BEGIN CERTIFICATE-----MIIEVzCCAj+gAwIBAgIRAKp18eYrjwoiCWbTi7/UuqEwDQYJKoZIhvcNAQELBQAwTzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2VhcmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMjQwMzEzMDAwMDAwWhcNMjcwMzEyMjM1OTU5WjAyMQswCQYDVQQGEwJVUzEWMBQGA1UEChMNTGV0J3MgRW5jcnlwdDELMAkGA1UEAxMCRTcwdjAQBgcqhkjOPQIBBgUrgQQAIgNiAARB6ASTCFh/vjcwDMCgQer+VtqEkz7JANurZxLP+U9TCeioL6sp5Z8VRvRbYk4P1INBmbefQHJFHCxcSjKmwtvGBWpl/9ra8HW0QDsUaJW2qOJqceJ0ZVFT3hbUHifBM/2jgfgwgfUwDgYDVR0PAQH/BAQDAgGGMB0GA1UdJQQWMBQGCCsGAQUFBwMCBggrBgEFBQcDATASBgNVHRMBAf8ECDAGAQH/AgEAMB0GA1UdDgQWBBSuSJ7chx1EoG/aouVgdAR4wpwAgDAfBgNVHSMEGDAWgBR5tFnme7bl5AFzgAiIyBpY9umbbjAyBggrBgEFBQcBAQQmMCQwIgYIKwYBBQUHMAKGFmh0dHA6Ly94MS5pLmxlbmNyLm9yZy8wEwYDVR0gBAwwCjAIBgZngQwBAgEwJwYDVR0fBCAwHjAcoBqgGIYWaHR0cDovL3gxLmMubGVuY3Iub3JnLzANBgkqhkiG9w0BAQsFAAOCAgEAjx66fDdLk5ywFn3CzA1w1qfylHUDaEf0QZpXcJseddJGSfbUUOvbNR9N/QQ16K1lXl4VFyhmGXDT5Kdfcr0RvIIVrNxFh4lqHtRRCP6RBRstqbZ2zURgqakn/Xip0iaQL0IdfHBZr396FgknniRYFckKORPGyM3QKnd66gtMst8I5nkRQlAg/Jb+Gc3egIvuGKWboE1G89NTsN9LTDD3PLj0dUMrOIuqVjLB8pEC6yk9enrlrqjXQgkLEYhXzq7dLafv5Vkig6Gl0nuuqjqfp0Q1bi1oyVNAlXe6aUXw92CcghC9bNsKEO1+M52YY5+ofIXlS/SEQbvVYYBLZ5yeiglV6t3SM6H+vTG0aP9YHzLn/KVOHzGQfXDP7qM5tkf+7diZe7o2fw6O7IvN6fsQXEQQj8TJUXJxv2/uJhcuy/tSDgXwHM8Uk34WNbRT7zGTGkQRX0gsbjAea/jYAoWv0ZvQRwpqPe79D/i7Cep8qWnA+7AE/3B3S/3dEEYmc0lpe1366A/6GEgk3ktr9PEoQrLChs6Itu3wnNLB2euC8IKGLQFpGtOO/2/hiAKjyajaBP25w1jF0Wl8Bbqne3uZ2q1GyPFJYRmT7/OXpmOH/FVLtwS+8ng1cAmpCujPwteJZNcDG0sF2n/sc0+SQf49fdyUK0ty+VUwFj9tmWxyR/M=-----END CERTIFICATE-----";                     ///< CA vacía por defecto; definir vía .env

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