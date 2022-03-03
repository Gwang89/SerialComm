# SerialComm
This code is to communicate using various serial communication protocols in MSP430. 

# UART 
- UART is configured to receive/transmit what user types and displays on the terminal console. The buad rate is 19.2Kbits/sec. 

# SPI
- SPI is configured to communicate with nokia5110 LCD module. This module aceepts MOSI/CLK/SS signals. 
- SPI is also used to communicate with the encoder LS7336R module, which is attached to the motor. This can give an angular displacement as pulse count. 
This module accepts MOSI/MISO/CLK/SS signals. 

# I2C
- I2C is configured to communicate with pixyCam.

