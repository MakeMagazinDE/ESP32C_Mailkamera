#include "BluetoothSerial.h"

const uint8_t LED_PIN = 4;

BluetoothSerial bluetooth;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  bluetooth.begin("ESP32-CAM");
  bluetooth.register_callback(bluetoothCallback);
  Serial.println("ESP32-CAM-Board ist bereit zur Kopplung.");
}

void loop() {}

void bluetoothCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t* parameter) {
  if (event == ESP_SPP_START_EVT) {
    Serial.println("Serial Port Profile (SPP) initialisiert.");
  } else if (event == ESP_SPP_SRV_OPEN_EVT ) {
    Serial.println("Client verbunden.");
  } else if (event == ESP_SPP_CLOSE_EVT  ) {
    Serial.println("Client getrennt.");
  } else if (event == ESP_SPP_DATA_IND_EVT ) {
    Serial.println("Datenempfang.");
    while (bluetooth.available()) {
      const int16_t data = bluetooth.read();
      Serial.printf("Empfangen: %d\n", data);
      if (data == '1') {
        bluetooth.println("LED an.");
        digitalWrite(LED_PIN, HIGH);
      } else if (data == '0') {
        bluetooth.println("LED aus.");
        digitalWrite(LED_PIN, LOW);
      }
    }
  }
}
