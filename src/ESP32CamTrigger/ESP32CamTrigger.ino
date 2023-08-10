#include "ESP_Mail_Client.h"
#include "WiFi.h"
#include "init_camera.h"

const char* SMTP_ACCOUNT = "<SMTP-ACCOUNT EINTRAGEN>";
const char* SMTP_PASSWORD = "<SMTP-PASSWORT EINTRAGEN>";
const char* SMTP_SERVER = "<SMTP-SERVER EINTRAGEN>";
const uint16_t SMTP_PORT = 465;

const char* EMAIL_SUBJECT = "Schnappschuss vom ESP32-CAM-Board";
const char* EMAIL_RECIPIENT_ADDR = "<EMPFÄNGER-ADRESSE EINTRAGEN>";
const char* EMAIL_RECIPIENT_NAME = "<EMPFÄNGER-NAME EINTRAGEN>";
const char* PHOTO_NAME = "photo.jpg";
const char* PHOTO_PATH = "/photo.jpg";

const char* SSID = "<WLAN-SSID EINTRAGEN>";
const char* PASSWORD = "<WLAN-PASSWORT EINTRAGEN>";

SMTPSession smtp;

void setup() {
  Serial.begin(115200);
  initCamera();
  initFileSystem();
  initWiFi();
  smtp.debug(1);
  smtp.callback(smtpCallback);
}

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

void initWiFi() {
  WiFi.begin(SSID, PASSWORD);
  Serial.print("Verbinde mit WLAN...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("IP-Adresse: ");
  Serial.println(WiFi.localIP());
}

bool initFileSystem() {
  if (!LittleFS.begin(false)) {
    Serial.println("Dateisystem konnte nicht eingebunden werden.");
    if (!LittleFS.begin(true)) {
      Serial.println("Dateisystem konnte nicht formatiert werden.");
      return false;
    } else {
      Serial.println("Dateisystem wurde formatiert.");
      return true;
    }
  } else {
    Serial.println("Dateisystem ist OK.");
    return true;
  }
}

void storePhoto(const String& path, const camera_fb_t* frameBuffer) {
  File file = LittleFS.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Konnte Datei nicht erzeugen.");
  } else {
    file.write(frameBuffer->buf, frameBuffer->len);
    Serial.printf("Datei %s wurde gespeichert.\n", path.c_str());
    file.close();
  }
}

void sendPhoto() {
  SMTP_Attachment attachment;
  attachment.descr.mime = "image/jpeg";
  attachment.descr.filename = PHOTO_NAME;
  attachment.descr.transfer_encoding = Content_Transfer_Encoding::enc_base64;
  attachment.file.path = PHOTO_PATH;
  attachment.file.storage_type = esp_mail_file_storage_type_flash;

  SMTP_Message message;
  message.enable.chunking = true;
  message.sender.name = "ESP32-CAM";
  message.sender.email = SMTP_ACCOUNT;
  message.subject = EMAIL_SUBJECT;
  message.addRecipient(EMAIL_RECIPIENT_NAME, EMAIL_RECIPIENT_ADDR);
  message.html.content = "<h2>Anbei ein Foto von der ESP32-CAM.</h2>";
  message.html.charSet = "utf-8";
  message.html.transfer_encoding = Content_Transfer_Encoding::enc_qp;
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_normal;
  message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;
  message.addAttachment(attachment);

  Session_Config config;
  config.time.ntp_server = "pool.ntp.org,time.nist.gov";
  config.time.gmt_offset = 2;
  config.time.day_light_offset = 1;
  config.server.host_name = SMTP_SERVER;
  config.server.port = SMTP_PORT;
  config.login.email = SMTP_ACCOUNT;
  config.login.password = SMTP_PASSWORD;
  config.login.user_domain = "";

  if (!smtp.connect(&config))
    return;

  if (!MailClient.sendMail(&smtp, &message, true))
    Serial.println("Konnte E-Mail nicht senden: " + smtp.errorReason());
}

void smtpCallback(SMTP_Status status) {
  Serial.println(status.info());

  if (status.success()) {
    Serial.printf("Erfolgreich gesendet: %d\n", status.completedCount());
    Serial.printf("Nicht gesendet: %d\n", status.failedCount());

    for (size_t i = 0; i < smtp.sendingResult.size(); i++){
      const SMTP_Result result = smtp.sendingResult.getItem(i);
      const time_t timestamp = (time_t)result.timestamp;
      struct tm dateTime;

      localtime_r(&timestamp, &dateTime);

      Serial.printf("E-Mail Nr.: %d\n", i + 1);
      Serial.printf("Status: %s\n", result.completed ? "gesendet" : "fehler");
      Serial.printf("Zeitstempel: %d/%02d/%02d %02d:%02d:%02d\n",
        dateTime.tm_year + 1900, dateTime.tm_mon + 1, dateTime.tm_mday,
        dateTime.tm_hour, dateTime.tm_min, dateTime.tm_sec);
      Serial.printf("Empfänger: %s\n", result.recipients.c_str());
      Serial.printf("Betreff: %s\n", result.subject.c_str());
    }
    Serial.println("----------------\n");

   smtp.sendingResult.clear();
  }
}
