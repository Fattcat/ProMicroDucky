#include <Arduino.h>
#include <HID.h>
#include <SPI.h>
#include <SD.h>

// ==================== PINY ====================
#define PIN_LED 17
#define PIN_BTN_1 2
#define PIN_BTN_2 3
#define PIN_SD_CS 10

#define LED_INVERTED false

bool isRunning = false;
bool ledForceOn = false;

// ==================== HID REPORT ====================

typedef struct report {
  uint8_t modifiers;
  uint8_t reserved;
  uint8_t keys[6];
} report;

report prev_report = {0,0,{0,0,0,0,0,0}};

// ==================== MODIFIERS ====================

// ❌ PÔVODNE:
// ALTGR = 64

// ✅ OPRAVA:
#define MOD_CTRL   0x01
#define MOD_SHIFT  0x02
#define MOD_ALT    0x04
#define MOD_GUI    0x08
#define MOD_ALTGR  (0x40 | 0x01) // ALTGR = CTRL + RIGHT ALT

// ==================== HID DESCRIPTOR ====================

const uint8_t keyboardDescriptor[] PROGMEM = {
  0x05, 0x01, 0x09, 0x06, 0xa1, 0x01, 0x85, 0x02,
  0x05, 0x07, 0x19, 0xe0, 0x29, 0xe7,
  0x15, 0x00, 0x25, 0x01,
  0x75, 0x01, 0x95, 0x08, 0x81, 0x02,
  0x95, 0x01, 0x75, 0x08, 0x81, 0x03,
  0x95, 0x06, 0x75, 0x08,
  0x15, 0x00, 0x25, 0x73,
  0x05, 0x07, 0x19, 0x00, 0x29, 0x73, 0x81, 0x00,
  0xc0
};

report makeReport(uint8_t mod=0, uint8_t k=0) {
  return {mod, 0x00, {k,0,0,0,0,0}};
}

void send(report* r) {
  HID().SendReport(2, (uint8_t*)r, sizeof(report));
}

void release() {
  prev_report = makeReport();
  send(&prev_report);
}

void typeKey(uint8_t code, uint8_t mod=0) {
  prev_report = makeReport(mod, code);
  send(&prev_report);
  delay(8); // ✅ OPRAVA timing
  release();
  delay(8);
}

// ==================== DEAD KEYS ====================

// ❌ PÔVODNE:
// náhodné keycodes

// ✅ OPRAVA (stabilné dead keys)
void dead_acute() { typeKey(45); delay(15); }
void dead_caron() { typeKey(31, MOD_ALTGR); delay(15); }
void dead_circ()  { typeKey(32, MOD_ALTGR); delay(15); }
void dead_dia()   { typeKey(34, MOD_ALTGR); delay(15); }

// ==================== SK ASCII MAP ====================

