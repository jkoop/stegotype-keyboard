/**
 * Written for and tested on a esp32doit-devkit-v1 with platformIO.
 * @todo report actual battery level instead of hardcoding 100%
 */

/**
 * Based on https://gist.github.com/manuelbl/66f059effc8a7be148adb1f104666467
 * with heavy modifications.
 */

#define US_KEYBOARD 1

#include <Arduino.h>
#include "BLEDevice.h"
#include "BLEHIDDevice.h"
#include "HIDTypes.h"
#include "HIDKeyboardTypes.h"

// Change the below values if desired
#define DEVICE_NAME "ESP32 Keyboard"
#define DEVICE_MANUFACTURER "FOSS Community"
#define STATUS_LED 18
const uint8_t MATRIX_ROWS[] = {25, 33, 32, 35, 34};
const uint8_t MATRIX_COLS[] = {13, 12, 14, 27, 26};

// [row][col]
const char KEY_MAP[5][5] = {
    // (assuming qwerty layout for plove)
    // sorry these are in such a strange order
    {'p', 'o', 'i', 'u', 'y'},
    {';', 'l', 'k', 'j', 'h'},
    {'t', 'r', 'e', 'w', 'q'},
    {'g', 'f', 'd', 's', 'a'},
    { 0,  'm', 'n', 'v', 'c'},
};

#define STATUS_STARTING 0
#define STATUS_DISCONNECTED 1
#define STATUS_CONNECTED 2
uint8_t status = STATUS_STARTING;

// Message (report) sent when a key is pressed or released
struct InputReport {
    uint8_t modifiers;	     // bitmask: CTRL = 1, SHIFT = 2, ALT = 4
    uint8_t reserved;        // must be 0
    uint8_t pressedKeys[31]; // up to 31 concurrently pressed keys
};

// Message (report) received when an LED's state changed
struct OutputReport {
    uint8_t leds;            // bitmask: num lock = 1, caps lock = 2, scroll lock = 4, compose = 8, kana = 16
};

// The report map describes the HID device (a keyboard in this case) and
// the messages (reports in HID terms) sent and received.
static const uint8_t REPORT_MAP[] = {
    USAGE_PAGE(1),      0x01,       // Generic Desktop Controls
    USAGE(1),           0x06,       // Keyboard
    COLLECTION(1),      0x01,       // Application
    REPORT_ID(1),       0x01,       //   Report ID (1)
    USAGE_PAGE(1),      0x07,       //   Keyboard/Keypad
    USAGE_MINIMUM(1),   0xE0,       //   Keyboard Left Control
    USAGE_MAXIMUM(1),   0xE7,       //   Keyboard Right Control
    LOGICAL_MINIMUM(1), 0x00,       //   Each bit is either 0 or 1
    LOGICAL_MAXIMUM(1), 0x01,
    REPORT_COUNT(1),    0x08,       //   8 bits for the modifier keys
    REPORT_SIZE(1),     0x01,
    HIDINPUT(1),        0x02,       //   Data, Var, Abs
    REPORT_COUNT(1),    0x01,       //   1 byte (unused)
    REPORT_SIZE(1),     0x08,
    HIDINPUT(1),        0x01,       //   Const, Array, Abs
    REPORT_COUNT(1),    0x1F,       //   31 bytes (for up to 31 concurrently pressed keys)
    REPORT_SIZE(1),     0x08,
    LOGICAL_MINIMUM(1), 0x00,
    LOGICAL_MAXIMUM(1), 0x65,       //   101 keys
    USAGE_MINIMUM(1),   0x00,
    USAGE_MAXIMUM(1),   0x65,
    HIDINPUT(1),        0x00,       //   Data, Array, Abs
    REPORT_COUNT(1),    0x05,       //   5 bits (Num lock, Caps lock, Scroll lock, Compose, Kana)
    REPORT_SIZE(1),     0x01,
    USAGE_PAGE(1),      0x08,       //   LEDs
    USAGE_MINIMUM(1),   0x01,       //   Num Lock
    USAGE_MAXIMUM(1),   0x05,       //   Kana
    LOGICAL_MINIMUM(1), 0x00,
    LOGICAL_MAXIMUM(1), 0x01,
    HIDOUTPUT(1),       0x02,       //   Data, Var, Abs
    REPORT_COUNT(1),    0x01,       //   3 bits (Padding)
    REPORT_SIZE(1),     0x03,
    HIDOUTPUT(1),       0x01,       //   Const, Array, Abs
    END_COLLECTION(0)               // End application collection
};

BLEHIDDevice* hid;
BLECharacteristic* input;
BLECharacteristic* output;

const InputReport NO_KEY_PRESSED = {};

/*
 * Callbacks related to BLE connection
 */
class BleKeyboardCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* server) {
        // Allow notifications for characteristics
        BLE2902* cccDesc = (BLE2902*)input->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
        cccDesc->setNotifications(true);

        // Serial.println("Client has connected");
        status = STATUS_CONNECTED;
    }

    void onDisconnect(BLEServer* server) {
        // Disallow notifications for characteristics
        BLE2902* cccDesc = (BLE2902*)input->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
        cccDesc->setNotifications(false);

        // Serial.println("Client has disconnected");
        status = STATUS_DISCONNECTED;
    }
};

/*
 * Called when the client (computer, smart phone) wants to turn on or off
 * the LEDs in the keyboard.
 *
 * bit 0 - NUM LOCK
 * bit 1 - CAPS LOCK
 * bit 2 - SCROLL LOCK
 */
class OutputCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* characteristic) {
        // OutputReport* report = (OutputReport*) characteristic->getData();
        // Serial.print("LED state: ");
        // Serial.print((int) report->leds);
        // Serial.println();
    }
};

void bluetoothTask(void*) {
    // initialize the device
    BLEDevice::init(DEVICE_NAME);
    BLEServer* server = BLEDevice::createServer();
    server->setCallbacks(new BleKeyboardCallbacks());

    // create an HID device
    hid = new BLEHIDDevice(server);
    input = hid->inputReport(1); // report ID
    output = hid->outputReport(1); // report ID
    output->setCallbacks(new OutputCallbacks());

    // set manufacturer name
    hid->manufacturer()->setValue(DEVICE_MANUFACTURER);
    // set USB vendor and product ID
    hid->pnp(0x02, 0xe502, 0xa111, 0x0210);
    // information about HID device: device is not localized, device can be connected
    hid->hidInfo(0x00, 0x02);

    // Security: device requires bonding
    BLESecurity* security = new BLESecurity();
    security->setAuthenticationMode(ESP_LE_AUTH_BOND);

    // set report map
    hid->reportMap((uint8_t*)REPORT_MAP, sizeof(REPORT_MAP));
    hid->startServices();

    // set battery level to 100%
    hid->setBatteryLevel(100);

    // advertise the services
    BLEAdvertising* advertising = server->getAdvertising();
    advertising->setAppearance(HID_KEYBOARD);
    advertising->addServiceUUID(hid->hidService()->getUUID());
    advertising->addServiceUUID(hid->deviceInfo()->getUUID());
    advertising->addServiceUUID(hid->batteryService()->getUUID());
    advertising->start();

    status = STATUS_DISCONNECTED;
    delay(portMAX_DELAY);
};

// increments once per POLLING_DUR
uint16_t led_timer = 0;
/**
 * Updates the status led
 */
void updateLed() {
    led_timer++;
    if (status == STATUS_CONNECTED) {
        digitalWrite(STATUS_LED, 1);
    } else if (status == STATUS_DISCONNECTED) {
        digitalWrite(STATUS_LED, (led_timer & (1 << 11)) > 0);
    } else {
        digitalWrite(STATUS_LED, (led_timer & (1 << 10)) > 0);
    }
}

// keyStates and keyStatesPrev are debounced
bool keyStates[5][5];
bool keyStatesPrev[5][5];
int8_t keyStatesBounce[5][5];
#define POLLING_DUR 200 // microseconds
#define KEY_DEBOUNCE 5000 // microseconds

void updateKeyData() {
    memcpy(&keyStatesPrev, &keyStates, sizeof(keyStates));

    for (int8_t col = 0; col < 5; col++) {
        digitalWrite(MATRIX_COLS[col], 1);

        for (int8_t row = 0; row < 5; row++) {
            if (digitalRead(MATRIX_ROWS[row])) {
                keyStatesBounce[row][col]++;
            } else {
                keyStatesBounce[row][col]--;
            }

            if (keyStatesBounce[row][col] > KEY_DEBOUNCE / POLLING_DUR) {
                keyStatesBounce[row][col] = KEY_DEBOUNCE / POLLING_DUR;
            } else if (keyStatesBounce[row][col] < 0) {
                keyStatesBounce[row][col] = 0;
            }

            if (keyStatesBounce[row][col] == KEY_DEBOUNCE / POLLING_DUR) {
                keyStates[row][col] = true;
            } else if (keyStatesBounce[row][col] == 0) {
                keyStates[row][col] = false;
            }
        }

        digitalWrite(MATRIX_COLS[col], 0);
    }
}

void sendUpdatesIfNeeded() {
    // needed?
    if (memcmp(&keyStates, &keyStatesPrev, sizeof(keyStates)) == 0) return;

    // create input report
    InputReport report = {
        .modifiers = 0,
        .reserved = 0,
        .pressedKeys = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    };

    uint_fast8_t pressedKeysIndex = 0;
    for (int8_t col = 0; col < 5; col++) {
        for (int8_t row = 0; row < 5; row++) {
            if (keyStates[row][col]) {
                if (KEY_MAP[row][col] != 0)
                report.pressedKeys[pressedKeysIndex] = keymap[KEY_MAP[row][col]].usage;
                pressedKeysIndex++;
            }
        }
    }

    // send the input report
    input->setValue((uint8_t*) &report, sizeof(report));
    input->notify();
}

void setup() {
    Serial.begin(115200);

    // clear states
    status = STATUS_STARTING;
    memset(&keyStates, 0, sizeof(keyStates));
    memset(&keyStatesPrev, 0, sizeof(keyStatesPrev));
    memset(&keyStatesBounce, 0, sizeof(keyStatesBounce));

    // configure pins
    pinMode(STATUS_LED, OUTPUT);
    for (int8_t i = 0; i < 5; i++) {
        pinMode(MATRIX_ROWS[i], INPUT_PULLDOWN);
        pinMode(MATRIX_COLS[i], OUTPUT);
    }

    // start Bluetooth task
    xTaskCreate(bluetoothTask, "bluetooth", 20000, NULL, 5, NULL);
}

void loop() {
    updateLed();
    updateKeyData();
    sendUpdatesIfNeeded();

    delayMicroseconds(POLLING_DUR);
}
