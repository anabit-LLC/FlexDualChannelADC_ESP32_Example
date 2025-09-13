/********************************************************************************************************
This example ESP32 Arduino sketch is meant to work with Anabit's Flex Dual Channel ADC open source reference design

Product link: https://anabit.co/products/flex-dual-channel-adc

This version is meant to work with an ESP32 based dev boards because it uses fast pin setting functions (faster than digitalwrite) to help get the max sample rate 
from the ADC. See the other Flex ADC example code sketch that is configured to work with any arduino board

The Flex Dual Channel ADC design features a two channel input ADC, signal routing is handled by integrated MUX. Why is it pseudo differential? This ADC provides 
14 bits of resolution and a voltage range defined by three optional precision voltage references: 2.5V, 3.3V, and 4.096. Be sure to set the correct reference value
below in the code constants (see "VREF_VOLTAGE" constant)

This sketch deomonstrats how to use the Flex ADC to make a single measurement or to make a group or burst of measurements as fast as possible. The single versus
burst mode is set by the "#define" MODE_SINGLE_MEASUREMENT or MODE_BURST_CAPTURE, comment out the mode you don't want to use

Please report any issue with the sketch to the Anagit forum: https://anabit.co/community/forum/analog-to-digital-converters-adcs
Example code developed by Your Anabit LLC © 2025
Licensed under the Apache License, Version 2.0.
**************************************************************************************************************/

#include <SPI.h>
#include "soc/gpio_struct.h"   // Gives access to GPIO.out_w1ts
#include "soc/io_mux_reg.h"

// === CONFIGURATION === uncomment one of the modes
#define MODE_SINGLE_MEASUREMENT
//#define MODE_BURST_CAPTURE

//sets some constants including chip select
#define CS_PIN         5
#define VREF_VOLTAGE   4.096f  //2.5f 3.3f set to vref value of your Flex 
#define ADC_BITS       16383.0f //14 bit ADC value (2^14 - 1)

//SPI Configuration: max SPI clock rate for this ADC is 40MHz, 
//ESP32 does not work well with Flex at low clock rates, recommend using 40MHz
SPISettings adsSettings(40000000, MSBFIRST, SPI_MODE0);  

//settings specific to burst mode, first variable sets the number of points measured in a burst
#if defined(MODE_BURST_CAPTURE)
const int NUM_SAMPLES = 1024;
int16_t adcRaw[NUM_SAMPLES];
float adcVoltage[NUM_SAMPLES];
#endif

void setup() {
  Serial.begin(115200);
  delay(2500); //delay to give time to get serial monitor or plotter setup

  // Setup chip select pin and start SPI communication
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  SPI.begin();

//code for single measurement mode, loops and prints out measured value 
//along with raw ADC code
#if defined(MODE_SINGLE_MEASUREMENT)
  Serial.println("Single Measurement Mode (CH0 via GPIO)");
  readADS7945(0xC000);  // Dummy read
  delayMicroseconds(5);
  while(1) {
    uint16_t usRaw = readADS7945(0xC000) >> 1; //one shift is specific for ESP32
    Serial.print("ADC Value unsigned hex: "); Serial.println(usRaw,HEX);
    Serial.print("ADC Value unsigned decimal: "); Serial.println(usRaw);
    float voltage = convertToVoltage(usRaw);
    Serial.print("Voltage: "); Serial.print(voltage, 4); Serial.println(" V");
    Serial.println();
    delay(2500);
  }

//code for burst mode. Grabs a burst of measurements and prints them to serial plotter
//loops and repeats after delay of a couple seconds
#elif defined(MODE_BURST_CAPTURE)
  Serial.println("Burst Capture Mode (CH0 via GPIO)");
  while(1) {
    captureBurstFast();
    for (int i = 0; i < NUM_SAMPLES; i++) {
      Serial.println(adcVoltage[i], 4);  // For Serial Plotter
    }
    delay(2000);
  }
#else
  #error "Please define one of the modes."
#endif
}

void loop() {
  // Empty
}

// Function to read data from ADC
// Input arument is command, for ADC reading can set command to all zeros
// Returns result from ADC
uint16_t readADS7945(uint16_t cmd) {
  uint16_t value;
  SPI.beginTransaction(adsSettings);
  digitalWrite(CS_PIN, LOW);
  value = SPI.transfer16(cmd);
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();
  return value;
}


//Takes code form the ADC and converts it to a voltage value
//this function uses various constants defined earlier in the sketch
//input argument is the ADC code and returns voltage as float
float convertToVoltage(int16_t raw_code) {
    raw_code = raw_code & 0x3FFF; //get rid of 15th bit telling us its negative
    return (((float)raw_code /ADC_BITS) * VREF_VOLTAGE);
}

//function for running burst mode
//optimized for speed to run the ADC as fast as possible
//This function uses ESP32 specific GPIO commands for faster 
//spedds compared to traditional digitalwrite() functions
#if defined(MODE_BURST_CAPTURE)
void captureBurstFast() {
  // Dummy read to start pipeline
  readADS7945(0xC000);
  delayMicroseconds(5);
  noInterrupts();  // Max speed, no preemption

  SPI.beginTransaction(adsSettings);
  uint32_t startTime = micros();  // Start timer, timer tells you how long it took to capture samples
  for (int i = 0; i < NUM_SAMPLES; i++) {
    GPIO.out_w1tc = (1 << CS_PIN);  // Set pin LOW
    adcRaw[i] = SPI.transfer16(0xC000); //14 bit so shift result in 16 bit variable
    GPIO.out_w1ts = (1 << CS_PIN);  // Set pin HIGH
  }

  uint32_t endTime = micros();  // End timer
  uint32_t duration = endTime - startTime; //get time it took to run burst mode
  Serial.print("Operation took ");
  Serial.print(duration);
  Serial.println(" microseconds.");

  SPI.endTransaction();

  interrupts();  // Restore interrupts

  //convert ADC codes to voltage values and save in global array
  for (int i = 0; i < NUM_SAMPLES; i++) {
    adcVoltage[i] = convertToVoltage((adcRaw[i]>>1));
  }
}
#endif

