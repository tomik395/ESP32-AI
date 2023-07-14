extern StaticJsonDocument<1024> dataDoc;

void textToSpeech(String text) {
  Serial.print("Getting Text to Speech........ ");

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient httpClient;
  httpClient.begin(client, "https://texttospeech.googleapis.com/v1/text:synthesize?key=" + dataDoc["apiKeyGoogle"].as<String>());

  DynamicJsonDocument doc(1024);
  doc["input"]["text"] = text;
  doc["voice"]["languageCode"] = "en-US";
  doc["voice"]["name"] = "en-US-Wavenet-A";
  doc["audioConfig"]["audioEncoding"] = "LINEAR16";
  doc["audioConfig"]["sampleRateHertz"] = int(dataDoc["sampleRate"]);
  String requestBody;
  serializeJson(doc, requestBody);

  httpClient.addHeader("Content-Type", "application/json");
  int httpCode = httpClient.POST(requestBody);
  int contentLength = httpClient.getSize();

  // Save the JSON response to a file on the SD card in chunks
  File jsonResponseFile = SD.open(dataDoc["jsonResponseFilePath"].as<String>(), FILE_WRITE);

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
  File jsonResponseFileRead = SD.open(dataDoc["jsonResponseFilePath"].as<String>(), FILE_READ);
  jsonResponseFileRead.seek(audioContentPos + 17);

  //write base64 into sdcard
  File audioSamplesFile_base64 = SD.open(dataDoc["samplesGoogleFilePath_base64"].as<String>(), FILE_WRITE);
  while (jsonResponseFileRead.available()) {
    audioSamplesFile_base64.write(jsonResponseFileRead.read());  // Read one character from the file and write to the new file
  }
  jsonResponseFileRead.close();
  audioSamplesFile_base64.close();

  //Delete the JSON response file
  //SD.remove(jsonResponseFilePath);
}


//Not working
String speechToText(){
  //USAGE: This function only has support for sending audio content from memory only.
  //       In the future, an additional function with support for sending from a Google Storage URI will be added.
  Serial.print("Getting Speech to Text........ ");  
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient httpClient;
  httpClient.begin(client, "https://speech.googleapis.com/v1p1beta1/speech:recognize?key=" + dataDoc["apiKeyGoogle"].as<String>());

  File contentBase64 = SD.open(dataDoc["samplesRecordedFilePath_base64"].as<String>(), FILE_READ);
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
  String requestBody = "{\"config\":{\"encoding\":\"LINEAR16\",\"sampleRateHertz\":8000,\"languageCode\":\"en-US\"},\"audio\":{\"content\":\"";
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

  //Serial.println(base64Str);
  //requestBody.replace("\n", "");
  //requestBody.replace("\r", "");  
  //base64Str.replace("\"}}", "");
  //requestBody.replace("\\", "");

  //Serial.println(requestBody);

  httpClient.addHeader("Content-Type", "application/json");
  int httpCode = httpClient.POST(requestBody);

  String responseString = httpClient.getString(); // get the whole response at once
  DynamicJsonDocument respDoc(4096);
  deserializeJson(respDoc, responseString);
  //Serial.println(responseString);

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
  String requestBody = "grant_type=refresh_token&response_type=token&refresh_token=" + dataDoc["refreshTokenGoogle"].as<String>() + "&client_secret=" + dataDoc["clientSecretGoogle"].as<String>() + "&client_id="+dataDoc["clientIDGoogle"].as<String>();
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


/*
//not finished
void uploadToDocs(String token, String fileName){
  Serial.print("Storing on Cloud...");

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient httpClient;
  httpClient.begin(client, "https://docs.googleapis.com/v1/documents");
  //.   /o/testobjectname
  //httpClient.addHeader("Content-Type", "application/json");
  //httpClient.addHeader("Content-length", "20");
  httpClient.addHeader("Authorization", "Bearer " + String(token));  // Your OAuth 2.0 token

  DynamicJsonDocument doc(1024);
  doc["documentId"] = "LINEAR16";
  doc["title"] = fileName;
//  doc["body"]["content"]["startIndex"] = 
  //doc["body"]["content"]["endIndex"] = 
  //doc["body"]["content"]["paragraph"] = 
  String requestBody;
  serializeJson(doc, requestBody);
  
  int httpCode = httpClient.POST(requestBody);
  int contentLength = httpClient.getSize();

  // Save the JSON response to a file on the SD card in chunks
  File jsonResponseFile = SD.open(jsonResponseFilePath, FILE_WRITE);
  String responseString;

  const size_t chunkSize = 1024;
  uint8_t buffer[chunkSize];
  size_t bytesReceived;
  while (httpClient.connected()) {
    bytesReceived = httpClient.getStream().readBytesUntil('\0', buffer, chunkSize);
    if (bytesReceived == 0) {
      break;
    }
    jsonResponseFile.write(buffer, bytesReceived);
    // Append the received bytes to the response string
    responseString += String((char*)buffer);
  }

  Serial.print("Status Code ");
  Serial.println(httpCode);
  Serial.print("Response: ");
  Serial.println(responseString);

  jsonResponseFile.close();
  httpClient.end();
}


void uploadOnDrive(String token, String fileName){
  Serial.print("Storing on Cloud (Chunk)......  ");

 // Open the file on the SD card
  File audioFile = SD.open("/audio_samples.wav", FILE_READ);

  // Calculate the total file size
  size_t totalFileSize = audioFile.size();

  // Read the entire file into memory
  uint8_t fileData[totalFileSize];
  audioFile.read(fileData, totalFileSize);

  // Define a boundary for the multipart request
  String boundary = "foo_bar_baz";

  // Create the metadata for the file
  String metadata = "--" + boundary + "\r\n";
  metadata += "Content-Type: application/json; charset=UTF-8\r\n\r\n";
  metadata += "{ \"name\": \"" + fileName + "\", \"mimeType\": \"audio/wav\" }\r\n";
  metadata += "--" + boundary + "\r\n";
  metadata += "Content-Type: audio/wav\r\n\r\n";

  // Convert the metadata and file data to bytes
  uint8_t metadataBytes[metadata.length()];
  metadata.getBytes(metadataBytes, metadata.length());

  // Create the HTTP client and start the connection
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient httpClient;
  httpClient.begin(client, "https://www.googleapis.com/upload/drive/v3/files?uploadType=multipart");

  // Set the appropriate headers
  httpClient.addHeader("Content-Type", "multipart/related; boundary=" + boundary);
  httpClient.addHeader("Authorization", "Bearer " + String(token));

  // Create a Stream to hold the request body
  WiFiClient requestStream;
  //requestStream.write(metadataBytes, metadata.length());  // Write the metadata part
  //requestStream.write(fileData, totalFileSize);           // Write the file data part
  //requestStream.write("\r\n--" + boundary + "--\r\n");    // Write the end boundary

  // Send the POST request
  int httpCode = httpClient.sendRequest("POST", &requestStream, requestStream.available());

  // Close the file
  audioFile.close();

  // Handle the response
  if (httpCode > 0) {
      String response = httpClient.getString();
      Serial.println("Response: " + response);
  } else {
      Serial.println("Error: " + httpClient.errorToString(httpCode));
  }
  Serial.print("dome");
}*/