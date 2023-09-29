#include "Google_Wrapper.h"
#include "global.h"
#include "config.h"

void textToSpeech(String text) {
  Serial.print("Getting Text to Speech........ ");

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient httpClient;
  httpClient.begin(client, "https://texttospeech.googleapis.com/v1/text:synthesize?key=" + String(APIKEYGOOGLE));

  DynamicJsonDocument doc(1024);
  doc["input"]["text"] = text;
  doc["voice"]["languageCode"] = "en-US";
  doc["voice"]["name"] = "en-US-Wavenet-A";
  doc["audioConfig"]["audioEncoding"] = "LINEAR16";
  doc["audioConfig"]["sampleRateHertz"] = SAMPLE_RATE;
  String requestBody;
  serializeJson(doc, requestBody);

  httpClient.addHeader("Content-Type", "application/json");
  int httpCode = httpClient.POST(requestBody);
  int contentLength = httpClient.getSize();

  // Save the JSON response to a file on the SD card in chunks
  File jsonResponseFile = SD.open(jsonResponseFilePath, FILE_WRITE);

  const size_t chunkSize = 1024;
  uint8_t buffer[chunkSize];
  size_t bytesReceived;
  while (httpClient.connected()) {
    bytesReceived = httpClient.getStream().readBytesUntil('\0', buffer, chunkSize);
    if (bytesReceived == 0) {
      break;
    }
    jsonResponseFile.write(buffer, bytesReceived);
  }

  httpClient.end();

  Serial.print("Status Code ");
  Serial.println(httpCode);
      
  // Find the position of the audioContent field in the JSON response
  int audioContentPos = jsonResponseFile.find("\"audioContent\":");
  if (audioContentPos == -1) {
    Serial.println("Failed to find audioContent field in the JSON response");
    jsonResponseFile.close();
    return;
  }

  // Close and reopen as read, skip to audio content...
  jsonResponseFile.close();
  File jsonResponseFileRead = SD.open(jsonResponseFilePath, FILE_READ);
  jsonResponseFileRead.seek(audioContentPos + 17);

  //write base64 into sdcard
  File audioSamplesFile_base64 = SD.open(samplesGoogleFilePath_base64, FILE_WRITE);
  while (jsonResponseFileRead.available()) {
    audioSamplesFile_base64.write(jsonResponseFileRead.read());  // Read one character from the file and write to the new file
  }
  jsonResponseFileRead.close();
  audioSamplesFile_base64.close();

  //Delete the JSON response file
  //SD.remove(jsonResponseFilePath);
}

String speechToText(){
  //USAGE: This function only has support for sending audio content from memory only.
  //       In the future, an additional function with support for sending from a Google Storage URI will be added.
  Serial.print("Getting Speech to Text........ ");  
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient httpClient;
  httpClient.begin(client, "https://speech.googleapis.com/v1p1beta1/speech:recognize?key=" + String(APIKEYGOOGLE));

  File contentBase64 = SD.open(samplesRecordedFilePath_base64, FILE_READ);
  Serial.println("PAYLOAD STATISTICS:");
  Serial.printf("File Size:       %d Bytes\n", contentBase64.size());

  int docSize = contentBase64.size() + 2048;
  int availSpace = esp_get_free_heap_size();
  Serial.print("Free heap size at start: ");
  Serial.println(availSpace);

  if (docSize > availSpace){
    Serial.println("Payload requires more than available memory. Handle this.");
  }
  
  //construct parameter string for API call
  String requestBody = "{\"config\":{\"encoding\":\"LINEAR16\",\"sampleRateHertz\":16000,\"languageCode\":\"en-US\"},\"audio\":{\"content\":\"";
  const int BUFFER_SIZE = 1;
  char buffer[BUFFER_SIZE];
  String base64Str = "";
  //read bytes of Base64 encoded recorded audio content from SD card
  while (contentBase64.available()) {
    int bytesRead = contentBase64.readBytes(buffer, BUFFER_SIZE);
    base64Str += String(buffer).substring(0, bytesRead);
  }
  requestBody += base64Str;
  requestBody += "\"}}";

  httpClient.addHeader("Content-Type", "application/json");
  httpClient.addHeader("Content-Length", String(requestBody.length()));
  int httpCode = httpClient.POST(requestBody);
  Serial.println(requestBody);
  String responseString = httpClient.getString(); // get the whole response at once
  DynamicJsonDocument respDoc(4096);
  deserializeJson(respDoc, responseString);
  Serial.println(responseString);

  // Extract the content field
  String content = respDoc["response"]["results"]["alternatives"]["transcript"];
  int confidence = respDoc["response"]["results"]["alternatives"]["confidence"];

  httpClient.end();
  contentBase64.close();

  Serial.print("Status Code ");
  Serial.println(httpCode);

  Serial.print("Transcripted Audio: ");
  Serial.println(content);
  Serial.print("Transcription Confidence: ");
  Serial.println(confidence);

  return content;
}

//Use Google refresh token to obtain your Oauth 2.0 token
String getOAuthToken(){
  Serial.print("Requesting Auth 2.0 Token.....");

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient httpClient;

  httpClient.begin("https://oauth2.googleapis.com/token");
  httpClient.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String requestBody = "grant_type=refresh_token&response_type=token&refresh_token=" + String(REFRESHTOKENGOOGLE) + "&client_secret=" + String(CLIENTSECRETGOOGLE) + "&client_id="+ String(CLIENTIDGOOGLE);
  int httpCode = httpClient.POST(requestBody);

  String responseString = httpClient.getString(); // get the whole response at once
  DynamicJsonDocument respDoc(1024);
  deserializeJson(respDoc, responseString); //deserialize into JSON

  String accessToken = respDoc["access_token"]; //obtain oauth2.0 token
  if (httpCode == 200 && accessToken.length() > 30){  Serial.println(" Success!"); }

  httpClient.end();
  return accessToken; //return token
}

//not finished, only required if audio we are sending to speech-to-text is over 10mb or 1 minute
void uploadToStorage(String token, String fileName){
  Serial.print("Storing on Cloud.............. ");

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient httpClient;
  httpClient.begin(client, "https://storage.googleapis.com/upload/storage/v1/b/hara-esp32/o");
  httpClient.addHeader("Authorization", "Bearer " + String(token));  // Your OAuth 2.0 token

  DynamicJsonDocument doc(1024);
  doc["documentId"] = "LINEAR16";
  doc["title"] = fileName;
  String requestBody;
  serializeJson(doc, requestBody);
  
  int httpCode = httpClient.POST(requestBody);

  String responseString = httpClient.getString(); // get the whole response at once
  DynamicJsonDocument respDoc(1024);
  deserializeJson(respDoc, responseString);

  Serial.print("Status Code ");
  Serial.println(httpCode);
  Serial.print("Response: ");
  Serial.println(responseString);

  httpClient.end();
}

// ... other function implementations