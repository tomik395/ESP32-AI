#include "setup.h"

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("---------- Starting ------------ ");
  if(!start()){
    return;
  }
  i2s_mic_install(); //Install I2S microphone driver
  createWavAudio(samplesRecordedFilePath, SAMPLE_RATE*SECONDS);
  WavToBase64inSDcard(samplesRecordedFilePath, samplesRecordedFilePath_base64);
  String transcription = speechToText(); //transcribe audio to text
  String answer = getAnswer(transcription);
  i2s_driver_uninstall(I2S_NUM_0);
  i2s_speaker_install(); //Install I2S Speaker Driver
  textToSpeech(answer); //play the answer
  Base64ToWavinSDcard(samplesGoogleFilePath_base64, samplesGoogleFilePath);
  playAudioSamples(samplesGoogleFilePath);
}

void loop() {
  // not used
}
