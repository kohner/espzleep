# ESP Zzzzleep 😴

In this project, we record motion data during sleep to analyze our sleep habits.

The setup on this page is designed to be active during sleep hours to collect sleep data. Analysis of this data provides valuable insight into your sleep activity.

For example: Long term data collection reveals sleep habits and can help you find out important facts about your personal sleep - such as the relationship between wake-up time and sleep quality, sleep quality of naps vs. sleep quality at night, etc.


## Bill of Materials / Things You'll Need
- ESP8266 (microcontroller)
- GY-521 MPU-6050 (accelerometer / gyroscope sensor)
- Breadboard and wires
- Power supply or powerbank for independant operation
- USB / micro USB cable
- Soldering iron for preparation of the sensor module

- **Sostware:**
  - Arduino IDE: https://www.arduino.cc/en/software
  - IOT2TANGLE Streams HTTP Gateway: https://github.com/iot2tangle/Streams-http-gateway

## Complete Setup Instructions
We assume you are starting from scratch; only in posession of items mentioned in the BoM.

### 1. Preparation
If your sensor comes with seperate header pins I recommended you to solder them onto the chip first (the ESP8266 usually ships with pre-soldered header pins). If you are just getting started with soldering you can find guidance on this task here: https://www.instructables.com/Solder-Arduino-Header-Pins-Easily/

### 2. Assembly and Wiring
- Stick the sensor and the ESP onto the breadboard similar to how it is shown in the first image
- Attach the wires according to the scheme shown in the second image

#### Breadboard Setup:
![Alt text](./esp-with-sensor.jpg)

#### Wiring Scheme:
![Alt text](./ESP8266-wiring.png)

### 3. Preparing your ESP8266
Setup Arduino IDE with ESP8266 on your computer. You need to do this in order to compile and upload the code onto the ESP. Follow advice from these sites if you don't know how to do this:
- https://www.arduino.cc/en/Guide
- https://www.instructables.com/Setting-Up-the-Arduino-IDE-to-Program-ESP8266

When your Arduino IDE is ready to upload to ESP8266 copy and paste the code from esp.ino into Arduino IDE and edit
- ssid
- password
- wifi/serial (true or false, depending on how you want the ESP to communicate with your device - see step 4)
before hitting upload. As allways make sure to choose the appropriate board and USB port.

Your ESP should connect to the WiFi network you specified in the source code.

### 4. Fetch the data
Format: ```timestamp,X-Axis,Y-Axis,Z-Axis``` for example: ```161607797332,-1.70,-1.57,-0.59```
#### Via WiFi
If you have a device running in the same network as your ESP you can use it to fetch data from the ESP. In order to use the python script proviede follow these steps:

1.) Make sure WiFi communication is enabled in the source code (esp.ino, wifi = true). Reupload it to the ESP if necessary.

2.) In Arduino IDE open Serial Monitor (Baud Rate: 115200 baud) to receive data from ESP. Plug the power cable in and wait until the ESP prints its IP address.

3.) Once you have the IP address you can power the ESP via alternative sources for ex. a powerbank. You only need to read the IP address once - usually :).

5.) When you are ready start the python script with the correct IP set in the code; ESP should be running. Both devices (ESP and your computer) need to be in the same WiFi network. If everything is working you should see data being printed to the terminal. Everything will be saved to a file as well.

6.) Leave it running as long as you want! Any data received will be saved to a file.


### Via Serial
Read Serial Ouput via ```tail -f /dev/ttyUSB0``` where /dev/ttyUSB0 has to be replaced with the appropriate USB port file. Pipe it to a file like so: ```tail -f /dev/tty/USB0 > test-esp```.


### 5. Analyze!
The data sent to any device is in CSV format which allows for easy visualization via Spreadsheets. For automatic detection of sleep cycles and analysis of other features of your sleep a second python script will be provided in the future.

Information about your sleep cycles can provide valuable insights. You can find out for example different parameters which influence your sleep quality or learn about your long-term sleep habits.

## Movement / Gyro data indicates sleep cycles
Data during sleep:
![Alt text](./sleep-activity1.svg)
Sample motion data:
![Alt text](./sample1.svg)
