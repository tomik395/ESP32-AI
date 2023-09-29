
#include "global.h"
#include "config.h"

#include "recording.h"
#include "processing.h"
#include "playing.h"
#include "base64_wrapper.h"
#include "other.h"
#include "Google_Wrapper.h"

bool initSDCard() {
  Serial.print("Initializing SD card.......... ");
  if (!SD.begin(CHIPSELECTPIN)) {
    return false;
  }
  if (SD.exists(samplesRecordedFilePath)) {SD.remove(samplesRecordedFilePath);}
  if (SD.exists(samplesRecordedFilePath_base64)) {SD.remove(samplesRecordedFilePath_base64);}
  if (SD.exists(samplesGoogleFilePath)) {SD.remove(samplesGoogleFilePath);}
  if (SD.exists(samplesGoogleFilePath_base64)) {SD.remove(samplesGoogleFilePath_base64);}
  if (SD.exists(jsonCallFilePath)) {SD.remove(jsonCallFilePath);}
  if (SD.exists(jsonResponseFilePath)) {SD.remove(jsonResponseFilePath);}
  return true;
}

bool connectToWiFi() {
  Serial.print("Connecting to Wi-Fi........... ");

  WiFi.begin(SSID, PASSWORD);
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startTime > 20000) { // Timeout after 20 seconds
      return false;
    }
    delay(1000);
  }
  return true;
}

bool i2s_mic_install() {
  Serial.print("Installing I2S (Mic) ......... ");
  const i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE * 8, //after averaging and considering processor timing, 64200 I2S sample rate is aprx 8000hz
    .bits_per_sample = i2s_bits_per_sample_t(16),
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
    .intr_alloc_flags = 0,
    .dma_buf_count = 128,
    .dma_buf_len = BUFFERLEN,
    .use_apll = false
  };
  esp_err_t i2s_install_status = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);

  const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = -1,
    .data_in_num = I2S_SD
  };
  esp_err_t i2s_pin_status = i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_start(I2S_NUM_0); //moved here, it was outside before

  if (i2s_install_status == ESP_OK && i2s_pin_status == ESP_OK){
    Serial.println("Success!");
    return true;
  }
}

bool i2s_speaker_install() {
  Serial.print("Installing I2S (Speaker) ..... ");

  i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
      .sample_rate = SAMPLE_RATE,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
      .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // Change to ONLY_LEFT
      .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
      .intr_alloc_flags = 0,
      .dma_buf_count = 4,
      .dma_buf_len = 512,
      .use_apll = false
  };
  esp_err_t i2s_install_status = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);

  i2s_pin_config_t pin_config = {
      .bck_io_num = 11,//bclkPin,
      .ws_io_num = 12,//wclkPin,
      .data_out_num = 10,//dataPin,
      .data_in_num = I2S_PIN_NO_CHANGE};
  esp_err_t i2s_pin_status = i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_set_sample_rates(I2S_NUM_0, SAMPLE_RATE);

  if (i2s_install_status == ESP_OK && i2s_pin_status == ESP_OK){
    Serial.println("Success!");
    return true;
  }
}


// -------------------------------------------------------

bool start(){
  if (!initSDCard()) {
    Serial.println("Failed. Exiting...");
    return false;
  }
  Serial.println("Success!");

  if (!connectToWiFi()) {
    Serial.println("Failed. Exiting...");
    return false;
  }
  Serial.println("Success!");  
  return true;
}

