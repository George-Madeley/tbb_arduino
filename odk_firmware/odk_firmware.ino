#undef pinMode
#include <MCP23017.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <stdint.h>
#include <ArduinoBLE.h>

#include "odk_gps.h"
#include "odk_ble.h"
#include "odk_temp_sensor.h"
#include "odk_battery.h"

extern TinyGPSPlus gps;
extern BLECharacteristic anlgCh0Chstic;
extern BLECharacteristic anlgCh1Chstic;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_MOSI  38
#define OLED_CLK   48
#define OLED_DC    18
#define OLED_CS    21
#define OLED_RESET 17

#define GPS_RX D1
#define GPS_TX D0

#define SR_DATA  7
#define SR_LATCH 9
#define SR_CLOCK 8

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
  OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

uint16_t analogue_0 = 0;  // variable to store the value coming from the sensor
uint16_t analogue_1 = 0;
float temperature = 0;

#define ODK_SPLASH_HEIGHT 64
#define ODK_SPLASH_WIDTH 128

// array size is 1024
static const uint8_t PROGMEM ODK_splash[] = {
    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b01110000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b11111000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b11111100, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 0b00000000, 0b00000001, 0b11111110, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 0b00000000, 0b00000011, 0b11111110, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 0b00000000, 0b00000111, 0b11111111, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 0b00000000, 0b00001111, 0b11111111, 0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 0b00000000, 0b00011111, 0b11111111, 0b11000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 0b00000000, 0b00011111, 0b11111111, 0b11100000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 0b00000000, 0b00111111, 0b11111111, 0b11110000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 0b00000000, 0b01111111, 0b11111111, 0b11111000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 0b00000000, 0b11111111, 0b11011111, 0b11111000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 0b00000001, 0b11111111, 0b10001111, 0b11111100, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 0b00000011, 0b11111111, 0b00000111, 0b11111110, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 0b00000111, 0b11111110, 0b00000011, 0b11111111, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 0b00000111, 0b11111110, 0b00000001, 0b11111111, 0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 0b00001111, 0b11111100, 0b00000000, 0b11111111, 0b11000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 0b00011111, 0b11111000, 0b00000000, 0b11111111, 0b11000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 0b00111111, 0b11110000, 0b00000000, 0b01111111, 0b11100000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 0b01111111, 0b11100000, 0b00000000, 0b00111111, 0b11110000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00011100, 0b01111111, 0b11000000, 0b00000000, 0b00011111, 0b11111001, 0b11000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00111111, 0b11111111, 0b11000000, 0b00000000, 0b00001111, 0b11111111, 0b11100000, 0b00000000, 0b01111110, 0b00000000, 0b11111111, 0b00000000, 0b01111000, 0b00111110, 0b00000000, 
    0b00000000, 0b00111111, 0b11111111, 0b10000000, 0b00000000, 0b00001111, 0b11111111, 0b11100000, 0b00000001, 0b11111111, 0b10000000, 0b11111111, 0b11000000, 0b01111000, 0b01111100, 0b00000000, 
    0b00000000, 0b01111111, 0b11111111, 0b00000000, 0b00000000, 0b00011111, 0b11111111, 0b11110000, 0b00000011, 0b11111111, 0b11000000, 0b11111111, 0b11100000, 0b01111000, 0b11111000, 0b00000000, 
    0b00000000, 0b01111111, 0b11111110, 0b00000000, 0b00000000, 0b00001111, 0b11111111, 0b11110000, 0b00000111, 0b11111111, 0b11000001, 0b11111111, 0b11100000, 0b11111001, 0b11110000, 0b00000000, 
    0b00000000, 0b01111111, 0b11111100, 0b00000000, 0b00000000, 0b00000111, 0b11111111, 0b11111000, 0b00001111, 0b11000011, 0b11100001, 0b11110001, 0b11110000, 0b11111011, 0b11100000, 0b00000000, 
    0b00000000, 0b11111111, 0b11111100, 0b00000000, 0b00000000, 0b00000011, 0b11111111, 0b11111000, 0b00001111, 0b10000011, 0b11100001, 0b11100001, 0b11110000, 0b11110011, 0b11100000, 0b00000000, 
    0b00000000, 0b01111111, 0b11111000, 0b00000000, 0b00000000, 0b00000011, 0b11111111, 0b11110000, 0b00011111, 0b00000011, 0b11100001, 0b11100001, 0b11110000, 0b11110111, 0b11000000, 0b00000000, 
    0b00000000, 0b00111111, 0b11111000, 0b00000000, 0b00000000, 0b00000001, 0b11111111, 0b11100000, 0b00011111, 0b00000011, 0b11100011, 0b11100001, 0b11110001, 0b11111111, 0b10000000, 0b00000000, 
    0b00000000, 0b00001111, 0b11111000, 0b00000000, 0b00000000, 0b00000001, 0b11111111, 0b10000000, 0b00011111, 0b00000011, 0b11100011, 0b11100001, 0b11110001, 0b11111111, 0b10000000, 0b00000000, 
    0b00000000, 0b00000111, 0b11110000, 0b00000000, 0b00000000, 0b00000000, 0b11111111, 0b10000000, 0b00011110, 0b00000011, 0b11000011, 0b11100001, 0b11110001, 0b11111111, 0b10000000, 0b00000000, 
    0b00000000, 0b00001111, 0b11110000, 0b00000000, 0b00000000, 0b00000000, 0b11111111, 0b10000000, 0b00011110, 0b00000011, 0b11000011, 0b11000001, 0b11100001, 0b11111111, 0b10000000, 0b00000000, 
    0b00000000, 0b00001111, 0b11110000, 0b00000000, 0b00000000, 0b00000110, 0b11111111, 0b10000000, 0b00011110, 0b00000111, 0b11000011, 0b11000011, 0b11100001, 0b11100111, 0b11000000, 0b00000000, 
    0b00000000, 0b00001111, 0b11110000, 0b00000000, 0b00000000, 0b00001110, 0b01111111, 0b10000000, 0b00011110, 0b00000111, 0b10000111, 0b11000011, 0b11100011, 0b11100111, 0b11000000, 0b00000000, 
    0b00000000, 0b00001111, 0b11110000, 0b00000000, 0b00000000, 0b00001110, 0b01111111, 0b10000000, 0b00011111, 0b00001111, 0b10000111, 0b11001111, 0b11000011, 0b11100111, 0b11000000, 0b00000000, 
    0b00000000, 0b00001111, 0b11110000, 0b00000000, 0b00000000, 0b00001110, 0b01111111, 0b10000000, 0b00011111, 0b11111111, 0b00000111, 0b11111111, 0b10000011, 0b11000011, 0b11100000, 0b00000000, 
    0b00000000, 0b00000111, 0b11110000, 0b00000000, 0b00000000, 0b00001100, 0b01111111, 0b10000000, 0b00001111, 0b11111110, 0b00000111, 0b11111111, 0b00000011, 0b11000011, 0b11100000, 0b00000000, 
    0b00000000, 0b00001111, 0b11110000, 0b00000000, 0b00000000, 0b00011100, 0b11111111, 0b10000000, 0b00000111, 0b11111100, 0b00000111, 0b11111110, 0b00000011, 0b11000011, 0b11100000, 0b00000000, 
    0b00000000, 0b00011111, 0b11111000, 0b00000000, 0b00000000, 0b00011100, 0b11111111, 0b11100000, 0b00000001, 0b11110000, 0b00000111, 0b11110000, 0b00000011, 0b11000001, 0b11100000, 0b00000000, 
    0b00000000, 0b01111111, 0b11111000, 0b00000000, 0b00000000, 0b00111000, 0b11111111, 0b11110000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b11111111, 0b11111100, 0b00000000, 0b00000000, 0b01110000, 0b11111111, 0b11111000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b01111111, 0b11111100, 0b00000000, 0b00000000, 0b11110001, 0b11111111, 0b11111000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b01111111, 0b11111110, 0b00000000, 0b00000011, 0b11100011, 0b11111111, 0b11110000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b01111111, 0b11111111, 0b00000000, 0b00110011, 0b10000011, 0b11111111, 0b11110000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00111111, 0b11111111, 0b10000000, 0b01111001, 0b00001111, 0b11111111, 0b11110000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00111111, 0b11111111, 0b11000000, 0b01110000, 0b00011111, 0b11111111, 0b11100000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00011110, 0b11111111, 0b11110000, 0b00000000, 0b00111111, 0b11111011, 0b11100000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 0b01111111, 0b11111100, 0b00000001, 0b11111111, 0b11110000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 0b00111111, 0b11111111, 0b11111111, 0b11111111, 0b11100000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 0b00011111, 0b11111111, 0b11111111, 0b11111111, 0b11000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 0b00001111, 0b11111111, 0b11111111, 0b11111111, 0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 0b00000111, 0b11111111, 0b11111111, 0b11111111, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 0b00000011, 0b11111111, 0b11111111, 0b11111110, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 0b00000011, 0b11111111, 0b11111111, 0b11111111, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 0b00000011, 0b11111111, 0b11111111, 0b11111111, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 0b00000111, 0b11111111, 0b11111111, 0b11111111, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 0b00000111, 0b11111110, 0b00000001, 0b11111111, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 0b00000011, 0b11111100, 0b00000000, 0b11111111, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 0b00000000, 0b11111000, 0b00000000, 0b11111100, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 0b00000000, 0b00110000, 0b00000000, 0b01100000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 
    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000
};

