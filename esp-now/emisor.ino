#include <esp_now.h>  // Librería para comunicación ESP-NOW
#include <WiFi.h>     // Librería WiFi para ESP32

#define ESPNOW_WIFI_CHANNEL 4

uint8_t unicastAddress[] = {0x08, 0xD1, 0xF9, 0xDE, 0x03, 0x48};

esp_now_peer_info_t peerInfo;

bool turnOnLed = false; 

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.print("\r\nEstado del último envío:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Entrega exitosa" : "Fallo en entrega");
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error al inicializar ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  memcpy(peerInfo.peer_addr, unicastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Error al añadir el receptor");
    return;
  }
}

void loop() {
  esp_err_t result = esp_now_send(unicastAddress, (uint8_t *) &turnOnLed, sizeof(turnOnLed));

  if (result == ESP_OK)
  {
    Serial.println("Datos enviados con éxito");
    Serial.print("Datos enviados: ");
    Serial.println(*(uint8_t *) &turnOnLed);
  }
  else
  {
    Serial.println("Error al enviar los datos");
  }

  turnOnLed = !turnOnLed;
  delay(1000);
}
