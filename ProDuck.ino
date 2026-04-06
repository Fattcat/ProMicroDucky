// For research & educational usage.
#include <SPI.h>
#include <SD.h>
#include <HID-Project.h>

const int chipSelect = 10;
const int LED_READY = LED_BUILTIN_TX;
bool isSlovak = false;

void setup() {
  pinMode(LED_READY, OUTPUT);
  digitalWrite(LED_READY, HIGH); // Vypnutá (invertovaná)

  if (!SD.begin(chipSelect)) {
    blinkError(3);
    return;
  }

  File injectFile = SD.open("inject.txt");
  if (!injectFile) {
    blinkError(5);
    return;
  }

  // Prečítanie Locale
  String locale = injectFile.readStringUntil('\n');
  locale.trim();
  isSlovak = (locale == "LocaleSK");

  digitalWrite(LED_READY, LOW); // Status: READY
  delay(1000); 

  // --- KONTROLA CAPS LOCKU ---
  // Ak svieti Caps Lock, Arduino ho vypne, aby nerozbilo STRINGy
  if (BootKeyboard.getLeds() & LED_CAPS_LOCK) {
    Keyboard.write(KEY_CAPS_LOCK);
    delay(100);
  }

  // Hlavný Parser Loop
  while (injectFile.available()) {
    String line = injectFile.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) processLine(line);
  }

  injectFile.close();
  digitalWrite(LED_READY, HIGH); // Koniec
  Keyboard.end();
}

void loop() {}

void processLine(String line) {
  if (line.startsWith("//")) return;

  int spaceIdx = line.indexOf(' ');
  String command = (spaceIdx == -1) ? line : line.substring(0, spaceIdx);
  String arg = (spaceIdx == -1) ? "" : line.substring(spaceIdx + 1);

  if (command == "STRING") {
    for (int i = 0; i < arg.length(); i++) {
      uint8_t c = arg[i];
      // Detekcia UTF-8 pre slovenskú diakritiku
      if (isSlovak && (c == 0xC3 || c == 0xC4)) {
        i++;
        handleSlovakUTF8(c, (uint8_t)arg[i]);
      } else {
        sendKeySK_Basic(c);
      }
      delay(2);
    }
  } 
  else if (command == "DELAY") {
    delay(arg.toInt());
  } 
  else if (command == "ENTER") {
    Keyboard.write(KEY_ENTER);
  } 
  else if (command == "GUI" || command == "WIN") {
    if (arg.length() > 0) {
      Keyboard.press(KEY_LEFT_GUI);
      Keyboard.press(arg[0]);
      delay(50);
      Keyboard.releaseAll();
    } else {
      Keyboard.write(KEY_LEFT_GUI);
    }
  }
  else if (command == "CTRL") {
    Keyboard.press(KEY_LEFT_CTRL);
    if (arg.length() > 0) Keyboard.press(arg[0]);
    delay(50); Keyboard.releaseAll();
  }
  else if (command == "ALT") {
    Keyboard.press(KEY_LEFT_ALT);
    if (arg.length() > 0) Keyboard.press(arg[0]);
    delay(50); Keyboard.releaseAll();
  }
  else if (command == "TAB") {
    Keyboard.write(KEY_TAB);
  }
}

