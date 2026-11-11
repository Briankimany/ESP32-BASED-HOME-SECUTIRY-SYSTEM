// camera_handler.cpp – QQVGA first, fallback to QQQVGA with timeouts
#include "camera_handler.h"
#include "OV7670.h"
#include "BMP.h"
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClient.h>

// ---------------------------------------------------------------------------
// Pin configuration
// ---------------------------------------------------------------------------
const int SIOD  = 21;
const int SIOC  = 22;
const int VSYNC = 36;
const int HREF  = 26;
const int XCLK  = 27;
const int PCLK  = 39;
const int D0    = 15;
const int D1    = 33;
const int D2    = 13;
const int D3    = 32;
const int D4    = 16;
const int D5    = 35;
const int D6    = 14;
const int D7    = 34;

// ---------------------------------------------------------------------------
// WiFi / Endpoint
// ---------------------------------------------------------------------------
#define WIFI_SSID     "mol"
#define WIFI_PASSWORD "Linxroot1"

const char* endpointHost = "10.149.83.58";
const int   endpointPort = 5000;
const char* endpointPath = "/upload";

// ---------------------------------------------------------------------------
// Globals
// ---------------------------------------------------------------------------
WiFiMulti wifiMulti;
OV7670*   camera             = nullptr;
static bool     camera_initialized = false;
static uint16_t frame_width        = 0;
static uint16_t frame_height       = 0;
static size_t   frame_bytes        = 0;
static unsigned char bmpHeader[BMP::headerSize];

// ---------------------------------------------------------------------------
// printMemoryInfo
// ---------------------------------------------------------------------------
void printMemoryInfo() {
    Serial.println("\n=== MEMORY INFO ===");
    Serial.printf("Free Heap:          %d bytes (%d KB)\n",
                  ESP.getFreeHeap(), ESP.getFreeHeap() / 1024);
    Serial.printf("Min Free Heap:      %d bytes\n", ESP.getMinFreeHeap());
    Serial.printf("Total Heap:         %d bytes (%d KB)\n",
                  ESP.getHeapSize(), ESP.getHeapSize() / 1024);
    Serial.printf("Largest free block: %d bytes (%d KB)\n",
                  heap_caps_get_largest_free_block(MALLOC_CAP_8BIT),
                  heap_caps_get_largest_free_block(MALLOC_CAP_8BIT) / 1024);
    Serial.println("===================");
}

// ---------------------------------------------------------------------------
// createCamera – tries a given mode and returns true if successful
// ---------------------------------------------------------------------------
static bool createCamera(OV7670::Mode mode, const char* modeName) {
    Serial.printf("Attempting %s...\n", modeName);
    camera = new OV7670(mode, SIOD, SIOC, VSYNC, HREF, XCLK, PCLK,
                        D0, D1, D2, D3, D4, D5, D6, D7);
    if (camera == nullptr || camera->xres == 0) {
        Serial.println("  Constructor failed (no VSYNC or I2C error).");
        return false;
    }
    // If the constructor succeeded but the sensor didn't produce a frame,
    // we'll detect that later in capture. For now, assume it's okay.
    return true;
}

// ---------------------------------------------------------------------------
// init_camera – tries QQVGA first, then QQQVGA
// ---------------------------------------------------------------------------
esp_err_t init_camera() {
    Serial.println("Initializing camera (trying QQVGA first)...");
    printMemoryInfo();

    if (createCamera(OV7670::Mode::QQVGA_RGB565, "QQVGA (160x120)")) {
        // Successfully created the object – now capture a test frame to see if it works
        // (we need to call oneFrame and check if it times out or returns valid data)
        // Since we added timeouts to the library, oneFrame() will return false on timeout.
        // We'll test with a short capture.
        Serial.println("Testing QQVGA capture...");
        bool ok = camera->oneFrame();   // should return bool if we modified the library
        if (ok && camera->frame != nullptr && camera->xres > 0) {
            Serial.println("QQVGA works!");
            frame_width  = camera->xres;
            frame_height = camera->yres;
            frame_bytes  = (size_t)frame_width * frame_height * 2;
            BMP::construct16BitHeader(bmpHeader, frame_width, frame_height);
            camera_initialized = true;
            Serial.printf("Camera ready: %u x %u = %zu bytes\n",
                          frame_width, frame_height, frame_bytes);
            printMemoryInfo();
            return ESP_OK;
        } else {
            Serial.println("QQVGA capture failed (timeout or zero frame).");
            delete camera;
            camera = nullptr;
        }
    }

    // Fallback to QQQVGA
    Serial.println("Falling back to QQQVGA (80x60)...");
    if (createCamera(OV7670::Mode::QQQVGA_RGB565, "QQQVGA (80x60)")) {
        // QQQVGA is known to work, so we assume it succeeds
        frame_width  = camera->xres;
        frame_height = camera->yres;
        frame_bytes  = (size_t)frame_width * frame_height * 2;
        BMP::construct16BitHeader(bmpHeader, frame_width, frame_height);
        camera_initialized = true;
        Serial.printf("Camera ready: %u x %u = %zu bytes\n",
                      frame_width, frame_height, frame_bytes);
        printMemoryInfo();
        return ESP_OK;
    }

    Serial.println("All resolutions failed – check wiring/power.");
    return ESP_FAIL;
}

