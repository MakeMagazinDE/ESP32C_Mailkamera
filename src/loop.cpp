void loop() {
  const uint8_t TOUCH_THRESHOLD = 20;
  const bool touched = (touchRead(T5) < TOUCH_THRESHOLD);

  if (touched) {
    Serial.println("Schiesse Foto...");
    camera_fb_t* frameBuffer = esp_camera_fb_get();
    if (!frameBuffer) {
      Serial.println("Foto wurde nicht aufgenommen.");
    } else {
      storePhoto(PHOTO_PATH, frameBuffer);
      esp_camera_fb_return(frameBuffer);
      sendPhoto();
    }
  }
}
