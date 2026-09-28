#include <SPI.h>
#include <SdFat.h>
#include <LiquidCrystal.h>

#pragma region Pin Definitions

LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

const uint8_t sticky = A1;
const uint8_t joystickButton = 8;
const uint8_t bbutton = 9;
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

enum {
  JOYSTICK_NONE,
  JOYSTICK_A_SINGLE,
  JOYSTICK_A_DOUBLE
};

uint8_t readJoystickAction(uint8_t pin) {
  static bool buttonWasDown = false;
  static uint32_t firstPressMillis = 0;
  static uint32_t lastReleaseMillis = 0;
  static bool waitingForSecondClick = false;
  static bool suppressNextRelease = false;

  uint32_t now = millis();
  bool buttonDown = (digitalRead(pin) == LOW);

  if (buttonDown && !buttonWasDown) {
    buttonWasDown = true;
    if (waitingForSecondClick && now - lastReleaseMillis <= 300) {
      waitingForSecondClick = false;
      suppressNextRelease = true;
      return JOYSTICK_A_DOUBLE;
    }
    firstPressMillis = now;
  }

  if (!buttonDown && buttonWasDown) {
    buttonWasDown = false;
    if (suppressNextRelease) {
      suppressNextRelease = false;
    } else if ((now - firstPressMillis) <= 250) {
      waitingForSecondClick = true;
      lastReleaseMillis = now;
    }
  }

  if (waitingForSecondClick && now - lastReleaseMillis > 300) {
    waitingForSecondClick = false;
    return JOYSTICK_A_SINGLE;
  }

  return JOYSTICK_NONE;
}

bool readButtonBPress(uint8_t pin) {
  static bool buttonWasDown = false;
  static uint32_t lastStateChangeMillis = 0;

  bool buttonDown = (digitalRead(pin) == LOW);
  if (buttonDown == buttonWasDown || millis() - lastStateChangeMillis < 30) {
    return false;
  }

  buttonWasDown = buttonDown;
  lastStateChangeMillis = millis();
  return buttonDown;
}

void showPlaylistItem(const char* title, uint8_t index, uint8_t count) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(title);
  lcd.setCursor(0, 1);
  lcd.print(index + 1);
  lcd.print("/");
  lcd.print(count);
  lcd.print(" D=sel S=rnd");
}

void showSongItem(const char* title, uint8_t index, uint8_t count) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(title);
  lcd.setCursor(0, 1);
  lcd.print(index + 1);
  lcd.print("/");
  lcd.print(count);
  lcd.print(" D=play S=rnd");
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

  uint8_t header[44];
  if (audioFile.read(header, sizeof(header)) != sizeof(header)) {
    audioFile.close();
    return;
  }

  lcd.clear();
  lcd.print("Playing...");
  lcd.setCursor(0, 1);
  lcd.print(filename + 7);
  Serial.print("Playing: ");
  Serial.println(filename);
  uint32_t lastStatusMillis = millis();

  while (audioFile.available()) {
    int bytesRead = audioFile.read(audioBuffer, BUFFER_SIZE);
    for (int i = 0; i + 1 < bytesRead; i += 2) {
      int16_t sample = (int16_t)((audioBuffer[i + 1] << 8) | audioBuffer[i]);
      int32_t amplifiedSample = ((int32_t)sample * 3) / 2;
      if (amplifiedSample > 32767) amplifiedSample = 32767;
      if (amplifiedSample < -32768) amplifiedSample = -32768;
      uint16_t dacValue = (uint16_t)((amplifiedSample + 32768) >> 4);
      analogWrite(DAC_PIN, dacValue);
      delayMicroseconds(62);
    }

    if (millis() - lastStatusMillis >= 5000) {
      Serial.print("Playing: ");
      Serial.println(filename);
      lastStatusMillis = millis();
    }
  }

  audioFile.close();
  analogWrite(DAC_PIN, 2048);
}

void chooserandomsong() {
  uint8_t randomPlaylistIndex = random(PLAYLIST_COUNT);
  Playlist& playlist = playlists[randomPlaylistIndex];
  uint8_t randomSongIndex = random(playlist.songCount);
  playSong(playlist.songs[randomSongIndex]);
}

void chooseSong(uint8_t playlistIndex) {
  Playlist& playlist = playlists[playlistIndex];
  selectedItem = 0;
  showSongItem(playlist.songs[selectedItem] + 7, selectedItem, playlist.songCount);

  while (true) {
    if (moveSelection(playlist.songCount)) {
      showSongItem(playlist.songs[selectedItem] + 7, selectedItem, playlist.songCount);
    }

    uint8_t action = readJoystickAction(joystickButton);

    if (action == JOYSTICK_A_DOUBLE) {
      playSong(playlist.songs[selectedItem]);
      showSongItem(playlist.songs[selectedItem] + 7, selectedItem, playlist.songCount);
    }

    if (action == JOYSTICK_A_SINGLE) {
      chooserandomsong();
      showSongItem(playlist.songs[selectedItem] + 7, selectedItem, playlist.songCount);
    }

    if (readButtonBPress(bbutton)) {
      return;
    }
  }
}

void setup() {
  Serial.begin(9600);
  delay(1000);
  analogWriteResolution(12);
  bool sdInitialized = sd.begin(SD_CS_PIN, SD_SCK_MHZ(4));

  if (!sdInitialized) {
    lcd.begin(16, 2);
    lcd.clear();
    lcd.print("SD init failed");
    Serial.println("SD initialization failed");
    while (true) {
      delay(1000);
    }
  }

  pinMode(joystickButton, INPUT_PULLUP);
  pinMode(bbutton, INPUT_PULLUP);
  lcd.begin(16, 2);
  lcd.print("Starting...");
  delay(1000);
}

void loop() {
  showPlaylistItem(playlists[selectedItem].name, selectedItem, PLAYLIST_COUNT);

  while (true) {
    if (moveSelection(PLAYLIST_COUNT)) {
      showPlaylistItem(playlists[selectedItem].name, selectedItem, PLAYLIST_COUNT);
    }

    uint8_t action = readJoystickAction(joystickButton);

    if (action == JOYSTICK_A_DOUBLE) {
      uint8_t playlistIndex = selectedItem;
      chooseSong(playlistIndex);
      selectedItem = playlistIndex;
      break;
    }

    if (action == JOYSTICK_A_SINGLE) {
      chooserandomsong();
      break;
    }

    readButtonBPress(bbutton);
  }
}