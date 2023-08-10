const uint8_t TOUCH_THRESHOLD = 20;

bool touchedT5 = false;

void T5wasActivated() {
  touchedT5 = true;
}

void setup() {
  Serial.begin(115200);
  touchAttachInterrupt(T5, T5wasActivated, TOUCH_THRESHOLD);
}

void loop() {
  if (touchedT5) {
    touchedT5 = false;
    Serial.println("Sensor T5");
  }
}