#define MCP23017_ADDR 0x20
MCP23017 mcp = MCP23017(MCP23017_ADDR);

void setup() {

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if(!display.begin(SSD1306_SWITCHCAPVCC)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;); // Don't proceed, loop forever
    }
    display.clearDisplay();
    display.drawBitmap(0,0,ODK_splash, ODK_SPLASH_WIDTH, ODK_SPLASH_HEIGHT,1);
    display.display();
	for (int i = 0; i < 8; i ++)
	{
		delay(100);
		display.invertDisplay(1);
		delay(100);
		display.invertDisplay(0);
	}

    Wire.begin();
    Serial.begin(115200);
    
    mcp.init();
    mcp.portMode(MCP23017Port::A, 0);          //Port A as output
    mcp.portMode(MCP23017Port::B, 0b11111111); //Port B as input

    mcp.writeRegister(MCP23017Register::GPIO_A, 0x00);    //Reset port A 
    mcp.writeRegister(MCP23017Register::GPIO_B, 0x00);    //Reset port B

    // GPIO_B reflects the same logic as the input pins state
    mcp.writeRegister(MCP23017Register::IPOL_B, 0x00);
    // Uncomment this line to invert inputs (press a button to lit a led)
    //mcp.writeRegister(MCP23017Register::IPOL_B, 0xFF);

    display.setTextSize(1);      // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE); // Draw white text

    srInit();

    wire_init();
    ble_init();
	gps_init();
	gps_poll(true);
}

