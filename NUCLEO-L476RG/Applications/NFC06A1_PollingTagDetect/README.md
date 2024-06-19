
## <b>NFC06A1 Polling Tag Detect</b>

This application reads/writes NDEF messages as well as work in CE mode

Example Description:

This directory contains a set of source files that implement a simple example based on ST25R3916.
------------------------------------
WARNING: When starting the project from Example Selector in STM32CubeMX and regenerating it
from ioc file, you may face a build issue. To solve it, assuming you started the project for the
Nucleo-L476RG board, remove from the IDE project the file stm32l4xx_nucleo.c in the Application/User
virtual folder and delete, from Src and Inc folders, the files: stm32l4xx_nucleo.c, stm32l4xx_nucleo.h,
stm32l4xx_nucleo_bus.c, stm32l4xx_nucleo_bus.h and stm32l4xx_nucleo_errno.h.
The same operations apply if you starts the project for another STM32 Nucleo board (e.g. for
Nucleo-L053R8, the files to be removed are stm32l0xx_nucleo.c, stm32l0xx_nucleo.c, stm32l0xx_nucleo.h,
stm32l0xx_nucleo_bus.c, stm32l0xx_nucleo_bus.h and stm32l0xx_nucleo_errno.h).
------------------------------------

### <b>Keywords</b>

NFC, NDEF, SPI, ST25R3916

### <b>Directory contents</b>

 - app_x-cube-nfcx.c      NFC6 initialization and applicative code
 
 - analogConfigTbl_NFC06A1.c  Analog configuration file
 
 - main.c                 Main program body
 
 - logger.c               Logger file

 - demo_polling.c         NDEF polling File

 - demo_ce.c              manage card emul mode
 
 - stm32**xx_hal_msp.c    Source code for MSP Initialization and de-Initialization

 - stm32**xx_it.c         Source code for interrupt Service Routines

 - stm32**xx_nucleo.c     Source file for the BSP Common driver 
						
 - stm32**xx_nucleo_bus.c Source file for the BSP BUS IO driver
 
 - system_stm32**xx.c     CMSIS Cortex-Mx Device Peripheral Access Layer System Source File

### <b>Hardware and Software environment</b>

 - This example runs on STM32 Nucleo devices with NFC6 expansion board
 - X-NUCLEO-NFC06A1
 
 - This example has been tested with STMicroelectronics:
 - NUCLEO-L053R8 RevC board
 - NUCLEO-L476RG RevC board
 and can be easily tailored to any other supported device and development board.

ADDITIONAL_BOARD : X-NUCLEO-NFC06A1 https://www.st.com/en/ecosystems/x-nucleo-nfc06a1.html
ADDITIONAL_COMP  : ST25R3916 https://www.st.com/en/nfc/st25r3916.html
  
### <b>How to use it?</b>

In order to make the program work, you must do the following :
 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the example
 - < Add any other step(s) to be done by the user to make the example working >

### <b>Author</b>

SRA Application Team

### <b>License</b>

Copyright (c) 2022 STMicroelectronics.
All rights reserved.

This software is licensed under terms that can be found in the LICENSE file
in the root directory of this software component.
If no LICENSE file comes with this software, it is provided AS-IS.
