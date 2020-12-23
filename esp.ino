#include <Wire.h>
#include <stdio.h>
#include <stdlib.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "arduino_secrets.h"


// Network / Communication


// The ESP will send the data via Serial Connection
// additionally it can be sent via WiFi / HTTP to a server.

// Define your SSID and password in the seperate file arduino_secrets.h
const char* ssid = SECRET_SSID;
const char* password = SECRET_PASS;

ESP8266WebServer server(80);


// Configuration


// The interval controls how much time the ESP spends collecting data
// before sending it to the server. Set to for example 60000 (1 minute).
// Maximum is 72 minutes (see 'millis() rollover, I hope my calculation is correct').
// When debugging you probably want to set this lower...
// Keep in mind that lowering this value may cause problems when running
// for a longer period of time as the memory on the ESP is very limited.
const int interval = 60000;

// Values exceeding the threshold will be appended to the data vector.
// All other data will be discarded.
// This limits the amount of memory taken up by the date avector.
// Additionally during sleep most of the time values are below a certain value
// and are of no interest so the threshold comes without loss of quality in
// sleep analysis when set correctly.
// To find out the optimal threshold value just set it to zero and make a test run...
const float thresholdAccel = 0.1;
const float thresholdGyro = 1.5;


// Button


const int buttonPin = D0;
int buttonState;


// Loop Variables


// All data collected will be stored in this vector.
// Each tuples holds the maximum values of the
// Gyro- and Accelerometer sensor for a specific time span.
std::vector<std::tuple<int, float, float, float, float>> dataVector{};

// Additional variables for the loop
int startTime;
bool btn;
float GyroX = 0;
float GyroY = 0;
float GyroZ = 0;
float preGyroX = 9;
float preGyroY = 9;
float preGyroZ = 9;
float maxAccel = 0;
int skippedVals = 0;


// Sensor


struct rawdata {
  int16_t AcX;
  int16_t AcY;
  int16_t AcZ;
  int16_t Tmp;
  int16_t GyX;
  int16_t GyY;
  int16_t GyZ;
};

struct scaleddata {
  float AcX;
  float AcY;
  float AcZ;
  float Tmp;
  float GyX;
  float GyY;
  float GyZ;
};

const uint8_t MPU_addr = 0x68; // I2C address of the MPU-6050

const float MPU_GYRO_250_SCALE = 131.0;
const float MPU_GYRO_500_SCALE = 65.5;
const float MPU_GYRO_1000_SCALE = 32.8;
const float MPU_GYRO_2000_SCALE = 16.4;
const float MPU_ACCL_2_SCALE = 16384.0;
const float MPU_ACCL_4_SCALE = 8192.0;
const float MPU_ACCL_8_SCALE = 4096.0;
const float MPU_ACCL_16_SCALE = 2048.0;

bool checkI2c(byte addr);
void mpu6050Begin(byte addr);
void setMPU6050scales(byte addr, uint8_t Gyro, uint8_t Accl);
void getMPU6050scales(byte addr, uint8_t &Gyro, uint8_t &Accl);
rawdata mpu6050Read(byte addr, bool Debug);
scaleddata convertRawToScaled(byte addr, rawdata data_in, bool Debug);


// MPU Sensor Functions


// mpu6050Begin: This function initializes the MPU-6050 IMU Sensor.
// It verifys the address is correct and wakes up the MPU.
void mpu6050Begin(byte addr) {
  if (checkI2c(addr)) {
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x6B); // PWR_MGMT_1 register
    Wire.write(0); // set to zero (wakes up the MPU-6050)
    Wire.endTransmission(true);

    delay(30); // Ensure gyro has enough time to power up
  }
}

// checkI2c: We are using the return value of the Write.endTransmisstion
// to see if a device acknowledged to the address.
bool checkI2c(byte addr) {
  Serial.println(" ");
  Wire.beginTransmission(addr);

  if (Wire.endTransmission() == 0)
  {
    Serial.print(" Device Found at 0x");
    Serial.println(addr, HEX);
    return true;
  }
  else
  {
    Serial.print(" No Device Found at 0x");
    Serial.println(addr, HEX);
    return false;
  }
}

// mpu6050Read: This function reads the raw 16-bit data values from the MPU-6050
rawdata mpu6050Read(byte addr, bool Debug) {
  rawdata values;

  Wire.beginTransmission(addr);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(addr, 14); // request a total of 14 registers
  values.GyX = Wire.read() << 8 | Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  values.GyY = Wire.read() << 8 | Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  values.GyZ = Wire.read() << 8 | Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
  values.Tmp = Wire.read() << 8 | Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  values.AcX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  values.AcY = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  values.AcZ = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)

  if (Debug) {
    Serial.print(" GyX = "); Serial.print(values.GyX);
    Serial.print(" | GyY = "); Serial.print(values.GyY);
    Serial.print(" | GyZ = "); Serial.print(values.GyZ);
    Serial.print(" | Tmp = "); Serial.print(values.Tmp);
    Serial.print(" | AcX = "); Serial.print(values.AcX);
    Serial.print(" | AcY = "); Serial.print(values.AcY);
    Serial.print(" | AcZ = "); Serial.println(values.AcZ);
  }

  return values;
}

