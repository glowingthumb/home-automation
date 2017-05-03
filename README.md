# Homeautomation

This is a home automation project in which you can switch ON or switch OFF eight devices using SMS. You also 
receive an SMS confirmation of the device state after each successful switching.
Time can be set on the onboard RTC using an SMS and you can also retrieve the time set at the RTC using SMS.
The on-board LCD also shows the date and time values.

The project if you see has been demonstrated on a 8051 development board with eight LEDs, a RTC, LCD and a SIM300 GSM module.
Instead of the LEDs one can put a Relay driver circuit to work with real life loads.

The code here can work as a boilerplate code for your Automation projects. 

As we know that 8051 does not have a I2C controller, I have emulated I2C using 2 IO lines.
All latest controllers have lot of protocol bus controllers with pin muxing like I2C, SPI, etc. 
but if you are new to microcontrollers and want to understand how I2C works in bit level
you can go through the project code.

# Hardware Description:

You can check out the video of the project on my YouTube channel.
I have used P89V51RD2 microcontroller on this board, DS 1307 RTC chip with battery backup and an LCD display. 
IO ports 2.6 and 2.7 are used as SLC and SDA for the RTC chip.

# There are 5 functions that this project does:

1. Communicates with the SIM 300 GSM module to send and receive SMSes.
2. Writes and Reads date time information to and from the RTC chip using I2C bus emulation.
3. Writes the date and time information to the LCD.
4. Toggles the GPIO pins connected to the onboard LEDs.
5. Updates the time information on the LCD with 1 second interrupt to the microcontroller from the RTC chip.

You can use these modules in your own automtion project to create a practical product.

The code was compiled using Keil uVision 4 on Windows and yet to be tested on newer versions of Keil. 
You might require to change the headers if you want to compile it on Linux or other environment.

# Youtube Video:
https://youtu.be/y_ucW4E6sUI
