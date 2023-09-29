#ifndef recording_h
#define recording__h

#include "global.h"
#include "config.h"


void writeInt(File &file, uint32_t value);

void writeShort(File &file, uint16_t value);

void writeWavHeader(File &file, int sampleRate);

void createWavAudio(String filePath, int times);

#endif