// ---------------------------------------------------------------------------
// captureAndSend – direct send, no staging buffer
// ---------------------------------------------------------------------------
static bool captureAndSend() {
    if (!camera_initialized) {
        Serial.println("Camera not initialized!");
        return false;
    }

    // 1. Stop WiFi
    Serial.println("Pausing WiFi for clean capture...");
    WiFi.disconnect(false);
    delay(200);

    // 2. Capture (uses the library's timeout – will return false if no VSYNC)
    Serial.println("Capturing frame...");
    unsigned long startCap = millis();
    bool success = camera->oneFrame();   // now returns bool after timeout
    if (!success) {
        Serial.println("Capture failed – no VSYNC within timeout.");
        wifiMulti.run(); // try to reconnect before returning
        return false;
    }
    Serial.printf("Captured %u x %u = %zu bytes (took %lu ms)\n",
                  frame_width, frame_height, frame_bytes, millis() - startCap);
    Serial.printf("Free heap after capture: %d bytes\n", ESP.getFreeHeap());

    // 3. Reconnect WiFi with timeout
    Serial.println("Reconnecting WiFi...");
    unsigned long startReconnect = millis();
    while (wifiMulti.run() != WL_CONNECTED) {
        if (millis() - startReconnect > 10000) {
            Serial.println("\n✗ WiFi reconnect timed out!");
            return false;
        }
        delay(300);
        Serial.print(".");
    }
    Serial.println("\n✓ WiFi reconnected.");

    // 4. Send image
    WiFiClient client;
    if (!client.connect(endpointHost, endpointPort)) {
        Serial.println("Connection to endpoint failed!");
        return false;
    }

    const String boundary   = "----ESP32Boundary";
    const String partHeader = "--" + boundary + "\r\n"
                              "Content-Disposition: form-data; name=\"image\"; filename=\"image.bmp\"\r\n"
                              "Content-Type: image/bmp\r\n\r\n";
    const String partFooter = "\r\n--" + boundary + "--\r\n";

    int contentLength = partHeader.length() + BMP::headerSize + frame_bytes + partFooter.length();

    client.println("POST " + String(endpointPath) + " HTTP/1.1");
    client.println("Host: " + String(endpointHost));
    client.println("Content-Type: multipart/form-data; boundary=" + boundary);
    client.print("Content-Length: ");
    client.println(contentLength);
    client.println();

    client.print(partHeader);
    client.write(bmpHeader, BMP::headerSize);
    client.write((const uint8_t*)camera->frame, frame_bytes);
    client.print(partFooter);

    unsigned long startResponse = millis();
    while (client.available() == 0) {
        if (millis() - startResponse > 5000) {
            Serial.println("Timeout waiting for server response!");
            client.stop();
            return false;
        }
    }
    while (client.available()) {
        String line = client.readStringUntil('\r');
        Serial.println(line);
    }

    client.stop();
    Serial.println("✓ Image sent successfully!");
    return true;
}

// ---------------------------------------------------------------------------
// sendImageToEndpoint  (public)
// ---------------------------------------------------------------------------
void sendImageToEndpoint() {
    if (!camera_initialized) {
        Serial.println("Camera not initialized!");
        return;
    }
    if (!captureAndSend()) {
        Serial.println("Capture/send failed – will retry next cycle.");
    }
}

// ---------------------------------------------------------------------------
// setupCameraAndWiFi (public)
// ---------------------------------------------------------------------------
void setupCameraAndWiFi() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n=== ESP32 Camera System (QQVGA with fallback) ===");
    Serial.println("No PSRAM - Internal RAM only");

    if (init_camera() != ESP_OK) {
        Serial.println("✗ Camera init failed – halting.");
        while (true) delay(1000);
    }

    wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
    Serial.println("\nConnecting to WiFi...");
    unsigned long startWiFi = millis();
    while (wifiMulti.run() != WL_CONNECTED) {
        if (millis() - startWiFi > 15000) {
            Serial.println("\n✗ WiFi connection timed out!");
            return;
        }
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n✓ WiFi connected!");
    Serial.print("  IP address: ");
    Serial.println(WiFi.localIP());

    printMemoryInfo();
    Serial.println("\n✓ System ready!");
}