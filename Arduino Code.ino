#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <NimBLEDevice.h>
#include <Preferences.h>

// User hardware libraries
#include <Keypad.h>
#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>

// ═══════════════════════════════════════════
// STEP 1 — SET YOUR UNIQUE DEVICE ID & TYPE
// ═══════════════════════════════════════════
const char* DEVICE_ID   = "lock_01";       // ← Dashboard must use this ID
const char* DEVICE_TYPE = "hybrid";        // ← Acts as both sensor & actuator

const char* API_URL         = "https://io-t-smart-hub-ten.vercel.app/api/device/update";
const char* API_COMMAND_URL = "https://io-t-smart-hub-ten.vercel.app/api/device/command";

#define LED_PIN          LED_BUILTIN
#define SENSOR_INTERVAL  3000    // Send data every 3s (changed as requested)
#define COMMAND_INTERVAL 5000    // Poll commands every 5s

// BLE UUIDs — DO NOT CHANGE
#define BLE_SERVICE_UUID  "0000FFF6-0000-1000-8000-00805F9B34FB"
#define BLE_CHAR_SSID     "0000FFF7-0000-1000-8000-00805F9B34FB"
#define BLE_CHAR_PASS     "0000FFF8-0000-1000-8000-00805F9B34FB"
#define BLE_CHAR_STATUS   "0000FFF9-0000-1000-8000-00805F9B34FB"
#define BLE_CHAR_COMMAND  "0000FFFA-0000-1000-8000-00805F9B34FB"

enum DeviceState { STATE_BLE_ADVERTISING, STATE_CONNECTING_WIFI, STATE_RUNNING };
DeviceState currentState = STATE_BLE_ADVERTISING;

Preferences prefs;
HTTPClient  httpClient;
NimBLECharacteristic* charStatus = nullptr;

String wifiSSID = "NITP", wifiPassword = "Admin#2024";
bool   credsReceived = false;
unsigned long lastSensorSend = 0, lastCommandPoll = 0, lastBlink = 0;

// ═══════════════════════════════════════════
// SMART LOCK VARIABLES
// ═══════════════════════════════════════════
#define RELAY_PIN 4

// Fingerprint Config
HardwareSerial mySerial(1);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// Keypad Config
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {13, 12, 11, 10};
byte colPins[COLS] = {9, 46, 3, 8};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// State Variables
String correctPIN = "125";
String enteredPIN = "";
bool isUnlocked = false;
unsigned long unlockTime = 0;

// Enrollment State Machine
bool isEnrolling = false;
uint8_t enrollStep = 0;
uint8_t enrollID = 0;


// Function Prototypes
void sendStateUpdate();
void lockDoor();
void unlockDoor();

// ═══════════════════════════════════════════
// STEP 2 — HARDWARE INIT
// ═══════════════════════════════════════════
void initHardware() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Relay Setup
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // LOCKED by default

  // Fingerprint Setup (RX=16, TX=17)
  mySerial.begin(57600, SERIAL_8N1, 16, 17);
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Fingerprint ready ✅");
  } else {
    Serial.println("Fingerprint not detected ❌");
  }
}

// ═══════════════════════════════════════════
// STEP 3 — READ YOUR SENSOR DATA (Sent every 3s)
// ═══════════════════════════════════════════
void readSensors(JsonObject& data) {
  data["status"] = isUnlocked ? "unlocked" : "locked";
  data["current_pin"] = correctPIN; // Keep visible on dashboard
  data["enroll_mode"] = isEnrolling;
  data["fingerprint_sensor"] = finger.verifyPassword() ? "online" : "offline";
}

// ═══════════════════════════════════════════
// STEP 4 — HANDLE COMMANDS FROM DASHBOARD
// ═══════════════════════════════════════════
void handleAction(const char* action, JsonObject& cmd) {
  if (strcmp(action, "unlock") == 0) {
    unlockDoor();
  } 
  else if (strcmp(action, "lock") == 0) {
    lockDoor();
  } 
  else if (strcmp(action, "set_pin") == 0) {
    if (cmd.containsKey("pin")) {
      correctPIN = cmd["pin"].as<String>();
      Serial.println("✅ PIN changed via SmartHub to: " + correctPIN);
    }
  } 
  else if (strcmp(action, "enroll") == 0) {
    // Requires an "id" parameter from Dashboard (e.g. 1 to 127)
    enrollID = cmd["id"] | 1; 
    isEnrolling = true;
    enrollStep = 1;
    Serial.printf("🖐️ Started enrollment mode for ID #%d\n", enrollID);
  }
}

// ═══════════════════════════════════════════
// LOCK CONTROL LOGIC (Non-Blocking)
// ═══════════════════════════════════════════
void unlockDoor() {
  if (isUnlocked) return;
  Serial.println("🔓 Door Unlocked");
  digitalWrite(RELAY_PIN, LOW);
  isUnlocked = true;
  unlockTime = millis();
  sendStateUpdate(); // Tell hub immediately
}

void lockDoor() {
  if (!isUnlocked) return;
  Serial.println("🔒 Door Locked");
  digitalWrite(RELAY_PIN, HIGH);
  isUnlocked = false;
  sendStateUpdate(); // Tell hub immediately
}

