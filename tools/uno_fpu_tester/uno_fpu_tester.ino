/**
 * RC2014 FPU firmware exerciser for Arduino UNO.
 *
 * This sketch drives the Pico 2 FPU's 8-bit parallel interface and exposes a
 * lightweight command shell over the UNO's USB serial port. It can:
 *  - Strobe raw bytes onto the data bus for ad-hoc command/register tests.
 *  - Sample bytes returned by the FPU during read cycles.
 *  - Bridge the FPU's TTL debug UART back to the host PC for live logs.
 *
 * Wiring (adjust pin assignments as needed):
 *  - D2..D9  -> FPU D0..D7
 *  - D10     -> FPU WR (drive LOW to enable writes)
 *  - D11     -> FPU RD (drive LOW to enable reads)
 *  - D12     -> FPU EN/VALID strobe
 *  - A0 (D14)-> FPU BUSY (active-low; idle HIGH). Use a pull-up if the FPU
 *               board does not provide one.
 *  - A1 (D15)-> UART RX from FPU debug TX
 *  - A2 (D16)-> UART TX to FPU debug RX
 *
 * Serial shell commands (115200 baud):
 *  - help                : Print command list.
 *  - w <byte>            : Write a byte (hex/dec) to the FPU bus with a strobe.
 *  - r                   : Read a byte from the FPU bus and print it.
 *  - pair <a> <b>        : Convenience helper to write two sequential bytes,
 *                          e.g., an address then a payload.
 *  - debug "<text>"      : Send a text payload to the FPU debug UART.
 *  - status              : Report BUSY pin state.
 *
 * NOTE: The exact command framing for the FPU can evolve; this harness simply
 * pulses the VALID/EN line around raw byte writes/reads so you can validate
 * the firmware's command decoder and register interface without extra glue.
 */

#include <Arduino.h>
#include <SoftwareSerial.h>

