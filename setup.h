#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <mbedtls/base64.h>
#include <driver/i2s.h>
#include <Wire.h>
#include <SD.h>
#include <FS.h>
#include <SPI.h>

#include "recording.h"
#include "processing.h"
#include "playing.h"
//#include "base64_wrapper.h"
#include "other.h"
#include "Google_Wrapper.h"



//const char *ssid = "MOTO997C";
//const char *password = "rapidradish386";
//const char *ssid = "MySpectrumWiFi08-2G";
//const char *password = "yellownest082";
////const char *ssid = "SpectrumSetup-6B";
//const char *password = "greenairplane683";

StaticJsonDocument<1024> dataDoc;

bool loadJSON(){
  Serial.print("Loading JSON Data............. ");
  dataDoc["chipSelectPin"] = 5;
  dataDoc["I2S_SD"] = A1;
  dataDoc["I2S_WS"] = A2;
  dataDoc["I2S_SCK"] = A3;
  dataDoc["buffersize"] = 1000;
  dataDoc["bufferLen"] = 16;
  dataDoc["sampleRate"] = 8000; //impliment
  dataDoc["samplesRecordedFilePath"] = "/samples_recorded.wav";
  dataDoc["samplesRecordedFilePath_base64"] = "/samples_recorded_base64.txt";
  dataDoc["samplesGoogleFilePath"] = "/samples_Google.wav";
  dataDoc["samplesGoogleFilePath_base64"] = "/samples_Google_base64.txt";
  dataDoc["testfile"] = "/testfile.txt";

  dataDoc["jsonCallFilePath"] = "/json_call.txt";
  dataDoc["jsonResponseFilePath"] = "/json_response.txt";
  
  dataDoc["ssid"] = " ";
  dataDoc["password"] = " ";
  dataDoc["apiKeyGoogle"] = " ";
  dataDoc["apiKeyOpenAI"] = " ";
  dataDoc["refreshTokenGoogle"] = " ";
  dataDoc["clientSecretGoogle"] = " ";
  dataDoc["clientIDGoogle"] = " ";

  
  return true;
}

bool initSDCard() {
  Serial.print("Initializing SD card.......... ");
  if (!SD.begin(int(dataDoc["chipSelectPin"]))) {
    return false;
  }
  if (SD.exists(dataDoc["samplesRecordedFilePath"].as<String>())) {SD.remove(dataDoc["samplesRecordedFilePath"].as<String>());}
  if (SD.exists(dataDoc["samplesRecordedFilePath_base64"].as<String>())) {SD.remove(dataDoc["samplesRecordedFilePath_base64"].as<String>());}
  if (SD.exists(dataDoc["samplesGoogleFilePath"].as<String>())) {SD.remove(dataDoc["samplesGoogleFilePath"].as<String>());}
  if (SD.exists(dataDoc["samplesGoogleFilePath_base64"].as<String>())) {SD.remove(dataDoc["samplesGoogleFilePath_base64"].as<String>());}
  if (SD.exists(dataDoc["jsonCallFilePath"].as<String>())) {SD.remove(dataDoc["jsonCallFilePath"].as<String>());}
  if (SD.exists(dataDoc["jsonResponseFilePath"].as<String>())) {SD.remove(dataDoc["jsonResponseFilePath"].as<String>());}
  return true;
}

bool connectToWiFi() {
  Serial.print("Connecting to Wi-Fi........... ");

  WiFi.begin(dataDoc["ssid"].as<String>(), dataDoc["password"].as<String>());
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
    .sample_rate = 64200, //after averaging and considering processor timing, 64200 I2S sample rate is aprx 8000hz
    .bits_per_sample = i2s_bits_per_sample_t(16),
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
    .intr_alloc_flags = 0,
    .dma_buf_count = 128,
    .dma_buf_len = int(dataDoc["bufferLen"]),
    .use_apll = false
  };
  esp_err_t i2s_install_status = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);

  const i2s_pin_config_t pin_config = {
    .bck_io_num = dataDoc["I2S_SCK"],
    .ws_io_num = dataDoc["I2S_WS"],
    .data_out_num = -1,
    .data_in_num = dataDoc["I2S_SD"]
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
      .sample_rate = 8000,
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
  i2s_set_sample_rates(I2S_NUM_0, 8000);

  if (i2s_install_status == ESP_OK && i2s_pin_status == ESP_OK){
    Serial.println("Success!");
    return true;
  }
}


// -------------------------------------------------------

bool start(){
  if (!loadJSON()) {
    Serial.println("Failed. Exiting...");
    return false;
  }
  Serial.println("Success!");

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

