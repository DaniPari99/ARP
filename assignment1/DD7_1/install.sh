
unzip source.zip 

cd source/Master
gcc master.c -o master
cd ..
cd CommandConsole
gcc commandConsole.c -o commandConsole
cd ..
cd MotorX
gcc motorX.c -o motorX
cd ..
cd MotorZ
gcc motorZ.c -o motorZ
cd ..
cd InspectionConsole
gcc inspectionConsole.c -o inspectionConsole
cd ..
cd WatchDog
gcc watchDog.c -o watchDog

echo installation completed