namespace {

constexpr uint8_t DATA_PINS[8] = {2, 3, 4, 5, 6, 7, 8, 9};
constexpr uint8_t WR_PIN = 10;
constexpr uint8_t RD_PIN = 11;
constexpr uint8_t EN_PIN = 12;
constexpr uint8_t BUSY_PIN = A0;  // Input, active-low from FPU.

constexpr uint8_t DEBUG_RX_PIN = A1;  // UNO receives from FPU TX.
constexpr uint8_t DEBUG_TX_PIN = A2;  // UNO transmits to FPU RX.

constexpr uint16_t STROBE_PULSE_US = 2;
constexpr uint32_t BUSY_TIMEOUT_MS = 200;

SoftwareSerial fpuDebugSerial(DEBUG_RX_PIN, DEBUG_TX_PIN);

enum class DataDirection { Input, Output };

void setDataDirection(DataDirection dir) {
  const uint8_t mode = (dir == DataDirection::Output) ? OUTPUT : INPUT;
  for (uint8_t pin : DATA_PINS) {
    pinMode(pin, mode);
    if (dir == DataDirection::Input) {
      digitalWrite(pin, LOW);  // Disable pull-ups for clean sampling.
    }
  }
}

void driveData(uint8_t value) {
  for (uint8_t bit = 0; bit < 8; ++bit) {
    digitalWrite(DATA_PINS[bit], (value >> bit) & 0x1);
  }
}

uint8_t sampleData() {
  uint8_t value = 0;
  for (uint8_t bit = 0; bit < 8; ++bit) {
    if (digitalRead(DATA_PINS[bit])) {
      value |= (1 << bit);
    }
  }
  return value;
}

bool waitForReady(uint32_t timeoutMs = BUSY_TIMEOUT_MS) {
  const unsigned long start = millis();
  while (digitalRead(BUSY_PIN) == LOW) {
    if (millis() - start > timeoutMs) {
      return false;
    }
  }
  return true;
}

void strobeEnable() {
  digitalWrite(EN_PIN, HIGH);
  delayMicroseconds(STROBE_PULSE_US);
  digitalWrite(EN_PIN, LOW);
}

void writeBusByte(uint8_t value) {
  if (!waitForReady()) {
    Serial.println(F("WARN: BUSY stuck low; write skipped."));
    return;
  }
  setDataDirection(DataDirection::Output);
  digitalWrite(RD_PIN, HIGH);
  digitalWrite(WR_PIN, LOW);
  driveData(value);
  strobeEnable();
  digitalWrite(WR_PIN, HIGH);
}

uint8_t readBusByte() {
  if (!waitForReady()) {
    Serial.println(F("WARN: BUSY stuck low; read may be invalid."));
  }
  setDataDirection(DataDirection::Input);
  digitalWrite(WR_PIN, HIGH);
  digitalWrite(RD_PIN, LOW);
  strobeEnable();
  const uint8_t value = sampleData();
  digitalWrite(RD_PIN, HIGH);
  return value;
}

bool parseByteToken(const String& token, uint8_t& out) {
  char* end = nullptr;
  long val = strtol(token.c_str(), &end, 0);
  if (end == token.c_str() || val < 0 || val > 255) {
    return false;
  }
  out = static_cast<uint8_t>(val);
  return true;
}

void printHelp() {
  Serial.println(F("RC2014 FPU tester commands:"));
  Serial.println(F("  help            - Show this message"));
  Serial.println(F("  w <byte>        - Write byte to bus (hex/dec)"));
  Serial.println(F("  r               - Read byte from bus"));
  Serial.println(F("  pair <a> <b>    - Write two sequential bytes"));
  Serial.println(F("  debug \"text\"  - Send text to FPU debug UART"));
  Serial.println(F("  status          - Show BUSY level (HIGH=idle)"));
}

void handleCommand(const String& line) {
  if (line.length() == 0) {
    return;
  }

  if (line.equalsIgnoreCase(F("help"))) {
    printHelp();
    return;
  }

  if (line.equalsIgnoreCase(F("r"))) {
    const uint8_t value = readBusByte();
    Serial.print(F("READ: 0x"));
    if (value < 16) {
      Serial.print('0');
    }
    Serial.println(value, HEX);
    return;
  }

  if (line.equalsIgnoreCase(F("status"))) {
    Serial.print(F("BUSY pin: "));
    Serial.println(digitalRead(BUSY_PIN) == HIGH ? F("HIGH (idle)") : F("LOW (busy)"));
    return;
  }

  if (line.startsWith(F("w "))) {
    uint8_t value = 0;
    if (parseByteToken(line.substring(2), value)) {
      writeBusByte(value);
      Serial.print(F("WRITE: 0x"));
      if (value < 16) {
        Serial.print('0');
      }
      Serial.println(value, HEX);
    } else {
      Serial.println(F("ERR: Could not parse byte."));
    }
    return;
  }

  if (line.startsWith(F("pair "))) {
    int firstSpace = line.indexOf(' ');
    int secondSpace = line.indexOf(' ', firstSpace + 1);
    if (secondSpace < 0) {
      Serial.println(F("ERR: pair requires two byte arguments."));
      return;
    }
    uint8_t first = 0;
    uint8_t second = 0;
    if (parseByteToken(line.substring(firstSpace + 1, secondSpace), first) &&
        parseByteToken(line.substring(secondSpace + 1), second)) {
      writeBusByte(first);
      writeBusByte(second);
      Serial.print(F("PAIR write: 0x"));
      if (first < 16) {
        Serial.print('0');
      }
      Serial.print(first, HEX);
      Serial.print(F(", 0x"));
      if (second < 16) {
        Serial.print('0');
      }
      Serial.println(second, HEX);
    } else {
      Serial.println(F("ERR: Could not parse bytes."));
    }
    return;
  }

  if (line.startsWith(F("debug "))) {
    int quoteStart = line.indexOf('"');
    int quoteEnd = line.lastIndexOf('"');
    if (quoteStart >= 0 && quoteEnd > quoteStart) {
      String payload = line.substring(quoteStart + 1, quoteEnd);
      fpuDebugSerial.print(payload);
      fpuDebugSerial.flush();
      Serial.println(F("Sent debug payload."));
    } else {
      Serial.println(F("ERR: debug payload must be quoted."));
    }
    return;
  }

  Serial.println(F("ERR: Unknown command. Type 'help' for options."));
}

void pumpDebugUart() {
  while (fpuDebugSerial.available()) {
    Serial.write(fpuDebugSerial.read());
  }
}

String readLineFromHost() {
  String line = Serial.readStringUntil('\n');
  line.trim();
  return line;
}

}  // namespace

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(50);
  fpuDebugSerial.begin(115200);

  pinMode(WR_PIN, OUTPUT);
  pinMode(RD_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);
  pinMode(BUSY_PIN, INPUT_PULLUP);

  digitalWrite(WR_PIN, HIGH);
  digitalWrite(RD_PIN, HIGH);
  digitalWrite(EN_PIN, LOW);
  setDataDirection(DataDirection::Output);
  driveData(0);

  Serial.println(F("RC2014 FPU firmware tester ready. Type 'help' for commands."));
}

void loop() {
  pumpDebugUart();

  if (Serial.available()) {
    String line = readLineFromHost();
    handleCommand(line);
  }
}