void checkKeypad() {
  char key = keypad.getKey();
  if (key) {
    Serial.print("Key: "); Serial.println(key);
    if (key == '#') {
      if (enteredPIN == correctPIN) {
        Serial.println("✅ Correct PIN");
        unlockDoor();
      } else {
        Serial.println("❌ Wrong PIN");
      }
      enteredPIN = "";
    }
    else if (key == '*') {
      enteredPIN = "";
      Serial.println("Cleared");
    }
    else {
      enteredPIN += key;
    }
  }
}

void checkFingerprint() {
  if (finger.getImage() != FINGERPRINT_OK) return;
  if (finger.image2Tz() != FINGERPRINT_OK) return;

  if (finger.fingerFastSearch() == FINGERPRINT_OK) {
    Serial.print("✅ Fingerprint ID: ");
    Serial.println(finger.fingerID);
    unlockDoor();
  } else {
    Serial.println("❌ Fingerprint Not Recognized");
  }
}

// Non-blocking fingerprint enrollment state machine
void handleEnrollment() {
  if (enrollStep == 1) {
    Serial.println("Waiting for valid finger to enroll...");
    if (finger.getImage() == FINGERPRINT_OK) {
      if (finger.image2Tz(1) == FINGERPRINT_OK) {
        Serial.println("✅ Image taken. Remove finger.");
        enrollStep = 2;
        delay(2000); // Small pause for user to lift finger
      }
    }
  } else if (enrollStep == 2) {
    if (finger.getImage() == FINGERPRINT_NOFINGER) {
      Serial.println("Place same finger again...");
      enrollStep = 3;
    }
  } else if (enrollStep == 3) {
    if (finger.getImage() == FINGERPRINT_OK) {
      if (finger.image2Tz(2) == FINGERPRINT_OK) {
        if (finger.createModel() == FINGERPRINT_OK) {
          if (finger.storeModel(enrollID) == FINGERPRINT_OK) {
            Serial.printf("✅ Enrollment Success! Saved as ID #%d\n", enrollID);
          } else {
            Serial.println("❌ Failed to store model.");
          }
        } else {
          Serial.println("❌ Failed to create model (fingers didn't match).");
        }
        isEnrolling = false; // Exit enroll mode
        sendStateUpdate();
      }
    }
  }
}

// ════════════════════════════════════════════════════════════
// INFRASTRUCTURE — do not modify below unless you know why
// ════════════════════════════════════════════════════════════

void log(const String& tag, const String& msg) {
  Serial.printf("[%8lu][%-10s] %s\n", millis(), tag.c_str(), msg.c_str());
}

void executeCommandJson(const String& json) {
  StaticJsonDocument<256> doc;
  if (deserializeJson(doc, json)) return;
  JsonObject cmd    = doc.containsKey("command") ? doc["command"].as<JsonObject>() : doc.as<JsonObject>();
  const char* action = cmd["action"] | "unknown";
  log("CMD", "action=" + String(action));
  handleAction(action, cmd);
  sendStateUpdate();
}

class SSIDCallback : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* ch, NimBLEConnInfo& c) override { wifiSSID = String(ch->getValue().c_str()); }
};
class PassCallback : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* ch, NimBLEConnInfo& c) override {
    wifiPassword = String(ch->getValue().c_str());
    if (wifiSSID.length() > 0) {
      credsReceived = true;
      prefs.begin("wifi", false);
      prefs.putString("ssid", wifiSSID);
      prefs.putString("pass", wifiPassword);
      prefs.end();
    }
  }
};
class CmdCallback : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* ch, NimBLEConnInfo& c) override { executeCommandJson(String(ch->getValue().c_str())); }
};

void setupBLE() {
  NimBLEDevice::init(DEVICE_ID);
  NimBLEDevice::setPower(9);
  NimBLEServer*  srv = NimBLEDevice::createServer();
  NimBLEService* svc = srv->createService(BLE_SERVICE_UUID);
  svc->createCharacteristic(BLE_CHAR_SSID,    NIMBLE_PROPERTY::WRITE)->setCallbacks(new SSIDCallback());
  svc->createCharacteristic(BLE_CHAR_PASS,    NIMBLE_PROPERTY::WRITE)->setCallbacks(new PassCallback());
  charStatus = svc->createCharacteristic(BLE_CHAR_STATUS, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
  charStatus->setValue("{}");
  svc->createCharacteristic(BLE_CHAR_COMMAND, NIMBLE_PROPERTY::WRITE)->setCallbacks(new CmdCallback());
  svc->start();
  NimBLEDevice::getAdvertising()->addServiceUUID(BLE_SERVICE_UUID);
  NimBLEDevice::getAdvertising()->start();
  log("BLE", "Advertising as: " + String(DEVICE_ID));
}

void updateBLEStatus() {
  if (!charStatus) return;
  StaticJsonDocument<128> d;
  d["device_id"] = DEVICE_ID; d["ip"] = WiFi.localIP().toString(); d["type"] = DEVICE_TYPE;
  char b[128]; serializeJson(d, b);
  charStatus->setValue(b); charStatus->notify();
}

bool connectWiFi(const String& ssid, const String& pass) {
  WiFi.mode(WIFI_STA); WiFi.begin(ssid.c_str(), pass.c_str());
  int n = 0;
  while (WiFi.status() != WL_CONNECTED && n++ < 40) { delay(500); Serial.print("."); }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) { log("WiFi", "IP: " + WiFi.localIP().toString()); return true; }
  return false;
}

