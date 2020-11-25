# ESP Zzzzleep 😴

## Bill of Materials
- ESP8266 (microcontroller)
- GY-521 MPU-6050 (accelerometer / gyroscope sensor)
- breadboard and wires
- power supply or powerbank for independant operation
- soldering iron for preparation of the sensor module

## Complete Instructions
We assume you are starting from scratch; only in posession of items mentioned in the BoM.

#### 1. Preparation
If your sensor comes with seperate header pins it is recommended to solder them onto the chip first (the ESP usually ships with presoldered header pins). If you are just getting started with soldering you can find guidance on this task here: https://www.instructables.com/Solder-Arduino-Header-Pins-Easily/

#### 2. Assembly and Wiring
- Stick the sensor and the ESP onto the breadboard similar to how it is shown in the first image
- Attach the wires according to the scheme shown in the second image

##### Breadboard Setup:
![Alt text](./esp-with-sensor.jpg)

##### Wiring Scheme:
![Alt text](./ESP8266-wiring.png)

#### 3. Preparing your ESP
Setup Arduino IDE with ESP8266 on your computer. You need to do this in order to compile and upload the code onto the ESP. Follow advice from these sites if you don't know how to do this:
- https://www.arduino.cc/en/Guide
- https://www.instructables.com/Setting-Up-the-Arduino-IDE-to-Program-ESP8266

When your Arduino IDE is ready to upload to ESP8266 copy and paste the code from esp.ino into Arduino IDE and edit ssid, password and ip address (insert the ip address of the machine running the Streams-HTTP-Gateway) before hitting upload. Make sure you choose the appropriate board and USB port.

### 4. Start Streams-HTTP-gateway
Install the Streams-HTTP-gateway available on https://github.com/iot2tangle/Streams-http-gateway

It is necessary that you follow the instructions given on the Github page ie. install Rust, build dependencies etc. then run ```cargo run --release``` inside the project directory.

Start the server and you should see requests coming in after a few seconds.

### Movement indicates sleep cycles
![Alt text](./sleep-activity1.svg)
