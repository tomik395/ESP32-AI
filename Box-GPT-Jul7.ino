#include "setup.h"
extern StaticJsonDocument<1024> dataDoc;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("---------- Starting ------------ ");
  if(!start()){
    return;
  }
  i2s_mic_install(); //Install I2S microphone driver
  recordSomeSamples(); //Record some samples, as a question
  WavToBase64inSDcard(dataDoc["samplesRecordedFilePath"].as<String>(), dataDoc["samplesRecordedFilePath_base64"].as<String>());
  String transcription = speechToText(); //transcribe audio to text
  //printSDFileData(dataDoc["samplesRecordedFilePath_base64"].as<String>());
  String answer = getAnswer(transcription);
  i2s_driver_uninstall(I2S_NUM_0);
  i2s_speaker_install(); //Install I2S Speaker Driver
  textToSpeech(answer); //play the answer
  Base64ToWavinSDcard(dataDoc["samplesGoogleFilePath_base64"].as<String>(), dataDoc["samplesGoogleFilePath"].as<String>());
  playAudioSamples(dataDoc["samplesGoogleFilePath"].as<String>());
}

void loop() {
  // not used
}
