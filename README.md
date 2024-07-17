# o2macrodevice
Anderson Lab @ MIT

Author: Shravika Pendyala
When: UROP for Summer of 2024

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
