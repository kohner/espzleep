#!/usr/bin/env python3

import time
import sys
import matplotlib.pyplot as plt
import numpy as np
from scipy.interpolate import make_interp_spline, BSpline
from datetime import datetime


if len(sys.argv) == 1 or sys.argv[1] == "-h" or len(sys.argv) > 2:
    print("Usage: analyze-data.py <filename>")
    exit()



# addSkipped: Insert data for time periods without movement

# (the program on the esp removes motion data below
# a threshold and adds the number of skipped data samples
# (the counter) to the next data sample.)
def addSkipped(dataVector):

    newVector = []
    for i in dataVector:


        counter = int(i.split(",")[0])
        if counter != 0 and counter != 0.0:

            # insert data that has been omitted
            for j in range(counter) :
                newVector.append((0.0, 0.0, 0.0, 0.0))

        newVector.append(tuple(float(j) for j in i[i.find(",")+1:].split(",")))

    return newVector



def checkWakeup(dataVector):
    pass
    # check if there was movement shortly before waking up
    # movement indicates that the wakeup time was very good because it coincides with REM sleep



""" timeToIndex: This function converts seconds to index based on the interval (approx).
    For ex 60 seconds are 3 index steps in the list if the interval is 20 seconds.
"""
def secondsToIndex(seconds, interval):
    if interval < 1000:
        print("interval too small")
        return

    return int(seconds / int(interval/1000))



# dozeOff: return the index where the person likely fell asleep
# find the first NREM sleep phase
# falling asleep approx 10 min. before first NREM
def dozeOff(interval, dataVector):

    timespan = secondsToIndex(1800, interval) # 20 min.

    i=0
    while i <= len(dataVector):

        j = i
        while dataVector[j] == 0.0:
            j = j + 1

            if j - i >= timespan:
                return i - secondsToIndex(600, interval)

        i = j + 1

    return False



def nremPhases(interval, dataVector):
    nremPhasesList = []
    timespan = secondsToIndex(1200, interval) # 20 min.

    n = 0
    while n < len(dataVector):

        # increase n until we find start (no movement)
        #print(n, dataVector[n], dataVector[n] == (0.0, 0.0, 0.0, 0.0))
        if dataVector[n] == (0.0, 0.0, 0.0, 0.0):
            start = n
            
            # check if no movement for minimum phase length (20 min) after start
            stop = start + timespan
            section = dataVector[start:stop]

            # greatest element is 0 -> NREM phase detected
            if max(section) == (0.0, 0.0, 0.0, 0.0):

                # find end of nrem phase
                stop += 1
                while True:
                    if stop == len(dataVector) or dataVector[stop] != (0.0, 0.0, 0.0, 0.0):
                        nremPhasesList.append((start,stop-1))
                        n = stop + 1
                        break

                    stop += 1
        n += 1
    
    return nremPhasesList



def fallAsleep(nremPhasesList, interval):

    # calculate time where person fell asleep
    # 10 min. before first NREM phase
    return nremPhasesList[0][0] - secondsToIndex(600, interval)



def hypnogramMake(dataVector, nremPhasesList, fallAsleepIndex, interval, remVal, awakeVal):
    hypnogram = dataVector.copy()


    """NREM"""
    for nremPhase in nremPhasesList:
        for i in range(nremPhase[0], nremPhase[1]):
            hypnogram[i] = 0


    """REM"""
    for i in range(len(nremPhasesList)-1):
        for j in range(nremPhasesList[i][1], nremPhasesList[i+1][0]):
            hypnogram[j] = remVal

    # fix end:
    for i in range(nremPhasesList[-1][-1], len(dataVector)):
            hypnogram[i] = remVal

    # fix start:
    for i in range(fallAsleepIndex, nremPhasesList[0][0]):
            hypnogram[i] = remVal


    """Awake"""
    for i in range(fallAsleepIndex):
        hypnogram[i] = awakeVal
    for i in range(len(dataVector) - secondsToIndex(360, interval), len(dataVector)):
        hypnogram[i] = awakeVal


    return hypnogram



# detectWakeup: When the person wakes up she usually gets off heir bed, does some stuff like stretching or drinking water
# (meanwhile movement = 0 of course) and when she's done she pushes the button. So the moment of awakening is likely followed
# by a couple of quiet minutes.
def detectWakeup():
    pass