uint8_t mux_sel= 0;
uint8_t mux_sel_r = 0;
uint8_t mux_sel_out = 0;
void loop() {
    uint8_t currentB;
    currentB = mcp.readPort(MCP23017Port::B);
    //Serial.println(currentB,BIN);

	gps_poll();
	ble_poll();

    srDriveAllBits(~currentB);
    if (mux_sel < 8) {
        if (0x01 & ~currentB) {
            mux_sel += 1;
            delay(250);
        }
    } else {
        mux_sel = 0;
    }

    mux_sel_r = reverseBits(mux_sel,3);
    mcp.writeRegister(MCP23017Register::GPIO_A, mux_sel_r);  //Reset port A 


	char anlg0string[5] = "9999";
	char anlg1string[5] = "9999";
    analogue_0 = analogRead(1);
    analogue_1 = analogRead(2);
    temperature = get_temp();
	sprintf(anlg0string, "%4d",analogue_0);
	sprintf(anlg1string, "%4d",analogue_1);
	anlgCh0Chstic.writeValue((byte*)anlg0string, 4);
	anlgCh1Chstic.writeValue((byte*)anlg1string, 4);

    uint16_t batLevel = read_bat_reg(BAT_VCELL);
    uint16_t batDis = read_bat_reg(BAT_CRATE);

    display.clearDisplay();
    display.setCursor(0, 0);     // Start at top-left corner
    display.print(F("Battery level: ")); display.println((float)batLevel * 0.0001559892211404729);
    display.print(F("Battery Dscge: ")); display.print((float)batDis * 0.208); display.println("%");
	display.print(gps.date.year());display.print(F("/"));display.print(gps.date.month());display.print(F("/"));display.print(gps.date.day());display.print(F("  "));display.print(gps.time.hour());display.print(F(":"));display.print(gps.time.minute());display.print(F(":"));display.println(gps.time.second());
	display.print(F("Lat: ")); display.print(gps.location.lat()); display.print(F(", Lon: ")); display.println(gps.location.lng());
	display.println();
    display.print(F("Analogue Port: ")); display.println(mux_sel);
    display.print(F("A0: ")); display.print(analogue_0); display.print(", ");
    display.print(F("A1: ")); display.println(analogue_1);
    display.print(F("Temp: ")); display.println(temperature);
    display.display();
    // uint8_t bits = 0x01;
    // for (int i=0; i<8; i++) {
    //     bang_those_bits(bits);
    //     bits = bits << 1;
    //     delay(1000);
    // }

if (Serial.available() > 0) {
    // Read the incoming string from the serial input
    String inputStr = Serial.readStringUntil('\n');  // Read the input as a string
    inputStr.trim();  // Remove any leading/trailing spaces or newline characters

    int spaceIndex = inputStr.indexOf(' ');  // Find the space character (if any)
    
    if (spaceIndex == -1) {
        // No space found, perform a read operation
        long address = strtol(inputStr.c_str(), NULL, 16);  // Convert the string address to an integer (hex format)
        Serial.print("Value at "); 
        Serial.print((uint8_t)address); 
        Serial.print(" = "); 
        Serial.println(read_bat_reg((uint8_t)address));  // Call the read function
    } else {
        // Space found, perform a write operation
        String addressStr = inputStr.substring(0, spaceIndex);  // Extract the address part
        String dataStr = inputStr.substring(spaceIndex + 1);  // Extract the data part

        addressStr.trim();
        dataStr.trim();

        long address = strtol(addressStr.c_str(), NULL, 16);  // Convert the address to an integer (hex format)
        long data = strtol(dataStr.c_str(), NULL, 16);  // Convert the data to an integer (hex format)

        write_bat_reg((uint8_t)address, (uint16_t)data);  // Call the write function
        
        Serial.print("Wrote "); 
        Serial.print((uint16_t)data, HEX); 
        Serial.print(" to address "); 
        Serial.println((uint8_t)address, HEX);
    }
}

}

