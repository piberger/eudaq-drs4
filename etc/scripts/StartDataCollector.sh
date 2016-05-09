#!/bin/bash
#screen -X -S  "mylittlescreen" quit
#echo "Starting new screen"
#screen -S "mylittlescreen" -d -m
#echo "Starting Collector in screen"
#alias Collector='~/eudaq-drs4/bin/TestDataCollector.exe -r tcp://$HOSTNAME:$RCPORT'
#screen -r "mylittlescreen" -X stuff $'~/eudaq-drs4/bin/TestDataCollector.exe -r tcp://10.2.171.147:44000\n'
#echo "And attach"
#screen -r "mylittlescreen"

echo "starting data collector thing"
echo "-> ~/eudaq-drs4/bin/TestDataCollector.exe -r tcp://$HOSTNAME:$RCPORT"
~/eudaq-drs4/bin/TestDataCollector.exe -r tcp://$HOSTNAME:$RCPORT
echo "done."
