extern StaticJsonDocument<1024> dataDoc;
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>

String getAnswer(String question){
  Serial.print("Sending Open AI Call.......... ");

  // OpenAI API details
  const char *serverAddress = "api.openai.com";
  int port = 443;
  const char *apiPath = "/v1/chat/completions";

  // Initialize the Wi-Fi SSL client and HTTP client
  WiFiClientSecure client;
  HttpClient httpClient = HttpClient(client, serverAddress, port);
  client.setInsecure();

  // Prepare JSON payload
  String requestBody;
  DynamicJsonDocument doc(1024);
  doc["model"] = "gpt-3.5-turbo";
  JsonArray messages = doc.createNestedArray("messages");
  JsonObject message = messages.createNestedObject();
  message["role"] = "user";
  message["content"] = question;
  serializeJson(doc, requestBody);

  //start request call
  httpClient.beginRequest();
  httpClient.post(apiPath);

  // add headers
  httpClient.sendHeader("Authorization", "Bearer " + dataDoc["apiKeyOpenAI"].as<String>());
  httpClient.sendHeader("Content-Type", "application/json");
  httpClient.sendHeader("Content-Length", requestBody.length());

  httpClient.beginBody();
  httpClient.print(requestBody); // Write the JSON payload into string

  httpClient.endRequest(); //end request call

  // read the response status and body
  int statusCode = httpClient.responseStatusCode();
  String response = httpClient.responseBody();
  Serial.println(statusCode);

  // parse the response JSON
  DynamicJsonDocument responseDoc(2048);
  DeserializationError error = deserializeJson(responseDoc, response);

  // Extract the content field
  String content = responseDoc["choices"][0]["message"]["content"];

  Serial.print("Content: ");
  Serial.println(content);
  return content; //return the OpenAI response to question asked
}