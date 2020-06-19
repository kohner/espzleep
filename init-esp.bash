#!/usr/bin/env bash

file=$(date +"slept-with-esp-on-%Y-%m-%d~%H:%M:%S.csv")
origSSID="" # set to "" in order to disable reconnecting
pw=""
espAP=""

# CONNECT
counter=0
while true; do
    while [[ $(nmcli device wifi connect $espAP password $pw) || $? -ne 0 ]]; do
        echo -e "\n [ ERR ] Could not connect\n"
        (( counter++ ))
        if [[ $counter -eq 5 ]]; then
            echo -e "\n [ ERR ] Tried 5 times, giving up\n"
            exit 1
        fi
        echo sudo systemctl-restart NetworkManager:
        sudo systemctl-restart NetworkManager
    done
    echo -e "\n [ OK ] Succesfully connected\n"
    curl 192.168.4.1        
    if [[ $? -eq 0 ]]; then
        echo -e "\n [ OK ] Connection active\n"
        break
    elif [[ $counter -eq 5 ]]; then
        echo -e "\n [ ERR ] Tried 5 times, giving up\n"
        exit 1
    fi
    echo -e "\n [ ERR ] Connection to ESP AP present but curl failed, wrong IP?\n"
    (( counter++ ))
done

# FETCH
while true; do
    curl 192.168.4.1 >> $file
done &
whilePid=$!
echo -e "\n\n [ OK ] PID is $whilePid\n\n"

# EXIT
while true; do
    read -n 1 usrIn
    if [[ $usrIn == "q" ]]; then
        echo -e "\n\n [ OK ] q received, hold on a second\n"
        kill $whilePid
        if [[ $? -ne 0 ]]; then
            echo -e "\n [ ERR ] Could not terminate while loop, please clean up manually, thanks\n"
        fi
        killall curl
        echo -e "\n [ OK ] All curl instances killed\n"
        if [[ ! -z $origSSID ]]; then
            echo -e "\n [ OK ] Attempting to connect to $origSSID\n"
            nmcli device wifi connect $origSSID
            if [[ $? -ne 0 ]]; then
                echo -e "\n [ ERR ] Sorry, could not connect to $origSSID, exiting\n" && exit 1
            fi
            echo -e "\n [ OK ] Succesfully reconnected\n"
        fi
        exit 0
    fi
done