void srInit() {
    // set pins to output
    pinMode(SR_DATA, OUTPUT);
    pinMode(SR_LATCH, OUTPUT);
    pinMode(SR_CLOCK, OUTPUT);
    
    // drive all pins low
    digitalWrite(SR_DATA, LOW);
    digitalWrite(SR_LATCH, LOW);
    digitalWrite(SR_CLOCK, LOW);

    // clear the register 
    for (int i=0; i<8; i++) {
        digitalWrite(SR_CLOCK, HIGH);
        digitalWrite(SR_CLOCK, LOW);
    }

    // latch cleared register
    digitalWrite(SR_LATCH, HIGH);
    digitalWrite(SR_LATCH, LOW);
}


uint8_t reverseBits(uint8_t num, uint8_t bitSize) {
    uint8_t reversed = 0;

    // Loop through each bit in the number
    for (uint8_t i = 0; i < bitSize; i++) {
        // Extract the least significant bit of `num` and shift it to its reversed position
        if (num & (1 << i)) {
            reversed |= (1 << (bitSize - 1 - i));
        }
    }
    
    return reversed;
}

void srDriveAllBits (uint8_t bits) {
    // drive the entire bitstring in 'bits' to the shift register

    uint8_t top_bit;
    for (int i=0; i<8; i++) {
        // get the top bit
        top_bit = bits & 0x80;
        // shift the sequence left
        bits = bits << 1;
        // set the data either high or low based on the top bit
        if (top_bit) {
            digitalWrite(SR_DATA, HIGH);
        } else {
            digitalWrite(SR_DATA, LOW);
        }
        // clock bit into register
        digitalWrite(SR_CLOCK, HIGH);
        digitalWrite(SR_CLOCK, LOW);
    }
    // latch the final sequence
    digitalWrite(SR_LATCH, HIGH);
    digitalWrite(SR_LATCH, LOW);
}
