#include <SPI.h>
#include <SdFat.h>

#define SD_CS_PIN 10
#define DAC_PIN A0
#define BUFFER_SIZE 512

SdFat sd;
FsFile audioFile;
uint8_t audioBuffer[BUFFER_SIZE];

Playlist playlists[] = {
    {"Banjo", banjo,5},
    {"Persona",persona,5},
    {"Tuff",tuff,5},
    {"Jazz",jazz,5},
}

const char* banjo[] = {
    "/songs/mumbo.wav",
    "/songs/peak.wav",
    "/songs/intro.wav",
    "/songs/cove.wav",
    "/songs/spiral.wav"
};

const char* persona[] = {
    "/songs/lastsurprise.wav",
    "/songs/burnmydread.wav",
    "/songs/trueself.wav",
    "/songs/mass.wav",
    "/songs/shadowworld.wav"
};

const char* tuff[] = {
    "/songs/verity.wav",
};

const char* jazz[] = {
    "/songs/rhumba.wav",
    "/songs/spain.wav",
    "/songs/nutville.wav",
    "/songs/.wav",
    "/songs/footprints.wav"
};







void setup() {
Serial.begin(115200);
while (!Serial);


analogWriteResolution(12);

if (!sd.begin(SD_CS_PIN, SD_SCK_MHZ(12))) {
Serial.println("SD initialization failed!");
return;
}

if (!audioFile.open("/songs/song1.wav", O_READ)) {
Serial.println("Failed to open song1.wav!");
return;
}


char buffer[4];
while (audioFile.available()) {
audioFile.read(buffer, 4);
if (buffer[0] == 'd' && buffer[1] == 'a' && buffer[2] == 't' && buffer[3] == 'a') {
audioFile.read(buffer, 4); 
break;
}
}


while (audioFile.available()) {
int bytesRead = audioFile.read(audioBuffer, BUFFER_SIZE);

for (int i = 0; i < bytesRead - 1; i += 2) {
uint8_t lowByte = audioBuffer[i];
uint8_t highByte = audioBuffer[i + 1];

int16_t sample16 = (int16_t)((highByte << 8) | lowByte);
uint16_t dacValue = (sample16 + 32768) >> 4;

// Output to DAC
analogWrite(DAC_PIN, dacValue);

// 16kHz sample delay
delayMicroseconds(62);
}
}

audioFile.close();
Serial.println("Playback finished.");
}

void loop() {

}
