#include <SPI.h>
#include <SdFat.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int JOYSTICK_Y = A1;
const int BUTTON_A = 2;
const int BUTTON_B = 3;
selecteditem = 0;
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
  lcd.begin(16, 2);

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



}




void loop() {
  int joystickValue = analogRead(JOYSTICK_Y);

  if (joystickValue < 300 && lastJoystick >= 300) {
    selectedItem--;

    if (selectedItem < 0) {
      selectedItem = MENU_COUNT - 1;
    }

  }


  if (joystickValue > 700 && lastJoystick <= 700) {
    selectedItem++;

    if (selectedItem >= MENU_COUNT) {
      selectedItem = 0;
    }

  }

  lastJoystick = joystickValue;

  if (digitalRead(BUTTON_A) == LOW) {
    delay(200);

    if (selectedItem == 0) {
    

    } else if (selectedItem == 1) {
      

    } else if (selectedItem == 2) {
      

    }

    
  }

  
  if (digitalRead(BUTTON_B) == LOW) {
    delay(200);
    

  }





}

void Playsong(const char* filename) {
audioFile = filename;
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


analogWrite(DAC_PIN, dacValue);

delayMicroseconds(62);
}
}

audioFile.close();
Serial.println("Playback finished.");



while(True){
  bool playlistSelected = false;
  curr playlist = 0;
  lcd.print("Choose a playlist: ");
  while(True){

    case selectedItem == 0:
      lcd.print("Banjo");
      while(True){
        if (digitalRead(BUTTON_A) == LOW) {
          delay(200);
          playlistSelected = true;
          break;
        if (selectedItem != 0) {
            lcd.clear();
            break;
          }
      }
      break;

      case selectedItem == 1:
      lcd.print("Persona");
      while(True){
        if (digitalRead(BUTTON_A) == LOW) {
          delay(200);
          playlistSelected = true;
          break;
        if (selectedItem != 1) {
            lcd.clear();
            break;
          }
      }
      break;

      case selectedItem == 2:
      lcd.print("Tuff");
      while(True){
        if (digitalRead(BUTTON_A) == LOW) {
          delay(200);
          playlistSelected = true;
          break;
        if (selectedItem != 2) {
            lcd.clear();
            break;
          }
      }
      break;

      case selectedItem == 3:
      lcd.print("Jazz");
      while(True){
        if (digitalRead(BUTTON_A) == LOW) {
          delay(200);
          playlistSelected = true;
          playSong(playlists[currPlaylist].songs[0]);
          break;
        if (selectedItem != 3) {
            lcd.clear();
            break;
          }
      }
      break;





    



  }
    
      
    }

  }
}


}






