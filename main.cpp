#include <SPI.h>
#include "SdFat.h"

const int chippin = 10;
SdFs sd;
FsFile myFile;

void setup() {
Serial.begin(115200);
while (!Serial);

if (!sd.begin(chippin, SD_SCK_MHZ(16))) {
Serial.println("enter sd card");
return;
}


void loop() {

  
}
