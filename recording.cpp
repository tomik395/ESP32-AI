#include "global.h"
#include "config.h"

// functions to write int, 4 bytes to file
void writeInt(File &file, uint32_t value) {
  file.write((value >> 0) & 0xFF);
  file.write((value >> 8) & 0xFF);
  file.write((value >> 16) & 0xFF);
  file.write((value >> 24) & 0xFF);
}
//functions to write short, 2 bytes to file
void writeShort(File &file, uint16_t value) {
  file.write((value >> 0) & 0xFF);
  file.write((value >> 8) & 0xFF);
}

//write WAV header to .wav file in sd card
void writeWavHeader(File &file, int sampleRate) {
  const int bitsPerSample = 16;
  const int channels = 1;
  uint32_t totalAudioLen = file.size() - 44; //wav headers are 44 length
  uint32_t byteRate = sampleRate * channels * bitsPerSample / 8;

  file.seek(0);
  // RIFF chunk descriptor
  file.write((uint8_t*)"RIFF", 4);
  writeInt(file, 36 + totalAudioLen);  //chunk size (36 + SubChunk2Size)
  file.write((uint8_t*)"WAVE", 4);

  // fmt sub-chunk (formatting chunk)
  file.write((uint8_t*)"fmt ", 4);
  writeInt(file, 16);  // Subchunk1Size (16 for PCM)
  writeShort(file, 1);  // AudioFormat (1 for PCM)
  writeShort(file, channels);  // NumChannels
  writeInt(file, sampleRate);  // SampleRate
  writeInt(file, byteRate);  // ByteRate
  writeShort(file, channels * bitsPerSample / 8);  // BlockAlign
  writeShort(file, bitsPerSample);  // BitsPerSample

  // data sub-chunk
  file.write((uint8_t*)"data", 4);
  writeInt(file, totalAudioLen);  // Subchunk2Size
}


void createWavAudio(String filePath, int times){
  //recording process runs in a loop, by an iteration amount of 'times'
  //increase 'times' or make function button based
  Serial.print("Recording..................... ");
  int16_t sBuffer[BUFFERLEN];
  Serial.println(filePath);
  File samplesFile = SD.open(filePath, FILE_WRITE);
  int counter = times;
  float startTime = millis();
  while (counter>0){
    // in buffer from I2S audio data
    size_t bytesIn = 0;
    esp_err_t result = i2s_read(I2S_NUM_0, &sBuffer, BUFFERLEN, &bytesIn, portMAX_DELAY);
  
    if (result == ESP_OK)
    {
      // Read I2S data buffer
      int16_t samples_read = bytesIn / 8;
      if (samples_read > 0) {
        int16_t mean = 0;
        for (int16_t i = 0; i < samples_read; ++i) {
          mean += (sBuffer[i]*15);
        }
        // Average the data reading (average of 2 samples)
        mean /= samples_read;
        samplesFile.write((uint8_t *)&mean, sizeof(mean)); //write into .wav sd card file
      }
    }
    counter = counter-1;
  }
  float time = millis() - startTime;
  int realSampleRate = (times/time)*1000;
  Serial.println(realSampleRate);
  writeWavHeader(samplesFile, realSampleRate);
  samplesFile.close();
  Serial.println("Done!");
}