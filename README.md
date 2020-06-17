# ESP8266-server-GY-521-MPU-6050
### Fetch data from linux machine
```console
while true; do
curl esp.ip.here >> out.csv
done
```
```console
-1.6870, -1.6183, -0.4351, 29.0947, 2059
-1.7175, -1.4122, -0.5496, 29.0476, 2059
-1.8625, -1.7099, -0.5725, 29.1417, 2060
-1.4580, -1.6106, -0.5496, 29.0947, 2060
```

![Alt text](./movement-sample1.svg)

![Alt text](./temp-sample1.svg)

### Device setup
 
![Alt text](./esp-with-sensor.jpg)
 - D1 - SCL
 - D2 - SDA
 - power sensor with 3v from board

### Sleep cycles
![Alt text](./sleep-activity1.svg)
