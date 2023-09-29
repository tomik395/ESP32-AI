#ifndef base64_wrapper_h
#define base64_wrapper_h

#include "global.h"
#include "config.h"

void base64_decode_sd(Stream &in, File &out);
void base64_encode_sd(Stream &in, File &out);
void WavToBase64inSDcard(String readFilePath, String writeFilePath);
void Base64ToWavinSDcard(String readFilePath, String writeFilePath);

#endif