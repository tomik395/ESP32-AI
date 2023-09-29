#ifndef Google_Wrapper_h
#define Google_Wrapper_h

#include "global.h"
#include "config.h"

void textToSpeech(String text);
String speechToText();
String getOAuthToken();
void uploadToStorage(String token, String fileName);

#endif