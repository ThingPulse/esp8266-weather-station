// Wifi connection details
#define WIFI_SSID ""
#define WIFI_PWD  ""
#define HOSTNAME "OneDayForecaster"

// OTA settings
#define OTA_PASSWORD ""

// SPI Pins
#define TFT_DC 5
#define TFT_CS 15
#define TFT_LED 16
#define SD_CS 4

// Met Office API
#define API_KEY ""
#define LOC_CODE 123456

// Time offset
#define UTC_OFFSET 1

// Application settings
const int UPDATE_INTERVAL_SECS = 5 * 60;      // Update every 5 minutes
const int TIME_BEFORE_SLEEP_SECS = 20 * 60;   // Sleep after 20 mins
