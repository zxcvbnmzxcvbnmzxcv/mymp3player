#include <SPI.h>
#include <SdFat.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#pragma region Pin Definitions



LiquidCrystal_I2C lcd(0x27, 16, 2);

const uint8_t sticky = A1;
const uint8_t abutton = 2;
const uint8_t bbutton = 3;
const uint8_t SD_CS_PIN = 10;
const uint8_t DAC_PIN = A0;
const uint16_t BUFFER_SIZE = 512;

#pragma endregion

#pragma region Playlists
const char* banjoSongs[] = {
  "/songs/banjo/mumbo.wav",
  "/songs/banjo/peak.wav",
  "/songs/banjo/intro.wav",
  "/songs/banjo/cove.wav",
  "/songs/banjo/spiral.wav"
};

const char* personaSongs[] = {
  "/songs/persona/lastsurprise.wav",
  "/songs/persona/burnmydread.wav",
  "/songs/persona/trueself.wav",
  "/songs/persona/mass.wav",
  "/songs/persona/shadowworld.wav"
};

const char* tuffSongs[] = {
  "/songs/tuff/verity.wav",
  "/songs/tuff/rapture.wav"
};

const char* jazzSongs[] = {
  "/songs/jazz/rhumba.wav",
  "/songs/jazz/spain.wav",
  "/songs/jazz/nutville.wav",
  "/songs/jazz/footprints.wav"
};

#pragma endregion



struct Playlist {
  const char* name;
  const char** songs;
  uint8_t songCount;
};

Playlist playlists[] = {
  {"Banjo", banjoSongs, sizeof(banjoSongs) / sizeof(banjoSongs[0])},
  {"Persona", personaSongs, sizeof(personaSongs) / sizeof(personaSongs[0])},
  {"Tuff", tuffSongs, sizeof(tuffSongs) / sizeof(tuffSongs[0])},
  {"Jazz", jazzSongs, sizeof(jazzSongs) / sizeof(jazzSongs[0])}
};


const uint8_t PLAYLIST_COUNT = sizeof(playlists) / sizeof(playlists[0]);

SdFat sd;
uint8_t audioBuffer[BUFFER_SIZE];
int lastJoystick = 512;
uint8_t selectedItem = 0;




bool pressed(uint8_t pin) {
  if (digitalRead(pin) != LOW) {
    return false;
  }

  delay(25);
  while (digitalRead(pin) == LOW) {
    delay(5);
  }
  return true;
}

void showItem(const char* title, uint8_t index, uint8_t count) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(title);
  lcd.setCursor(0, 1);
  lcd.print(index + 1);
  lcd.print("/");
  lcd.print(count);
  lcd.print(" A=select");
}

bool moveSelection(uint8_t itemCount) {
  int joystickValue = analogRead(sticky);
  bool moved = false;

  if (joystickValue < 300 && lastJoystick >= 300) {
    selectedItem = selectedItem == 0 ? itemCount - 1 : selectedItem - 1;
    moved = true;
  } else if (joystickValue > 700 && lastJoystick <= 700) {
    selectedItem = (selectedItem + 1) % itemCount;
    moved = true;
  }

  lastJoystick = joystickValue;
  return moved;
}

void playSong(const char* filename) {
  FsFile audioFile;
  if (!audioFile.open(filename, O_RDONLY)) {
    lcd.clear();
    lcd.print("Open failed");
    Serial.print("Could not open ");
    Serial.println(filename);
    delay(1200);
    return;
  }

  // Skip the WAV header. The supported files are 16-bit PCM WAV files.
  uint8_t header[44];
  if (audioFile.read(header, sizeof(header)) != sizeof(header)) {
    audioFile.close();
    return;
  }

  lcd.clear();
  lcd.print("Playing...");
  lcd.setCursor(0, 1);
  lcd.print(filename + 7);

  while (audioFile.available()) {
    int bytesRead = audioFile.read(audioBuffer, BUFFER_SIZE);
    for (int i = 0; i + 1 < bytesRead; i += 2) {
      int16_t sample = (int16_t)((audioBuffer[i + 1] << 8) | audioBuffer[i]);
      uint16_t dacValue = (uint16_t)(((int32_t)sample + 32768) >> 4);
      analogWrite(DAC_PIN, dacValue);
      delayMicroseconds(62); // approximately 16 kHz
    }
  }

  audioFile.close();
  analogWrite(DAC_PIN, 2048);
}

void chooseSong(uint8_t playlistIndex) {
  Playlist& playlist = playlists[playlistIndex];
  selectedItem = 0;
  showItem(playlist.songs[selectedItem] + 7, selectedItem, playlist.songCount);

  while (true) {
    if (moveSelection(playlist.songCount)) {
      showItem(playlist.songs[selectedItem] + 7, selectedItem, playlist.songCount);
    }

    if (pressed(abutton)) {
      playSong(playlist.songs[selectedItem]);
      showItem(playlist.songs[selectedItem] + 7, selectedItem, playlist.songCount);
    }

    if (pressed(bbutton)) {
      return;
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(abutton, INPUT_PULLUP);
  pinMode(bbutton, INPUT_PULLUP);
  analogWriteResolution(12);

  lcd.init();
  lcd.backlight();
  lcd.print("Starting...");

  if (!sd.begin(SD_CS_PIN, SD_SCK_MHZ(12))) {
    lcd.clear();
    lcd.print("SD init failed");
    Serial.println("SD initialization failed");
    while (true) {
      delay(1000);
    }
  }
}

void loop() {
  showItem(playlists[selectedItem].name, selectedItem, PLAYLIST_COUNT);

  while (true) {
    if (moveSelection(PLAYLIST_COUNT)) {
      showItem(playlists[selectedItem].name, selectedItem, PLAYLIST_COUNT);
    }

    if (pressed(abutton)) {
      uint8_t playlistIndex = selectedItem;
      chooseSong(playlistIndex);
      selectedItem = playlistIndex;
      break;
    }
  }
}
