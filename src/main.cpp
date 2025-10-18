#include <Arduino.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

// todo make system ask sdcard for number of tracks.

// ---------- Pin definitions ----------
#define BUTTON_PIN 2    // PB2 (pin 7)
#define LED_PIN 1       // PB1 (pin 6)
#define DFPLAYER_RX 0   // PB0 (pin 5) - DFPlayer RX
#define DFPLAYER_TX 3   // PB3 (pin 2) - DFPlayer TX
#define DFPLAYER_BUSY 4 // PB4 (pin 3) - DFPlayer BUSY

// ---------- Constants ----------
#define DFPLAYER_BAUD 9600
#define DEBOUNCE_MS 50
#define LONG_PRESS_MS 1500
#define VOLUME_STEP 5
#define VOLUME_TIMEOUT 3000
#define INTRO_TRACK 1
#define MAX_VOLUME 30

// ---------- DFPlayer Serial ----------
SoftwareSerial dfSerial(DFPLAYER_TX, DFPLAYER_RX); // TX, RX

// ---------- State Machine ----------
enum State
{
  STATE_INTRO,
  STATE_IDLE,
  STATE_PLAYING,
  STATE_VOLUME
};

State currentState = STATE_INTRO;

// ---------- Globals ----------
bool buttonPressed = false;
unsigned long buttonPressStart = 0;
unsigned long lastVolumeAction = 0;
unsigned long playStart = 0; // For playback timing
byte volume = 5;
const int eepromAddress = 1; // EEPROM location to store volume
bool seededRandom = false;

// ---------- DFPlayer Command Helper ----------
void sendCommand(uint8_t cmd, uint16_t param)
{
  uint8_t packet[10];
  uint16_t checksum = 0 - (0xFF + 0x06 + cmd + 0x00 + (param >> 8) + (param & 0xFF));

  packet[0] = 0x7E;
  packet[1] = 0xFF;
  packet[2] = 0x06;
  packet[3] = cmd;
  packet[4] = 0x00;
  packet[5] = (param >> 8);
  packet[6] = (param & 0xFF);
  packet[7] = (checksum >> 8);
  packet[8] = (checksum & 0xFF);
  packet[9] = 0xEF;

  for (uint8_t i = 0; i < 10; i++)
    dfSerial.write(packet[i]);
}

void setVolume(byte vol)
{
  volume = constrain(vol, 0, MAX_VOLUME);
  sendCommand(0x06, volume);
}

void rememberVolume(byte v)
{
  EEPROM.update(eepromAddress, v);
}

byte readSavedVolume()
{
  byte v = EEPROM.read(eepromAddress);
  if (v > MAX_VOLUME)
    v = MAX_VOLUME / 2; // default
  return v;
}

void playTrack(uint16_t track)
{
  sendCommand(0x03, track);
}

void playRandomClip()
{
  int track = random(1, 144); // 143 clips
  playTrack(track);
}

// ---------- Button Handling ----------
bool updateButton()
{
  bool pressed = (digitalRead(BUTTON_PIN) == LOW);
  unsigned long now = millis();

  static bool lastReading = HIGH;
  static unsigned long lastDebounce = 0;

  if (pressed != lastReading)
  {
    lastDebounce = now;
    lastReading = pressed;
  }
  if ((now - lastDebounce) < DEBOUNCE_MS)
    return false;

  // press detected
  if (pressed && !buttonPressed)
  {
    buttonPressed = true;
    buttonPressStart = now;
  }

  // release detected
  if (!pressed && buttonPressed)
  {
    buttonPressed = false;
    return true;
  }

  return false;
}

// ---------- FSM Update ----------
void fsmUpdate()
{
  unsigned long now = millis();

  switch (currentState)
  {
  case STATE_INTRO:
    analogWrite(LED_PIN, 255);
    // Transition automatically to idle once intro finishes
    if (playStart == 0)
      playStart = now;

    if ((now - playStart) > 500 && digitalRead(DFPLAYER_BUSY) == HIGH)
    {
      currentState = STATE_IDLE;
      playStart = 0;
    }
    break;

  case STATE_IDLE:
    analogWrite(LED_PIN, 0);

    // Enter volume mode on long press
    if (buttonPressed && (now - buttonPressStart >= LONG_PRESS_MS))
    {
      currentState = STATE_VOLUME;
      lastVolumeAction = now;
      return;
    }

    // Short press → play random clip
    if (updateButton())
    {
      // Seed random every time (or just once if using seededRandom flag)
      randomSeed(now); // <-- you could remove seededRandom logic if you want

      if (digitalRead(DFPLAYER_BUSY) == HIGH)
      {
        playRandomClip();
        currentState = STATE_PLAYING;
        playStart = now;
      }
    }
    break;

  case STATE_PLAYING:
    analogWrite(LED_PIN, 255);
    // wait until DFPlayer finishes
    if (digitalRead(DFPLAYER_BUSY) == HIGH && playStart != 0 && (now - playStart) > 500)
    {
      currentState = STATE_IDLE;
      playStart = 0;
    }
    break;

  case STATE_VOLUME:
    analogWrite(LED_PIN, map(volume, 0, MAX_VOLUME, 0, 255));

    // handle timeout back to idle
    if (now - lastVolumeAction > VOLUME_TIMEOUT)
    {
      rememberVolume(volume);
      currentState = STATE_IDLE;
      return;
    }

    if (updateButton())
    {
      volume += VOLUME_STEP;
      if (volume > MAX_VOLUME)
        volume = 0;
      setVolume(volume);
      lastVolumeAction = now;
    }
    break;

  default:
    currentState = STATE_IDLE;
    break;
  }
}

// ---------- Setup ----------
void setup()
{
  dfSerial.begin(DFPLAYER_BAUD);
  delay(1000);
  pinMode(DFPLAYER_BUSY, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  analogWrite(LED_PIN, 0);

  delay(500);

  volume = readSavedVolume();
  setVolume(volume);
  delay(200);

  playTrack(INTRO_TRACK);
  playStart = 0;
}

// ---------- Main Loop ----------
void loop()
{
  fsmUpdate();
}
