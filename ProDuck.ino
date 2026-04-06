#include <SPI.h>
#include <SD.h>
#include <HID.h>

// ==================== KONFIGURÁCIA ====================
const int PIN_SD_CS    = 10;
const int PIN_LED      = LED_BUILTIN;
const int PIN_BTN_1    = 2;
const int PIN_BTN_2    = 3;

const bool LED_INVERTED = true;
const bool DEBUG_MODE   = false;

bool isSlovak = false;
bool ledForceOn = false;
volatile bool isRunning = false;

// ==================== DUCKIFY-STYLE HID ====================
namespace keyboard {
  typedef struct report {
    uint8_t modifiers;
    uint8_t reserved;
    uint8_t keys[6];
  } report;
  
  report prev_report = report { 0x00, 0x00, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
  
  const uint8_t keyboardDescriptor[] PROGMEM = {
    0x05, 0x01, 0x09, 0x06, 0xa1, 0x01, 0x85, 0x02, 0x05, 0x07, 0x19, 0xe0, 0x29, 0xe7, 
    0x15, 0x00, 0x25, 0x01, 0x75, 0x01, 0x95, 0x08, 0x81, 0x02, 0x95, 0x01, 0x75, 0x08, 
    0x81, 0x03, 0x95, 0x06, 0x75, 0x08, 0x15, 0x00, 0x25, 0x73, 0x05, 0x07, 0x19, 0x00, 
    0x29, 0x73, 0x81, 0x00, 0xc0
  };
  
  report makeReport(uint8_t modifiers = 0, uint8_t k1 = 0, uint8_t k2 = 0, 
                    uint8_t k3 = 0, uint8_t k4 = 0, uint8_t k5 = 0, uint8_t k6 = 0) {
    return report { modifiers, 0x00, { k1, k2, k3, k4, k5, k6 } };
  }
  
  void begin() {
    static HIDSubDescriptor node(keyboardDescriptor, sizeof(keyboardDescriptor));
    HID().AppendDescriptor(&node);
  }
  
  void send(report* k) { HID().SendReport(2, (uint8_t*)k, sizeof(report)); }
  void release() { prev_report = makeReport(); send(&prev_report); }
  
  void type(uint8_t k0, uint8_t k1, uint8_t k2, uint8_t k3, 
            uint8_t k4, uint8_t k5, uint8_t mod) {
    prev_report = makeReport(mod, k0, k1, k2, k3, k4, k5);
    send(&prev_report);
    release();
  }
}

inline void setLED(bool on) {
  digitalWrite(PIN_LED, LED_INVERTED ? !on : on);
}

// ==================== SK LAYOUT: ASCII → {mod, code} ====================
bool getSKKey(char c, uint8_t* mod, uint8_t* code) {
  switch(c) {
    case 'a': *mod=0; *code=4; return true;   case 'b': *mod=0; *code=5; return true;
    case 'c': *mod=0; *code=6; return true;   case 'd': *mod=0; *code=7; return true;
    case 'e': *mod=0; *code=8; return true;   case 'f': *mod=0; *code=9; return true;
    case 'g': *mod=0; *code=10; return true;  case 'h': *mod=0; *code=11; return true;
    case 'i': *mod=0; *code=12; return true;  case 'j': *mod=0; *code=13; return true;
    case 'k': *mod=0; *code=14; return true;  case 'l': *mod=0; *code=15; return true;
    case 'm': *mod=0; *code=16; return true;  case 'n': *mod=0; *code=17; return true;
    case 'o': *mod=0; *code=18; return true;  case 'p': *mod=0; *code=19; return true;
    case 'q': *mod=0; *code=20; return true;  case 'r': *mod=0; *code=21; return true;
    case 's': *mod=0; *code=22; return true;  case 't': *mod=0; *code=23; return true;
    case 'u': *mod=0; *code=24; return true;  case 'v': *mod=0; *code=25; return true;
    case 'w': *mod=0; *code=26; return true;  case 'x': *mod=0; *code=27; return true;
    case 'y': *mod=0; *code=28; return true;  case 'z': *mod=0; *code=29; return true;
    
    case 'A': *mod=2; *code=4; return true;   case 'B': *mod=2; *code=5; return true;
    case 'C': *mod=2; *code=6; return true;   case 'D': *mod=2; *code=7; return true;
    case 'E': *mod=2; *code=8; return true;   case 'F': *mod=2; *code=9; return true;
    case 'G': *mod=2; *code=10; return true;  case 'H': *mod=2; *code=11; return true;
    case 'I': *mod=2; *code=12; return true;  case 'J': *mod=2; *code=13; return true;
    case 'K': *mod=2; *code=14; return true;  case 'L': *mod=2; *code=15; return true;
    case 'M': *mod=2; *code=16; return true;  case 'N': *mod=2; *code=17; return true;
    case 'O': *mod=2; *code=18; return true;  case 'P': *mod=2; *code=19; return true;
    case 'Q': *mod=2; *code=20; return true;  case 'R': *mod=2; *code=21; return true;
    case 'S': *mod=2; *code=22; return true;  case 'T': *mod=2; *code=23; return true;
    case 'U': *mod=2; *code=24; return true;  case 'V': *mod=2; *code=25; return true;
    case 'W': *mod=2; *code=26; return true;  case 'X': *mod=2; *code=27; return true;
    case 'Y': *mod=2; *code=28; return true;  case 'Z': *mod=2; *code=29; return true;
    
    case '0': *mod=2; *code=39; return true;  case '1': *mod=2; *code=30; return true;
    case '2': *mod=2; *code=31; return true;  case '3': *mod=2; *code=32; return true;
    case '4': *mod=2; *code=33; return true;  case '5': *mod=2; *code=34; return true;
    case '6': *mod=2; *code=35; return true;  case '7': *mod=2; *code=36; return true;
    case '8': *mod=2; *code=37; return true;  case '9': *mod=2; *code=38; return true;
    
    case ' ': *mod=0; *code=44; return true;  case ',': *mod=0; *code=54; return true;
    case '.': *mod=0; *code=55; return true;  case '\'': *mod=0; *code=53; return true;
    case '-': *mod=0; *code=56; return true;  case '_': *mod=2; *code=56; return true;
    case '+': *mod=0; *code=46; return true;  case '=': *mod=0; *code=39; return true;
    
    case '!': *mod=2; *code=30; return true;  case '"': *mod=2; *code=31; return true;
    case '%': *mod=2; *code=34; return true;  case '(': *mod=2; *code=48; return true;
    case ')': *mod=2; *code=49; return true;  case '?': *mod=2; *code=54; return true;
    case ':': *mod=2; *code=55; return true;  case '/': *mod=2; *code=47; return true;
    case ';': *mod=2; *code=53; return true;
    
    case '@': *mod=64; *code=25; return true;  case '#': *mod=64; *code=27; return true;
    case '&': *mod=64; *code=6; return true;   case '*': *mod=64; *code=56; return true;
    case '{': *mod=64; *code=5; return true;   case '}': *mod=64; *code=17; return true;
    case '[': *mod=64; *code=9; return true;   case ']': *mod=64; *code=10; return true;
    case '\\': *mod=64; *code=20; return true; case '|': *mod=64; *code=26; return true;
    case '<': *mod=64; *code=54; return true;  case '>': *mod=64; *code=55; return true;
    case '^': *mod=64; *code=32; return true;  case '`': *mod=64; *code=36; return true;
    case '~': *mod=64; *code=30; return true;  case '$': *mod=64; *code=53; return true;
    
    default: return false;
  }
}

char swapYZ(char c) {
  if (c == 'y') return 'z'; if (c == 'z') return 'y';
  if (c == 'Y') return 'Z'; if (c == 'Z') return 'Y';
  return c;
}

void sendASCII(char c) {
  uint8_t mod, code;
  char swapped = swapYZ(c);
  if (getSKKey(swapped, &mod, &code)) {
    keyboard::type(code, 0, 0, 0, 0, 0, mod);
  } else {
    keyboard::type((uint8_t)c, 0, 0, 0, 0, 0, 0);
  }
}

// ==================== UTF-8: SLOVENSKÁ DIAKRITIKA ====================
void sendSlovakDiacritic(uint8_t b1, uint8_t b2) {
  if (b1 == 0xC3) {
    switch(b2) {
      case 0xA1: keyboard::type(37, 0,0,0,0,0, 0); break;  // á
      case 0xA9: keyboard::type(39, 0,0,0,0,0, 0); break;  // é
      case 0xAD: keyboard::type(38, 0,0,0,0,0, 0); break;  // í
      case 0xB3: keyboard::type(46, 0,0,0,0,0, 0); break;  // ó
      case 0xBA: keyboard::type(56, 0,0,0,0,0, 0); break;  // ú
      case 0xBD: keyboard::type(36, 0,0,0,0,0, 0); break;  // ý
      case 0xA4: keyboard::type(48, 0,0,0,0,0, 0); break;  // ä
      
      case 0xB4: // ô = SHIFT+100 + o
        keyboard::type(100, 0,0,0,0,0, 2);
        delay(10);
        keyboard::type(18, 0,0,0,0,0, 0);
        break;
      
      // Veľké s dĺžňom = deadkey (SHIFT+46) + písmeno
      case 0x81: case 0x84: case 0x89: case 0x8D: 
      case 0x93: case 0x9A: case 0x9D:
        {
          keyboard::type(46, 0,0,0,0,0, 2);
          delay(10);
          char base = (b2==0x84)?'A':(b2==0x89)?'E':(b2==0x8D)?'I':
                      (b2==0x93)?'O':(b2==0x9A)?'U':'Y';
          sendASCII(base);
        } break;
    }
  }
  else if (b1 == 0xC4) {
    switch(b2) {
      case 0x8D: keyboard::type(33, 0,0,0,0,0, 0); break;  // č
      case 0xA1: keyboard::type(32, 0,0,0,0,0, 0); break;  // š
      case 0xBE: keyboard::type(35, 0,0,0,0,0, 0); break;  // ž
      
      // Ostatné s mäkčeňom = deadkey + písmeno
      case 0x8F: case 0x88: case 0x95: case 0x91: case 0xBA:
        {
          keyboard::type(46, 0,0,0,0,0, 2);
          delay(10);
          char base = (b2==0x8F)?'d':(b2==0x88)?'n':(b2==0x95)?'t':
                      (b2==0x91)?'r':'l';
          sendASCII(base);
        } break;
      
      // Veľké s mäkčeňom
      case 0x8C: case 0x87: case 0x94: case 0x90: case 0xB9: case 0xA0: case 0xBD:
        {
          keyboard::type(46, 0,0,0,0,0, 2);
          delay(10);
          char base = (b2==0x8C)?'C':(b2==0x87)?'N':(b2==0x94)?'T':
                      (b2==0x90)?'R':(b2==0xB9)?'L':(b2==0xA0)?'S':'Z';
          sendASCII(base);
        } break;
    }
  }
}

// ==================== PARSER ====================
void processLine(const String& line) {
  if (line.startsWith("//") || line.length() == 0) return;
  
  int spaceIdx = line.indexOf(' ');
  String cmd = (spaceIdx == -1) ? line : line.substring(0, spaceIdx);
  String arg = (spaceIdx == -1) ? "" : line.substring(spaceIdx + 1);
  
  if (cmd == "STRING") {
    for (size_t i = 0; i < arg.length(); i++) {
      uint8_t b1 = (uint8_t)arg[i];
      if (b1 >= 0xC0 && i + 1 < arg.length()) {
        uint8_t b2 = (uint8_t)arg[i + 1];
        sendSlovakDiacritic(b1, b2);
        i++;
      } else {
        sendASCII((char)b1);
      }
      delay(3);
    }
  }
  else if (cmd == "DELAY") delay(constrain(arg.toInt(), 0, 10000));
  else if (cmd == "ENTER") keyboard::type(40, 0,0,0,0,0, 0);
  else if (cmd == "TAB") keyboard::type(43, 0,0,0,0,0, 0);
  else if (cmd == "GUI" || cmd == "WIN") {
    if (arg.length() > 0) {
      uint8_t mod, code; char key = swapYZ(arg[0]);
      if (getSKKey(key, &mod, &code)) keyboard::type(code, 0,0,0,0,0, 0x08 | mod);
    } else keyboard::type(0,0,0,0,0,0, 0x08);
  }
  else if (cmd == "CTRL") {
    if (arg.length() > 0) {
      uint8_t mod, code; char key = swapYZ(arg[0]);
      if (getSKKey(key, &mod, &code)) keyboard::type(code, 0,0,0,0,0, 0x01 | mod);
    } else keyboard::type(0,0,0,0,0,0, 0x01);
  }
  else if (cmd == "ALT") {
    if (arg.length() > 0) {
      uint8_t mod, code; char key = swapYZ(arg[0]);
      if (getSKKey(key, &mod, &code)) keyboard::type(code, 0,0,0,0,0, 0x04 | mod);
    } else keyboard::type(0,0,0,0,0,0, 0x04);
  }
  else if (cmd == "SHIFT") {
    if (arg.length() > 0) {
      uint8_t mod, code; char key = swapYZ(arg[0]);
      if (getSKKey(key, &mod, &code)) keyboard::type(code, 0,0,0,0,0, 0x02 | mod);
    }
  }
  else if (cmd == "LedON") { ledForceOn = true; setLED(true); }
  else if (cmd == "LedOFF") { ledForceOn = false; setLED(false); }
  
  #if DEBUG_MODE
    Serial.print("[CMD] "); Serial.println(cmd);
  #endif
}

// ==================== EXECUTOR ====================
bool runInjection(const char* filename) {
  File f = SD.open(filename, FILE_READ);
  if (!f) return false;
  String locale = f.readStringUntil('\n');
  locale.trim(); locale.replace("\r", "");
  isSlovak = (locale == "LocaleSK");
  delay(150);
  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim(); line.replace("\r", "");
    if (line.length() > 0) processLine(line);
  }
  f.close(); return true;
}

