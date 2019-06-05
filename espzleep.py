#!/ /usr/bin/env python3

# Dependencies:
# requests - https://pypi.org/project/requests/


import requests
import time

ip = "PUT YOUR ESP's IP ADDRESS HERE"
url = "http://" + ip

# File name format (incl. time format (python time module))
format = "SleepLog_%d-%m-%Y-%H-%M"
filename = time.strftime(format, time.localtime())

def writeIt(filename, line):
    f = open(filename, "a")
    f.write(response.text + "\n")
    f.close()

while True:
    response = requests.get(url)
    writeIt(filename, response.text)
    print(response.text)
