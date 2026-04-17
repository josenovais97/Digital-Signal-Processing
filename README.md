# Morse Light Decoder (MLD)

This project aims to develop a system that uses an LED to transmit signals in Morse code, which are captured by an LDR sensor and decoded. The decoding result is displayed on an LCD screen.

The system was developed for the Digital Signal Processing course of the Telecommunications and Informatics Engineering degree at the University of Minho.

## 👥 Authors
* José Novais (A105056)
* Miguel Machado (A103668)
* Tiago Diogo (A103665)

---

## 🎯 Objectives
* Implement a system that transmits and decodes Morse code with light.
* Ensure that 100% of messages are decoded correctly.
* Use an LED and an LDR, accessible components, to enable transmission and reading.
* Demonstrate a functional system using an Arduino microcontroller.
* Complete the project within a 2-month timeframe.

---

## 🏗️ System Architecture
The LED transmits Morse signals based on the message written in the terminal. The LDR captures the emitted light pulses. The captured signal is filtered to remove noise. The Arduino processes the signal and decodes the characters. The decoded characters are displayed on the LCD.

### Hardware Specifications
* **LED:** Red LED
* **LDR:** Light Dependent Resistor
* **Microcontroller:** Arduino UNO
* **Display:** 16x2 LCD
* **Resistors:** 1x 220 OHM and 1x 10 KOHM
* **Filter:** IIR (Infinite Impulse Response) Filter

### Software Specifications
Code for Morse transmission, decoding with digital filter, and LCD display output.

---

## 💻 Development and Implementation

The development environment was configured with the Arduino IDE to program the microcontroller, alongside signal analysis tools to design the digital filter.

**Steps for implementing the filter:**
1. Analyze the signal captured by the LDR to identify noise frequencies.
2. Design the IIR (Infinite Impulse Response) filter using MATLAB.
3. Convert the filter coefficients to C/C++ and integrate them into the Arduino code.

**Integration with the microcontroller:**
* Configure the LED to transmit Morse code.
* Read the LDR sensor and apply the filter.
* Display the decoded characters on the LCD.

---

## 🧪 Tests and Results
The system was able to consistently transmit and decode Morse code messages in environments with controlled lighting. The use of the digital filter significantly improved the quality of the signal read by the LDR. The project met all expected results.

**Adjustments made to improve performance:**
* Threshold calibration.
* Improvement in the symbol decoding algorithm.
* Hardware modification for better LED-LDR coupling and performance.

---

## 🚀 Conclusion and Future Work
The project achieved its objectives, demonstrating the feasibility of transmitting and decoding Morse signals using an LED and LDR. The integration of the digital filter was essential to the success of the system.

### Limitations
Difficulty operating in environments with very intense ambient light.

### Future Extensions and Applications
Use of more sophisticated sensors, such as photodiodes, for greater precision and performance.

---
### 📄 Documentation
* [View the Full Project Presentation (PDF)](https://github.com/josenovais97/Digital-Signal-Processing/blob/main/Morse%20Light%20Decoder%20-%20PDS/Processamento%20Digital%20de%20Sinal.pdf)