// ==================== SETUP & LOOP ====================
void setup() {
  #if DEBUG_MODE
    Serial.begin(115200); while(!Serial); Serial.println(F("[INIT] BadUSB SK Ready"));
  #endif
  keyboard::begin();
  pinMode(PIN_LED, OUTPUT); setLED(false);
  pinMode(PIN_BTN_1, INPUT_PULLUP); pinMode(PIN_BTN_2, INPUT_PULLUP);
  if (!SD.begin(PIN_SD_CS)) {
    while(true) { for(int i=0;i<5;i++){setLED(true);delay(80);setLED(false);delay(80);} delay(1000); }
  }
  setLED(true); delay(300); setLED(false);
  #if DEBUG_MODE
    Serial.println(F("[OK] Waiting for button..."));
  #endif
}

void loop() {
  if (isRunning) return;
  bool btn1 = digitalRead(PIN_BTN_1) == LOW;
  bool btn2 = digitalRead(PIN_BTN_2) == LOW;
  if (btn1 || btn2) {
    delay(50);
    if ((btn1 && digitalRead(PIN_BTN_1) == LOW) || (btn2 && digitalRead(PIN_BTN_2) == LOW)) {
      isRunning = true; ledForceOn = false; setLED(true); delay(100);
      bool ok = runInjection(btn1 ? "inject.txt" : "inject2.txt");
      if (ok) {
        if (!ledForceOn) {
          setLED(false);delay(150);setLED(true);delay(150);
          setLED(false);delay(150);setLED(true);delay(150); setLED(false);
        }
      } else {
        for(int i=0;i<3;i++){setLED(true);delay(300);setLED(false);delay(300);}
        ledForceOn = false;
      }
      while(digitalRead(btn1?PIN_BTN_1:PIN_BTN_2)==LOW) delay(10);
      delay(200); isRunning = false;
    }
  }
  delay(10);
}
