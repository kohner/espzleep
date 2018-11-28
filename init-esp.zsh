# CONNECT
counter=0
while true; do
    while [[ $(nmcli device wifi connect ESPap password thereisnospoon12) && $? -ne 0 ]]; do
        echo Could not connect
        (( counter++ ))
    done
    echo Succesfully connected
    curl 192.168.4.1        
    if [[ $? -eq 0 ]]; then
        echo Connection active
        break
    elif [[ $counter -eq 5 ]]; then
        echo "Connection to ESP AP present but curl failed, wrong IP?"
        echo "Tried 5 times, giving up"
        exit 1
    fi
done

# FETCH
while true; do
    curl 192.168.4.1
done &
whilePid=$!

# EXIT
while true; do
    read -k 1 usrIn
    if [[ $usrIn == "q" ]]; then
        echo q from user received, hold on a second
        kill $whilePid
        if [[ $? -ne 0 ]]; then
            echo "Could not terminate while loop, please clean up manually, thanks"
        fi
        exit 0
    fi
done
