#include <Adafruit_SHT31.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include "ThingSpeak.h"

// Pin Definitions
#define PH_UP_PUMP_PIN D12
#define PH_DOWN_PUMP_PIN D11
#define NUTRIENT_PUMP_PIN D10
#define WATER_LEVEL_PIN D3
#define ONE_WIRE_BUS D2
#define OLED_ADDRESS 0x3C

// Set the ThingSpeak channel and API key information
unsigned long myChannelNumber = 2206865;
const char* myWriteAPIKey = "OMHDEO8JY8ORDAN2";

// Set the WiFi network credentials
const char* ssid = "TP-Link_905D"; // your wifi SSID name
const char* password = "33072036"; // wifi password

// Set the ThingSpeak server address
const char* server = "api.thingspeak.com";

// Create a WiFiClient object to connect to the WiFi network
WiFiClient client;

// Set the time to wait between uploading data to ThingSpeak (in milliseconds)
int wait_between_uploads = 10000; // 10 seconds

// Constants
const float EC_TARGET = 1.5;        // Target EC value in dS/m
const float EC_TOLERANCE = 0.1;     // Tolerance for EC adjustment

// Calibration constants for conversion from TDS to EC
const int tdsPin = A2;
const float tdsToEcRatio = 1 / 640.0; // Conversion ratio from TDS to EC (adjust according to your calibration)

// Objects
Adafruit_SHT31 sht31 = Adafruit_SHT31();
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R2, /* reset=*/U8X8_PIN_NONE);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Variables
float airTemp = 0.0;
float waterTemp = 0.0;
float pHValue = 0.0;
bool waterFull = false; // Flag to indicate if the water is full
float ecValue = 0.0;
float tdsPPM = 0.0;
bool pHAdjustmentInProgress = false;
bool ecAdjustmentInProgress = false;

void setup() {
  pinMode(PH_UP_PUMP_PIN, OUTPUT);
  pinMode(PH_DOWN_PUMP_PIN, OUTPUT);
  pinMode(NUTRIENT_PUMP_PIN, OUTPUT);
  pinMode(WATER_LEVEL_PIN, INPUT);
  u8g2.begin();
  u8g2.setFlipMode(true); // Flip the display vertically if needed

  Serial.begin(9600);

  if (!sht31.begin(0x44)) {
    Serial.println("Could not find SHT31 sensor!");
    while (1);
  }
  // Start the serial communication at 112500 baud
 // Serial.begin(112500);

  // Disconnect any previous WiFi connection
  WiFi.disconnect();
  delay(10);

  // Connect to the WiFi network
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
    Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("ESP32 connected to WiFi: ");
  Serial.println(ssid);
  Serial.println();

  // Initialize the ThingSpeak library with the WiFi client
  ThingSpeak.begin(client);
    
  }

//  sensors.begin();
//}

void loop() {
  measureAirTemp();
  measureWaterTemp();
  measurePH();
  checkWaterLevel();
  measureEC();

  adjustPH();
  adjustEC();

  updateDisplay();

  delay(500); // Delay for 500 milliseconds

  // Set the values to be sent to ThingSpeak
  ThingSpeak.setField(1, airTemp);
  ThingSpeak.setField(2, waterTemp);
  ThingSpeak.setField(3, pHValue);
  ThingSpeak.setField(4, ecValue);
  ThingSpeak.setField(5, waterFull);
  

  // Send the data to ThingSpeak
  ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  // Print a message to the serial monitor indicating that the data has been uploaded
  Serial.println("Uploaded to ThingSpeak server.");

  // Disconnect the WiFi client
  client.stop();

  // Wait for the specified amount of time before uploading the next set of data
  // thingspeak needs minimum 15 sec delay between updates on a free acount,
  // a paid acount can update every 1 sec
  Serial.println("Waiting to upload next reading...");
  Serial.println();
  
  delay(wait_between_uploads);
}

void measureAirTemp() {
  airTemp = sht31.readTemperature();
}

void measureWaterTemp() {
  sensors.requestTemperatures();
  waterTemp = sensors.getTempCByIndex(0);
}

void measurePH() {
  int value = analogRead(A0);
  float voltage = value * (3.3 / 4095.0);
  pHValue = voltage  + 0.75;
}

void checkWaterLevel() {
  int waterLevelValue = digitalRead(WATER_LEVEL_PIN);
  waterFull = (waterLevelValue == LOW);
}

void measureEC() {
  int tdsValue = analogRead(tdsPin);
  ecValue = tdsValue * tdsToEcRatio;
}

void adjustPH() {
  if (!pHAdjustmentInProgress) {
    if (pHValue < 5.7) {
      activatePump(PH_UP_PUMP_PIN);
      pHAdjustmentInProgress = true;
    } else if (pHValue > 6.2) {
      activatePump(PH_DOWN_PUMP_PIN);
      pHAdjustmentInProgress = true;
    }
  } else {
    if (pHValue >= 5.7 && pHValue <= 6.2) {
      stopPump(PH_UP_PUMP_PIN);
      stopPump(PH_DOWN_PUMP_PIN);
      pHAdjustmentInProgress = false;
    }
  }
}

void adjustEC() {
  if (!ecAdjustmentInProgress) {
    if (ecValue < EC_TARGET - EC_TOLERANCE) {
      activatePump(NUTRIENT_PUMP_PIN);
      ecAdjustmentInProgress = true;
    }   
    else {
    if (ecValue >= EC_TARGET + EC_TOLERANCE) {
      stopPump(NUTRIENT_PUMP_PIN);
      ecAdjustmentInProgress = false;
    }
  }
 }
}
void activatePump(int pumpPin) {
  digitalWrite(pumpPin, HIGH);
}

void stopPump(int pumpPin) {
  digitalWrite(pumpPin, LOW);
}

void updateDisplay() {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.setCursor(0, 20);
    u8g2.print("Air Temp: ");
    u8g2.print(airTemp);
    u8g2.print(" C");

    u8g2.setCursor(0, 40);
    u8g2.print("pH: ");
    u8g2.print(pHValue, 2);
    u8g2.print("     ");
    u8g2.setCursor(0, 60);
    u8g2.print("water Temp: ");
    u8g2.print(waterTemp, 2);
    u8g2.print("     ");
  } while (u8g2.nextPage());

  delay(2000); // Wait for 2 seconds before switching to the next screen

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.setCursor(0, 20);
    u8g2.print("Water: ");
    u8g2.print(waterFull ? "Full" : "Not Full");
    u8g2.setCursor(0, 40);
    u8g2.print("EC: ");
    u8g2.print(ecValue, 2);
    u8g2.print(" dS/m");
    u8g2.setCursor(0, 60);
    u8g2.print("EC Adjusment: ");
    u8g2.print(ecAdjustmentInProgress ? "yes" : "No") ;
  } while (u8g2.nextPage());

  delay(2000); // Wait for 2 seconds before switching back to the first screen
}

float convertToPPM(int analogValue) {
  // Perform your TDS value conversion here based on your calibration and measurement method
  // This will depend on the specific characteristics and calibration of your TDS meter
  // Please refer to the documentation or calibration instructions provided with your TDS meter

  // Sample conversion equation (replace with your calibration equation)
  float ppm = analogValue * 2.0; // Sample equation: multiply the analog value by 2.0

  return ppm;
}
