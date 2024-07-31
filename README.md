# o2macrodevice
Anderson Lab @ MIT

Author: Shravika Pendyala
When: UROP for Summer of 2024


31 July 2024
Code perfectly working + set up with GitHub: ran into issue of where files are, but had to move all drivers/middleware to C:/.
Pulsing can be adjusted in the rfalNfcWorker() function in rfal_nfc.c file. set time in ms to be ON and time to be OFF using HAL_Delay

SetUp: Using ST-Link programmer to upload the code, have the Nucleo + NFC reader hooked up to a benchtop power supply. power is not received from the ST-Link! Custon antenna maybe proken? For this setup, using the NFC board antenna with AAT disabled. So, two LEDs on: Blue TX LED + Green PWR LED = ~10mA.

At 20%: 20min ON, 40min OFF
while ON (not being received) = ~56mA
while ON (received) = ~47mA



17 July 2024
Code form July 4 = Perfect + Working

19 June 2024

Right now, the only code used to upload to the ST Nucleo is main.c located in 
(NUCLEO-L476RG/Applications/NFC08A1_PollingTagDetectNDEF/Src/main.c)

The NFC06A1 is not used anywhere

Task: 
Determine what aspects of the Nucleo-L476RG are actually necessary to just power the NFC08A1 board + start thinking about designing a board that does the minimum required to power the NFC board. 

INSTRUCTIONS TO DOWNLOAD/SETUP
1. https://www.st.com/en/embedded-software/x-cube-nfc6.html : Download driver (has example code in it)
2. Extract the STM32 Folder inside the project you wan't to install. (Not the whole NDEFPolling folder, this will create build errors)


BOARDS INFO
1. In my personal file system, the folders are an accurate representation of the primary chip used in the project
