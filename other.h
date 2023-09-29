#ifndef other_h
#define other_h

#include "global.h"
#include "config.h"

void printSDFileData(String filename) {
  Serial.print("Data from SD file path: ");
  Serial.println(filename);
  File myFile = SD.open(filename);
  if (!myFile) {
    Serial.println("Error opening file");
    return;
  }

  while (myFile.available()) { Serial.write(myFile.read()); }
  myFile.close();
}

#endif