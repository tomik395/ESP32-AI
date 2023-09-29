#include "base64_wrapper.h"
#include "global.h"
#include "config.h"

void base64_decode_sd(Stream &in, File &out) {
  //stream 'in' is base64 decoded and written to file 'out'
  unsigned char buff_in[4];
  unsigned char buff_out[3];
  int counter = 0;

  while (in.available()) {
    size_t bytesRead = in.readBytes((char *)buff_in, 4);
    if (bytesRead == 0) { break; }

    for (int i = 0; i < 4; i++) {
      if (buff_in[i] >= 'A' && buff_in[i] <= 'Z') buff_in[i] = buff_in[i] - 'A';
      else if (buff_in[i] >= 'a' && buff_in[i] <= 'z') buff_in[i] = buff_in[i] - 'a' + 26;
      else if (buff_in[i] >= '0' && buff_in[i] <= '9') buff_in[i] = buff_in[i] - '0' + 52;
      else if (buff_in[i] == '+') buff_in[i] = 62;
      else if (buff_in[i] == '/') buff_in[i] = 63;
      else buff_in[i] = 0;
    }

    buff_out[0] = (buff_in[0] << 2) | (buff_in[1] >> 4);
    buff_out[1] = ((buff_in[1] & 0x0f) << 4) | (buff_in[2] >> 2);
    buff_out[2] = ((buff_in[2] & 0x03) << 6) | buff_in[3];

    size_t bytesToWrite = (buff_in[3] != '=' ? 3 : (buff_in[2] != '=' ? 2 : 1));
    size_t bytesWritten = out.write(buff_out, bytesToWrite);
    if (bytesWritten != bytesToWrite) {
      Serial.print("Write error at block ");
      Serial.println(counter);
      break;
    }
    counter++;
  }
}

//stream 'in' is base64 encoded and written to file 'out'
void base64_encode_sd(Stream &in, File &out) {
  size_t decodedLength;
  const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  unsigned char buff_in[3];
  unsigned char buff_out[4];
  int counter = 0;
    while (in.available()) {
      size_t bytesRead = in.readBytes((char *)buff_in, 3);
      if (bytesRead == 0) { break; }

      buff_out[0] = base64_chars[buff_in[0] >> 2];
      buff_out[1] = base64_chars[((buff_in[0] & 0x03) << 4) | ((buff_in[1] & 0xf0) >> 4)];
      buff_out[2] = (bytesRead > 1 ? base64_chars[((buff_in[1] & 0x0f) << 2) | ((buff_in[2] & 0xc0) >> 6)] : '=');
      buff_out[3] = (bytesRead > 2 ? base64_chars[buff_in[2] & 0x3f] : '=');

      size_t bytesWritten = out.write(buff_out, 4);
      if (bytesWritten != 4) {
          Serial.print("Write error at block ");
          Serial.println(counter);
          break;
      }
      counter++;
  }
}

// parent function for base64 encoding
void WavToBase64inSDcard(String readFilePath, String writeFilePath){
  Serial.print("Base64 Encoding Audio......... ");
  File readFile = SD.open(readFilePath, FILE_READ);
  File writeFile = SD.open(writeFilePath, FILE_WRITE);

  base64_encode_sd(readFile, writeFile);
  readFile.close();
  writeFile.close();
  Serial.println("Success!");
}

// parent function for base64 decoding
void Base64ToWavinSDcard(String readFilePath, String writeFilePath){
  Serial.print("Base64 Decoding Audio......... ");
  File readFile = SD.open(readFilePath, FILE_READ);
  File writeFile = SD.open(writeFilePath, FILE_WRITE);

  base64_decode_sd(readFile, writeFile);
  readFile.close();
  writeFile.close();
  Serial.println("Success!");
}