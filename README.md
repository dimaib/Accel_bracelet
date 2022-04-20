
# Wireless access device
## Description
This system allows you to distribute people between warehouse equipment and give permission to control certain types of machines. It also allows you to take into account the working hours of employees and conduct convenient analytics according to these hours.
- The system consists of a device that controls the warehouse machine directly
	>The printed circuit board and circuit diagram of the device are created in [KiCad](https://www.kicad.org) 
	Firmware created in Keil uVision for ARM in C languages
- Server that receives data from devices and adds this data to the database
	>The server part was created in Visual Studio C ++, as a console application
- Client application that connects to the database and in a convenient form, displays
	>The client application was created in Visual Studio C#, as an application with a user interface

## Main view PCB
![PCB top](https://github.com/dimaib/UBD/blob/master/schematic_pcb/UBD_stm32/top_view_pcb.png?raw=true)
![enter image description here](https://github.com/dimaib/UBD/blob/master/schematic_pcb/UBD_stm32/bottom_view_pcb.png?raw=true)

## Main client app
![Main client app](https://github.com/dimaib/UBD/blob/master/instruction/%D0%98%D0%BD%D1%82%D0%B5%D1%80%D1%84%D0%B5%D0%B9%D1%81.jpg?raw=true)

>1. _Control panel_
>2. _General statistics panel for the warehouse_
>3. _Personnel control panel_
>4. _Equipment control panel_