void sendSensorData() {
  StaticJsonDocument<512> doc;
  doc["device_id"] = DEVICE_ID;
  JsonObject data  = doc.createNestedObject("data");
  readSensors(data);
  data["uptime"] = millis() / 1000;
  data["rssi"]   = WiFi.RSSI();
  char buf[512]; serializeJson(doc, buf);
  httpClient.begin(API_URL);
  httpClient.addHeader("Content-Type", "application/json");
  httpClient.setTimeout(10000);
  int code = httpClient.POST(buf);
  log("CLOUD", code >= 200 && code < 300 ? "✅ " + String(code) : "❌ " + String(code));
  httpClient.end();
}

void sendStateUpdate() {
  StaticJsonDocument<128> doc;
  doc["device_id"]      = DEVICE_ID;
  doc["data"]["status"] = "command_executed";
  doc["data"]["uptime"] = millis() / 1000;
  char buf[128]; serializeJson(doc, buf);
  httpClient.begin(API_URL);
  httpClient.addHeader("Content-Type", "application/json");
  httpClient.setTimeout(8000);
  httpClient.POST(buf); httpClient.end();
}

void pollCommands() {
  String url = String(API_COMMAND_URL) + "?device_id=" + String(DEVICE_ID);
  httpClient.begin(url); httpClient.setTimeout(8000);
  int code = httpClient.GET();
  if (code != 200) { httpClient.end(); return; }
  String body = httpClient.getString(); httpClient.end();
  if (body.length() < 3 || body == "[]") return;
  log("CMD-POLL", body.substring(0, 80) + "...");
  StaticJsonDocument<1024> doc;
  if (deserializeJson(doc, body)) return;
  auto process =[](JsonObject cmd) {
    // Process full JSON body instead of just action so we can read "pin" and "id" params
    const char* action = cmd["command"]["action"] | "unknown";
    JsonObject payload = cmd["command"];
    handleAction(action, payload);
  };
  if (doc.is<JsonArray>()) for (JsonObject c : doc.as<JsonArray>()) process(c);
  else if (doc.is<JsonObject>()) process(doc.as<JsonObject>());
}

void setup() {
  Serial.begin(115200); delay(1000);
  Serial.println("\n╔═══════════════════════════════╗");
  Serial.printf ("║  SmartHub Device: %-13s║\n", DEVICE_ID);
  Serial.println("╚═══════════════════════════════╝");
  initHardware();
  prefs.begin("wifi", true);
  wifiSSID     = prefs.getString("ssid", "");
  wifiPassword = prefs.getString("pass", "");
  prefs.end();
  if (wifiSSID.length() > 0) {
    credsReceived = true; currentState = STATE_CONNECTING_WIFI;
  } else {
    currentState = STATE_BLE_ADVERTISING; setupBLE();
    Serial.println("  Waiting for gateway BLE commissioning...");
  }
}

void loop() {
  if (currentState == STATE_BLE_ADVERTISING) {
    if (millis() - lastBlink > 1000) { lastBlink = millis(); digitalWrite(LED_PIN, !digitalRead(LED_PIN)); }
    if (credsReceived) currentState = STATE_CONNECTING_WIFI;
    return;
  }
  if (currentState == STATE_CONNECTING_WIFI) {
    if (connectWiFi(wifiSSID, wifiPassword)) {
      updateBLEStatus();
      if (MDNS.begin(DEVICE_ID)) {
        MDNS.addService("smarthub","tcp",80);
        MDNS.addServiceTxt("smarthub","tcp","device_id",DEVICE_ID);
      }
      sendSensorData();
      currentState = STATE_RUNNING; lastSensorSend = millis();
      Serial.println("  ✅ COMMISSIONED & RUNNING!");
    } else { delay(5000); }
    return;
  }
  if (currentState == STATE_RUNNING) {
    if (WiFi.status() != WL_CONNECTED) { currentState = STATE_CONNECTING_WIFI; return; }
    
    // Cloud Comms
    if (millis() - lastSensorSend  >= SENSOR_INTERVAL)  { lastSensorSend  = millis(); sendSensorData(); }
    if (millis() - lastCommandPoll >= COMMAND_INTERVAL) { lastCommandPoll = millis(); pollCommands(); }
    
    // Hardware user logic
    if (isUnlocked && (millis() - unlockTime >= 5000)) {
       lockDoor(); // Auto relock after 5s without blocking
    }

    if (!isEnrolling) {
      checkKeypad();
      checkFingerprint();
    } else {
      handleEnrollment(); // Run state machine if in enroll mode
    }

    delay(10);
  }
}
