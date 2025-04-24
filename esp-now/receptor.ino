/*
  Objetivo:
  Imprimir por serial cada N milisengudos la dirección MAC del dispositivo.
*/
#include <WiFi.h>
#include <esp_now.h>

#define LED_PIN 2

bool turnOnLed;

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len)
{
  // Copiar los datos recibidos a nuestra estructura
  memcpy(&turnOnLed, incomingData, sizeof(turnOnLed));

  // Imprimir información de la recepción
  Serial.print("Datos recibidos: ");
  Serial.println(*incomingData);

  Serial.print("Encender LED: ");
  Serial.println(turnOnLed ? "true" : "false");

  digitalWrite(LED_PIN, turnOnLed ? HIGH : LOW);
    
  // Salto de línea
  Serial.println();
}

void setup()
{
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(115200);
 
  WiFi.mode(WIFI_STA);

  digitalWrite(LED_PIN, LOW);

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error al inicializar ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}

void loop()
{
}