#!/usr/bin/env python3

# Dependencies:
# python-requests - https://pypi.org/project/requests/

import sys
import requests
import time


domain = "esp8266.local"
url = "http://" + domain

format = "SleepLog_%d-%m-%Y-%H-%M"
filename = time.strftime(format, time.localtime())

if len(sys.argv) == 2 :
    filename = sys.argv[1]

def fetchData():


    # test connection
    try:
        status_code = str(requests.get(url + "?elements=0&offset=0").status_code)
        if not status_code == "200":
            print("Network error " + status_code)
            exit()
    except:
        print("Could not connect to host")
        exit()


    # get x elements of data step by step
    elements = 2
    offset = 0
    linesToFile = []
    while True:

        queryString = F"?elements={elements}&offset={offset}"
        try:
            response = requests.get(url + queryString)
        except:
            print("Connection lost")
            print("I still give you the data but it is incomplete")
            return linesToFile


        for line in response.text.split(";"):

            if line == "stop":
                # a request to <esp-domain>/ returns "interval xxx"
                try:
                    return linesToFile + [requests.get(url).text]
                except:
                    print("Connection Lost")
                    print("Could not fetch interval but the rest of the data will be written")
                    return linesToFile

            elif line != "":
                linesToFile.append(line)


        offset += elements



def main():

    f = open(filename, "a")

    for line in fetchData():
        f.write(line + "\n")
    f.write(str(int(time.time())))

    f.close()



main()
