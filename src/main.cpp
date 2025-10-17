#include <Arduino.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

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
#define INTRO_TRACK 26

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
byte volume = 5;
int currentBrightness = 0;
int targetBrightness = 0;
const int eepromAddress = 1;  // EEPROM location to store volume


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

// ---------- SET VOLUME  ----------
void setVolume(byte vol)
{
  volume = constrain(vol, 0, 30);
  sendCommand(0x06, volume);
}

// ---------- SAVE VOLUME VALUE ----------
void rememberVolume(byte currentVolume) {
 // if (EEPROM.read(eepromAddress) != currentVolume) {
    EEPROM.update(eepromAddress, currentVolume);
 // }
}

// ---------- READ SAVED VOLUME VALUE ----------
byte readSavedVolume() {
    byte v = EEPROM.read(eepromAddress);
  if (v > 30) v = 15;   // sanity check / default
  return v;
}

void playTrack(uint16_t track)
{
  sendCommand(0x03, track);
}

void playRandomClip()
{
  int track = random(1, 144); //  143 clips
  playTrack(track);
}

// ---------- Volume State Timeout ----------
void handleVolumeTimeout()
{
  if (currentState == STATE_VOLUME)
  {
    if (millis() - lastVolumeAction > VOLUME_TIMEOUT) // todo watch this variable
     {rememberVolume(volume);
 
    currentState = STATE_IDLE;
     }
  }
}


// ---------- PLAYBACK STATE MONITOR ----------
void updatePlaybackState()
{
  static unsigned long playStart = 0; // when playback (intro or clip) began

  // Handle playback (intro + normal)
  if (currentState == STATE_PLAYING || currentState == STATE_INTRO)
  {
    // Record the moment playback started
    if (playStart == 0)
      playStart = millis();

    // Give DFPlayer time to assert BUSY low (about 500 ms)
    if (millis() - playStart < 500)
      return;

    // After settle period, check if playback finished (BUSY goes high again)
    if (digitalRead(DFPLAYER_BUSY) == HIGH)
    {
      currentState = STATE_IDLE;
      analogWrite(LED_PIN, 0); // ensure LED off at end of clip
      playStart = 0;           // reset timer for next playback
    }
  }
  else
  {
    playStart = 0; // reset timer when not in a playing/intro state
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

  // Give DFPlayer extra time to boot and mount SD
  delay(500);

  // Set volume and play intro
  volume = readSavedVolume();
  setVolume(volume);
  delay(200);

  playTrack(1);
  delay(200);
}
// ---------- Input Handling ----------
void updateButton()
{
  bool pressed = (digitalRead(BUTTON_PIN) == LOW);
  unsigned long now = millis();

  // --- debounce ---
  static bool lastReading = HIGH;
  static unsigned long lastDebounce = 0;
  if (pressed != lastReading)
  {
    lastDebounce = now;
    lastReading = pressed;
  }
  if ((now - lastDebounce) < DEBOUNCE_MS)
    return; // still bouncing

  // --- press detected ---
  if (pressed && !buttonPressed)
  {
    buttonPressed = true;
    buttonPressStart = now;
  }

  // --- held long enough → enter VOLUME_CHANGE ---
  if (buttonPressed && (currentState != STATE_VOLUME) &&
      (now - buttonPressStart >= LONG_PRESS_MS))
  {
    currentState = STATE_VOLUME;
    lastVolumeAction = now;
    return;
  }

  // --- release detected ---
  if (!pressed && buttonPressed)
  {
    buttonPressed = false;

    switch (currentState)
    {
    case STATE_IDLE:
      // short press → play clip *only if DFPlayer not busy*
      if (digitalRead(DFPLAYER_BUSY) == HIGH)
      {
        playRandomClip();

        currentState = STATE_PLAYING;
      }

      break;

    case STATE_VOLUME:
      volume += VOLUME_STEP;
      if (volume > 30)
        volume = 0;
      setVolume(volume);
      lastVolumeAction = now;
      break;

    default:
      break;
    }
  }
}

// ---------- LED Control ----------
void updateLED()
{
  switch (currentState)
  {
  case STATE_INTRO:
    analogWrite(LED_PIN, 255);
    break;
  case STATE_PLAYING:
    analogWrite(LED_PIN, 255);
    break;
  case STATE_IDLE:
    analogWrite(LED_PIN, 0);
    break;
  case STATE_VOLUME:
    //   analogWrite(LED_PIN, ((now / 300) % 2) ? 255 : 0);  // Blink LED while adjusting volume
    //   targetBrightness = map(volume, 0, 30, 0, 255); // brightness based on volume
    if (volume == 0)
    {
      analogWrite(LED_PIN, 20);
    }
    else
    {
      analogWrite(LED_PIN, map(volume, 0, 30, 0, 255));
      break;
    }
  }
}


// ---------- Main Loop ----------
void loop()
{
  updateButton();
  handleVolumeTimeout();
  updatePlaybackState();
  updateLED();
}