bool getSKKey(char c, uint8_t* mod, uint8_t* code) {
  switch(c) {

    // LETTERS
    case 'a': *mod=0; *code=4; return true;
    case 'A': *mod=MOD_SHIFT; *code=4; return true;

    case 'b': *mod=0; *code=5; return true;
    case 'B': *mod=MOD_SHIFT; *code=5; return true;

    // ... (skrátené – rovnaký pattern pokračuje)

    // ==================== NUMBERS ====================

    // ❌ PÔVODNE:
    // case '1': *mod=2; *code=30;

    // ✅ OPRAVA:
    case '1': *mod=0; *code=30; return true;
    case '!': *mod=MOD_SHIFT; *code=30; return true;

    case '2': *mod=0; *code=31; return true;
    case '"': *mod=MOD_SHIFT; *code=31; return true;

    case '3': *mod=0; *code=32; return true;
    case '#': *mod=MOD_ALTGR; *code=32; return true;

    case '4': *mod=0; *code=33; return true;
    case '$': *mod=MOD_ALTGR; *code=33; return true;

    case '5': *mod=0; *code=34; return true;
    case '%': *mod=MOD_SHIFT; *code=34; return true;

    case '6': *mod=0; *code=35; return true;
    case '&': *mod=MOD_ALTGR; *code=35; return true;

    case '7': *mod=0; *code=36; return true;
    case '/': *mod=MOD_SHIFT; *code=36; return true;

    case '8': *mod=0; *code=37; return true;
    case '(': *mod=MOD_SHIFT; *code=37; return true;

    case '9': *mod=0; *code=38; return true;
    case ')': *mod=MOD_SHIFT; *code=38; return true;

    case '0': *mod=0; *code=39; return true;
    case '=': *mod=MOD_ALTGR; *code=39; return true;

    // SYMBOLS
    case '+': *mod=MOD_ALTGR; *code=30; return true;
    case '-': *mod=0; *code=56; return true;
    case '_': *mod=MOD_SHIFT; *code=56; return true;

    case '<': *mod=MOD_ALTGR; *code=54; return true;
    case '>': *mod=MOD_ALTGR; *code=55; return true;

    case '\\': *mod=MOD_ALTGR; *code=20; return true;
    case '|': *mod=MOD_ALTGR; *code=26; return true;

    case '{': *mod=MOD_ALTGR; *code=5; return true;
    case '}': *mod=MOD_ALTGR; *code=17; return true;

    case '[': *mod=MOD_ALTGR; *code=9; return true;
    case ']': *mod=MOD_ALTGR; *code=10; return true;

    case '@': *mod=MOD_ALTGR; *code=25; return true;

    case ' ': *mod=0; *code=44; return true;

    default: return false;
  }
}

// ==================== SEND ====================

// ❌ PÔVODNE:
// swapYZ()

// ✅ OPRAVA: odstránené
void sendASCII(char c) {
  uint8_t mod, code;
  if (getSKKey(c, &mod, &code)) {
    typeKey(code, mod);
  }
}

// ==================== DIAKRITIKA ====================

void sendSKExtended(char c) {
  switch(c) {

    case 'á': dead_acute(); sendASCII('a'); break;
    case 'č': dead_caron(); sendASCII('c'); break;
    case 'ď': dead_caron(); sendASCII('d'); break;
    case 'é': dead_acute(); sendASCII('e'); break;
    case 'í': dead_acute(); sendASCII('i'); break;
    case 'ľ': dead_caron(); sendASCII('l'); break;
    case 'ň': dead_caron(); sendASCII('n'); break;
    case 'ó': dead_acute(); sendASCII('o'); break;
    case 'ô': dead_circ(); sendASCII('o'); break;
    case 'š': dead_caron(); sendASCII('s'); break;
    case 'ť': dead_caron(); sendASCII('t'); break;
    case 'ú': dead_acute(); sendASCII('u'); break;
    case 'ý': dead_acute(); sendASCII('y'); break;
    case 'ž': dead_caron(); sendASCII('z'); break;

    default:
      sendASCII(c);
  }
}

// ==================== PARSER ====================

void processLine(const String& line) {

  if (line.startsWith("//") || line.length() == 0) return;

  if (line.startsWith("STRING ")) {
    String text = line.substring(7);

    for (size_t i=0;i<text.length();i++) {
      sendSKExtended(text[i]);
      delay(10);
    }
  }

  else if (line == "ENTER") typeKey(40);
  else if (line == "TAB") typeKey(43);

  else if (line.startsWith("DELAY ")) {
    delay(constrain(line.substring(6).toInt(),0,10000));
  }
}

// ==================== LED ====================

void setLED(bool on) {
  digitalWrite(PIN_LED, LED_INVERTED ? !on : on);
}

// ==================== SETUP ====================

void setup() {

  HIDSubDescriptor node(keyboardDescriptor, sizeof(keyboardDescriptor));
  HID().AppendDescriptor(&node);

  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BTN_1, INPUT_PULLUP);
  pinMode(PIN_BTN_2, INPUT_PULLUP);

  SD.begin(PIN_SD_CS);

  setLED(true);
  delay(300);
  setLED(false);
}

// ==================== LOOP ====================

void loop() {

  if (digitalRead(PIN_BTN_1) == LOW) {
    File f = SD.open("inject.txt");

    while (f.available()) {
      processLine(f.readStringUntil('\n'));
    }

    f.close();
  }

  delay(50);
}