void setMPU6050scales(byte addr, uint8_t Gyro, uint8_t Accl) {
  Wire.beginTransmission(addr);
  Wire.write(0x1B); // write to register starting at 0x1B
  Wire.write(Gyro); // Self Tests Off and set Gyro FS to 250
  Wire.write(Accl); // Self Tests Off and set Accl FS to 8g
  Wire.endTransmission(true);
}

void getMPU6050scales(byte addr, uint8_t &Gyro, uint8_t &Accl) {
  Wire.beginTransmission(addr);
  Wire.write(0x1B); // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(addr, 2); // request a total of 14 registers
  Gyro = (Wire.read() & (bit(3) | bit(4))) >> 3;
  Accl = (Wire.read() & (bit(3) | bit(4))) >> 3;
}

scaleddata convertRawToScaled(byte addr, rawdata data_in, bool Debug) {
  scaleddata values;
  float scale_value = 0.0;
  byte Gyro, Accl;

  getMPU6050scales(addr, Gyro, Accl);

  if (Debug) {
    Serial.print("Gyro Full-Scale = ");
  }

  switch (Gyro) {
    case 0:
      scale_value = MPU_GYRO_250_SCALE;
      if (Debug) {
        Serial.println("±250 °/s");
      }
      break;
    case 1:
      scale_value = MPU_GYRO_500_SCALE;
      if (Debug) {
        Serial.println("±500 °/s");
      }
      break;
    case 2:
      scale_value = MPU_GYRO_1000_SCALE;
      if (Debug) {
        Serial.println("±1000 °/s");
      }
      break;
    case 3:
      scale_value = MPU_GYRO_2000_SCALE;
      if (Debug) {
        Serial.println("±2000 °/s");
      }
      break;
    default:
      break;
  }

  values.GyX = (float) data_in.GyX / scale_value;
  values.GyY = (float) data_in.GyY / scale_value;
  values.GyZ = (float) data_in.GyZ / scale_value;

  scale_value = 0.0;
  if (Debug) {
    Serial.print("Accl Full-Scale = ");
  }
  switch (Accl) {
    case 0:
      scale_value = MPU_ACCL_2_SCALE;
      if (Debug) {
        Serial.println("±2 g");
      }
      break;
    case 1:
      scale_value = MPU_ACCL_4_SCALE;
      if (Debug) {
        Serial.println("±4 g");
      }
      break;
    case 2:
      scale_value = MPU_ACCL_8_SCALE;
      if (Debug) {
        Serial.println("±8 g");
      }
      break;
    case 3:
      scale_value = MPU_ACCL_16_SCALE;
      if (Debug) {
        Serial.println("±16 g");
      }
      break;
    default:
      break;
  }
  values.AcX = (float) data_in.AcX / scale_value;
  values.AcY = (float) data_in.AcY / scale_value;
  values.AcZ = (float) data_in.AcZ / scale_value;


  values.Tmp = (float) data_in.Tmp / 340.0 + 36.53;

  if (Debug) {
    Serial.print(" GyX = "); Serial.print(values.GyX);
    Serial.print(" °/s| GyY = "); Serial.print(values.GyY);
    Serial.print(" °/s| GyZ = "); Serial.print(values.GyZ);
    Serial.print(" °/s| Tmp = "); Serial.print(values.Tmp);
    Serial.print(" °C| AcX = "); Serial.print(values.AcX);
    Serial.print(" g| AcY = "); Serial.print(values.AcY);
    Serial.print(" g| AcZ = "); Serial.print(values.AcZ); Serial.println(" g");
  }

  return values;
}


// Data Collection


// refreshData: Fetch values from MPU and find new maximum values
void refreshValues() {
  rawdata next_sample;
  scaleddata values;

  // Get data from MPU
  setMPU6050scales(MPU_addr, 0b00000000, 0b00010000);
  next_sample = mpu6050Read(MPU_addr, false);
  values = convertRawToScaled(MPU_addr, next_sample, false);

  // Accelerometer: Get maximum acceleration
  // Using fabsf because it is not import in which direction we move.
  maxAccel = max(maxAccel, max(fabsf(values.AcZ), max(fabsf(values.AcY), fabsf(values.AcZ))));

  // Gyroscope: If the values have changed the position of the sensor is change
  // so we can conclude that the person has moved.
  if (values.GyX != GyroX) {
    GyroX = values.GyX;
  }
  if (values.GyY != GyroY) {
    GyroY = values.GyY;
  }
  if (values.GyZ != GyroZ) {
    GyroZ = values.GyZ;
  }
}


// Networking / Server


