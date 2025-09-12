# FlexDualChannelADC_ESP32_Example
This example ESP32 Arduino sketch is meant to work with Anabit's Flex Dual Channel ADC open source reference design
Product link: 

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
