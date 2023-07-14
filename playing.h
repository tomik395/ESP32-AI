extern StaticJsonDocument<1024> dataDoc;

// stop I2S audio by writing 0s
void stopAudio() {
  const int zero_samples = 1024;
  uint32_t zero_data = 0;
  for (int i = 0; i < zero_samples; i++) {
    size_t bytes_written = 0;
    i2s_write(I2S_NUM_0, &zero_data, sizeof(zero_data), &bytes_written, portMAX_DELAY);
  }
}

//play audio samples from the WAV file stored in SD card
void playAudioSamples(String wavFile) {
  Serial.print("Playing Audio................. ");
  File samplesFile = SD.open(wavFile, FILE_READ);

  const size_t bufferSize = 1024;
  int16_t buffer[bufferSize];
  size_t bytesRead;

  while ((bytesRead = samplesFile.read((uint8_t *)buffer, bufferSize * sizeof(int16_t))) > 0) {
    for (size_t i = 0; i < bytesRead / sizeof(int16_t); i++) {
      size_t bytes_written = 0;
      i2s_write(I2S_NUM_0, &buffer[i], sizeof(buffer[i]), &bytes_written, portMAX_DELAY);
    }
  }
  
  samplesFile.close();
  Serial.println("Done!");
  stopAudio();
}