def main():
    filename = sys.argv[1]

    date_object = datetime.strptime(filename, "SleepLog_%d-%m-%Y-%H-%M")
    print("date_object =", date_object)
    try:
        f = open(filename)
    except:
        print("Could not open file")
        exit()

    dataVector = f.read().splitlines()
    
    interval = int(dataVector[-2].split("; ")[0])
    timeSinceWakeup = int(dataVector[-2].split("; ")[1])
    print(interval, timeSinceWakeup)
    timestamp = int(dataVector[-1])
    dataVector = dataVector[:-2]

    dataVector = addSkipped(dataVector)

    nremPhasesList = nremPhases(interval, dataVector)
    print(nremPhasesList)

    duration = (((interval/1000)*len(dataVector))/60)/60 # hours
    print(duration)

    lst = [max(i) for i in dataVector if i != (0.0, 0.0, 0.0, 0.0)]
    avrg = sum(lst) / len(lst)
    if nremPhasesList == []:
        print("No NREM phases found :(")
        accel, gyrox, gyroy, gyroz = zip(*dataVector)
        graphs = (
            (accel, '#0269f9', 'gyroX'),
            (gyrox, '#64a4fc', 'gyroY'),
            (gyroy, '#00357f', 'gyroZ'),
            (gyroz, '#a1d5fc', 'acceleration'),
        )

        for vals, color, label in graphs:
            # the last value controls the grainness (I don't know if len * 200 is ok test with other examples)
            xnew = np.linspace(0, len(vals), len(vals) * 200)
            # the last value controls the degree of smoothing
            spl = make_interp_spline([i for i in range(len(vals))], vals, k=2)
            y_smooth = spl(xnew)
            plt.plot(xnew, y_smooth, color=color, label=label)

        plt.legend(loc='upper left')
        plt.xlabel('Time')
        plt.ylabel('Movement')
        plt.title(F"Sleep on {date_object}")

        plt.show()
        exit()
    else:
        fallAsleepIndex = fallAsleep(nremPhasesList, interval)
        timeAwake = len(dataVector[:fallAsleepIndex]) * (interval/1000)

        # Make the values for the hypnogram graph
        remVal = avrg*1.5/2 # height for REM (NREM is at 0)
        awakeVal = avrg*1.5 # height for Awake
        hypnogram = hypnogramMake(dataVector, nremPhasesList, fallAsleepIndex, interval, remVal, awakeVal)

        # Make the values for the basic movement graphs
        accel, gyrox, gyroy, gyroz = zip(*dataVector)
        graphs = (
            (accel, '#0269f9', 'gyroX'),
            (gyrox, '#64a4fc', 'gyroY'),
            (gyroy, '#00357f', 'gyroZ'),
            (gyroz, '#a1d5fc', 'acceleration'),
        )

        # Smoothing & plotting movement graphs
        for vals, color, label in graphs:
            # the last value controls the grainness (I don't know if len * 200 is ok test with other examples)
            xnew = np.linspace(0, len(vals), len(vals) * 200)
            # the last value controls the degree of smoothing
            spl = make_interp_spline([i for i in range(len(vals))], vals, k=2)
            y_smooth = spl(xnew)
            plt.plot(xnew, y_smooth, color=color, label=label)

        # Plotting hypnogram
        plt.plot(hypnogram, color="#ef7c5f", label="hypnogram", linewidth=2)

        # X axis tuning
        # xticks: 1. arg # of ticks, 2. arg value of ticks
        plt.xticks([i for i in range(len(dataVector))], [i * (duration/len(dataVector)) for i in range(len(dataVector))])
        # # of vals to display on both axis
        plt.locator_params(nbins=duration*2)
        # limit...
        #plt.xlim(right=len(dataVector), left=0)

        # Labels, title, legend
        plt.xlabel('Time')
        plt.ylabel('Depth of Sleep')
        plt.title(F"Sleep on {date_object}")
        plt.legend(loc='upper right')
        plt.legend()

        plt.show()



main()


"""
add support for bulk analysis (directory)
if only little data use minutes instead of hours
hide y vals

"""