// --- LOGIKA PRE SLOVENSKÚ DIAKRITIKU (UTF-8) ---
void handleSlovakUTF8(uint8_t b1, uint8_t b2) {
  if (b1 == 0xC3) {
    switch (b2) {
      case 0xA1: Keyboard.write('8'); break; // á
      case 0x81: typeShift('8'); break;      // Á
      case 0xA9: Keyboard.write('0'); break; // é
      case 0x89: typeDeadKey(false, 'E'); break; 
      case 0xAD: Keyboard.write('9'); break; // í
      case 0x8D: typeDeadKey(false, 'I'); break;
      case 0xB3: Keyboard.write('='); break; // ó
      case 0x93: typeDeadKey(false, 'O'); break;
      case 0xBA: Keyboard.write(';'); break; // ú
      case 0x9A: typeDeadKey(false, 'U'); break;
      case 0xBD: Keyboard.write('y'); break; // ý
      case 0x9D: typeDeadKey(false, 'Y'); break;
      case 0xA4: Keyboard.write('7'); break; // ä
      case 0x84: typeDeadKey(true, 'A'); break;
      case 0xB4: typeShift('6'); Keyboard.write('o'); break; // ô
      default: break;
    }
  } else if (b1 == 0xC4) {
    switch (b2) {
      case 0x8D: Keyboard.write('4'); break; // č
      case 0x8C: typeShift('4'); break;      // Č
      case 0x8F: typeDeadKey(true, 'd'); break;  // ď
      case 0x8E: typeDeadKey(true, 'D'); break;
      case 0xBA: typeDeadKey(true, 'l'); break;  // ľ
      case 0xB9: typeDeadKey(true, 'L'); break;
      case 0x9A: typeDeadKey(false, 'l'); break; // ĺ
      case 0x99: typeDeadKey(false, 'L'); break;
      case 0x88: typeDeadKey(true, 'n'); break;  // ň
      case 0x87: typeDeadKey(true, 'N'); break;
      case 0x95: typeShift(';'); break;          // ť
      case 0x94: typeDeadKey(true, 'T'); break;  // Ť
      case 0x91: typeDeadKey(false, 'r'); break; // ŕ
      case 0x90: typeDeadKey(false, 'R'); break;
      case 0xA1: Keyboard.write('3'); break;     // š
      case 0xA0: typeShift('3'); break;
      case 0xBE: Keyboard.write('6'); break;     // ž
      case 0xBD: typeShift('6'); break;
      default: break;
    }
  }
}

// --- ZÁKLADNÉ SYMBOLY A Y/Z ---
void sendKeySK_Basic(char c) {
  if (!isSlovak) { Keyboard.write(c); return; }
  switch (c) {
    case 'y': Keyboard.write('z'); break;
    case 'z': Keyboard.write('y'); break;
    case 'Y': typeShift('Z'); break;
    case 'Z': typeShift('Y'); break;
    case '@': Keyboard.press(KEY_RIGHT_ALT); Keyboard.write('v'); Keyboard.releaseAll(); break;
    case '#': Keyboard.press(KEY_RIGHT_ALT); Keyboard.write('x'); Keyboard.releaseAll(); break;
    case '&': Keyboard.press(KEY_RIGHT_ALT); Keyboard.write('c'); Keyboard.releaseAll(); break;
    case '{': Keyboard.press(KEY_RIGHT_ALT); Keyboard.write('b'); Keyboard.releaseAll(); break;
    case '}': Keyboard.press(KEY_RIGHT_ALT); Keyboard.write('n'); Keyboard.releaseAll(); break;
    case '[': Keyboard.press(KEY_RIGHT_ALT); Keyboard.write('f'); Keyboard.releaseAll(); break;
    case ']': Keyboard.press(KEY_RIGHT_ALT); Keyboard.write('g'); Keyboard.releaseAll(); break;
    case '\\': Keyboard.press(KEY_RIGHT_ALT); Keyboard.write('q'); Keyboard.releaseAll(); break;
    case '<': Keyboard.press(KEY_RIGHT_ALT); Keyboard.write('&'); Keyboard.releaseAll(); break;
    case '>': Keyboard.press(KEY_RIGHT_ALT); Keyboard.write('*'); Keyboard.releaseAll(); break;
    case '/': typeShift('ù'); break;
    case ':': typeShift('.'); break;
    case '-': Keyboard.write('/'); break;
    case '_': typeShift('/'); break;
    default: Keyboard.write(c); break;
  }
}

void typeDeadKey(bool isAccent, char letter) {
  if (isAccent) { typeShift('='); } else { Keyboard.write('='); }
  delay(15);
  Keyboard.write(letter);
}

void typeShift(char c) {
  Keyboard.press(KEY_LEFT_SHIFT);
  Keyboard.write(c);
  Keyboard.releaseAll();
}

void blinkError(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_READY, LOW); delay(200);
    digitalWrite(LED_READY, HIGH); delay(200);
  }
}
