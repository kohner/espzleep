# ESP8266-server-GY-521-MPU-6050
### Fetch data from linux machine
```console
while true; do
curl esp.ip.here >> out.csv
done
```
More info in [esp-init.bash](#init-script), this is some example CSV output
```console
-1.6870, -1.6183, -0.4351, 29.0947, 2059
-1.7175, -1.4122, -0.5496, 29.0476, 2059
-1.8625, -1.7099, -0.5725, 29.1417, 2060
-1.4580, -1.6106, -0.5496, 29.0947, 2060
```

![Alt text](./sample1.svg)

![Alt text](./temp-sample1.svg)

### Device setup
 
![Alt text](./esp-with-sensor.jpg)
 - D1 - SCL
 - D2 - SDA
 - power sensor with 3v from board

### Movement indicates sleep cycles
![Alt text](./sleep-activity1.svg)

### Init script
Meant to be run before going to bed with ESP :)  
It will  
 - connect to the ESP AP (via networkmanager, nmcli)
 - test connection to the IP provided
 - start background loop (curl fetches data)
 - quit when q is entered, everything else is ignored
 - => Don't quit with CTRL-C, the loop will keep running and you have to kill it manually. Entering q does this for you.
 
Consider making the following changes
 - SSID of ESP access point
 - password of ESP access point
 - private IP address of ESP
 - SSID of home wifi (reconnect before quitting)
 - (filename)
   
This script is meant to serve as guidance for you to create your own.