void handleRoot() {
  digitalWrite(LED_BUILTIN, HIGH);

  String content = "";
  int elements;
  int offset;
  int vectorSize = dataVector.size();
  std::tuple<int, float, float, float, float> element;

  if (server.args() == 2) {
    if (server.hasArg("elements") && server.hasArg("offset")) {
      elements = server.arg("elements").toInt();
      offset = server.arg("offset").toInt();
      Serial.println("Received request: offset " + String(offset) + ", elements " + String(elements));
      for (int i = offset; i < elements + offset; i++) {
        if (i >= vectorSize) {
          Serial.print("Vector size exceeded, ");
          server.send(200, "text/plain", content + "stop");
          Serial.println("Sent rest of vector + stop signal to client: " + content);
          return;
        }
        element = dataVector.at(i);
        content = content + String(std::get<0>(element)) + "," + String(std::get<1>(element)) + "," + String(std::get<2>(element)) + "," + String(std::get<3>(element)) + "," + String(std::get<4>(element)) + ";";
        Serial.println("appended: " + String(std::get<0>(element)) + "," + String(std::get<1>(element)) + ";");
      }
    }
    server.send(200, "text/plain", content);
    Serial.println("Sent to client: " + content);
  } else {
    server.send(200, "text/plain", String(interval));
    Serial.println("Sent to client:" + String(interval));
    // When the client requests the interval we interpret this as a notification
    // that all data has been received; therefore we just clear the data vector
    // so esp can start collection data again.
    //    dataVector.clear();
  }
  
  digitalWrite(LED_BUILTIN, LOW);
}


// Setup


// setup: The setup includes initializations of networking and hardware interfaces
void setup(void) {
  // Initialize serial communication
  Serial.begin(115200);

  // Initialize I2C communication with MPU
  Wire.begin();
  mpu6050Begin(MPU_addr);

  // Initialize the button and LED pins
  pinMode(buttonPin, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // Set timer
  startTime = millis();
}


// Loop


// After a set time span the maximum values it received during that time
// will be appended to the data vector.
void loop(void) {
  delay(25);


  // Refresh maximum and minimum values
  refreshValues();  
//  Serial.println("test: " + String(maxAccel) + "," + String(GyroX) + "," + String(GyroY) + "," + String(GyroZ));


  // If 'interval' time has elapsed
  // - (calculate Gyro difference)
  // - push the data pair to the vector
  // - reset timer and vars
  if (millis() - startTime > interval) {

    if (maxAccel > thresholdAccel || GyroX > preGyroX + thresholdGyro || GyroX < preGyroX - thresholdGyro || GyroY > preGyroY + thresholdGyro || GyroY < preGyroY - thresholdGyro || GyroZ > preGyroZ + thresholdGyro || GyroZ < preGyroZ - thresholdGyro) {
      // Make tuple and append to data vector along with number of skipped values
      dataVector.push_back(std::make_tuple(skippedVals, maxAccel, GyroX, GyroY, GyroZ));
      preGyroX = GyroX;
      preGyroY = GyroY;
      preGyroZ = GyroZ;
      Serial.println("Treshold exceeded! append: " + String(skippedVals) + "," + String(maxAccel) + "," + String(GyroX) + "," + String(GyroY) + "," + String(GyroZ));
      skippedVals = 0;
    } else {
      skippedVals ++;
      Serial.println("discarding: " + String(maxAccel) + "," + String(GyroX) + "," + String(GyroY) + "," + String(GyroZ));
    }

    // Reset timer / maxAccel
    startTime = millis();
    maxAccel = 0;
  }


  // Check button status
  // Start server if the button is pressed (HIGH)
  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH) {
    Serial.println("Button pressed");
    
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH);
    
    // WiFi connection
    Serial.print("Connecting to WiFi network " + String(ssid) + " ");

    // Make WiFi client and request connection to network
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) { // run <- status
      delay(500);
      digitalWrite(LED_BUILTIN, LOW);
      delay(500);
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.print(".");
      // Pressing the button will abort connection attempt
      buttonState = digitalRead(buttonPin);
      if (buttonState == HIGH) {
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.println();
        Serial.println("Button pressed, aborting attempt");
        delay(1000);
        return;
      }
    }
    digitalWrite(LED_BUILTIN, LOW);
    
    Serial.println("");
    Serial.println("Connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // mDNS
    // Start mDNS with name esp8266 -> addres esp8266.local
    if (MDNS.begin("esp8266")) {
      Serial.println("MDNS started");
    }

    // Server
    Serial.print("Starting server: ");
    server.on("/", handleRoot);
    server.begin();
    Serial.println("Ready.");

    // Keep server running until button is pressed again
    buttonState = digitalRead(buttonPin);
    while (buttonState == LOW) {
      server.handleClient();
      MDNS.update();
      buttonState = digitalRead(buttonPin);
    }
    
    digitalWrite(LED_BUILTIN, HIGH);
    
    Serial.println("Closing connection");
    // todo: confirm this works:                                  
    server.close();
    server.stop();
    WiFi.mode(WIFI_OFF);
    delay(1000);
  }
}
