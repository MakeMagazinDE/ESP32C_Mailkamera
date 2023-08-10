const uint8_t LED_PIN = 33;
const uint8_t HALL_THRESHOLD = 200;
const uint16_t HALL_SAMPLES = 1000;

int16_t cleanHallRead() {
  int32_t value = 0;
  for (uint16_t i = 0; i < HALL_SAMPLES; i++) {
    value += hallRead();
    delayMicroseconds(100);
  }
  return value / HALL_SAMPLES;
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  const int16_t sensorValue = cleanHallRead();
  Serial.println(sensorValue);
  if (abs(sensorValue) > HALL_THRESHOLD)
    digitalWrite(LED_PIN, HIGH);
  else
    digitalWrite(LED_PIN, LOW);
}
