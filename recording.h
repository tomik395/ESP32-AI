extern StaticJsonDocument<1024> dataDoc;
#include "base64_wrapper.h"

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


void recordSomeSamples(){
  //recording process runs in a loop, by an iteration amount of 'times'
  //increase 'times' or make function button based
  Serial.print("Recording..................... ");
  int16_t sBuffer[int(dataDoc["bufferLen"])];
  File samplesFile = SD.open(dataDoc["samplesRecordedFilePath"].as<String>(), FILE_WRITE);
  int times = 20000;
  float startTime = millis();
  while (times>0){
  
    // in buffer from I2S audio data
    size_t bytesIn = 0;
    esp_err_t result = i2s_read(I2S_NUM_0, &sBuffer, int(dataDoc["bufferLen"]), &bytesIn, portMAX_DELAY);
  
    if (result == ESP_OK)
    {
      // Read I2S data buffer
      int16_t samples_read = bytesIn / 8;
      if (samples_read > 0) {
        int16_t mean = 0;
        for (int16_t i = 0; i < samples_read; ++i) {
          mean += (sBuffer[i]);
        }
        // Average the data reading (average of 2 samples)
        mean /= samples_read;
        samplesFile.write((uint8_t *)&mean, sizeof(mean)); //write into .wav sd card file
      }
    }
    times = times-1;
  }
  float time = millis() - startTime;
  writeWavHeader(samplesFile, 8000);
  samplesFile.close();
  Serial.println("Done!");

  File samplesFile_Read = SD.open(dataDoc["samplesRecordedFilePath"].as<String>(), FILE_READ);
  int fileSize = samplesFile_Read.size();
  
  Serial.println("RECORDED STATISTICS:");
  Serial.printf("File Size:       %d Bytes\n", fileSize);
  Serial.printf("Fime Elapsed:    %f Seconds\n", time/1000);
  Serial.printf("Recorded Amount: %d Samples\n", fileSize/2);
  Serial.printf("Sampling Rate:   %f kHz\n", (fileSize/2)/time);
  samplesFile_Read.close();
}

