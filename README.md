# Morse decoder with light sensor – FRDM-KL05Z 

## Introduction
Student project on Microprocessor Technology II course at the AGH University of Krakow

## Project Description  
A Morse code decoder using the **FRDM-KL05Z** microcontroller and **ALS-PT19** light sensor.  
The light analysys using the ADC and then converted into Morse code characters.  
The decoded text is sent via **UART** and can be displayed on a terminal.  

## Calibration  
The sensitivity of Morse code detection can be adjusted by modifying:  
- `#define THRESHOLD` – brightness threshold for detecting light (ADC range 0-4096)  
- `#define MORSE_UNIT` – time unit in ms for a short signal  

## Example Usage  
- When the light blinks in Morse code, the decoder reads the patterns and displays characters.  
- The decoded characters are sent via UART and can be viewed in a terminal.  

https://github.com/user-attachments/assets/d809f1d9-b8c8-485c-b760-6339b4626a54

