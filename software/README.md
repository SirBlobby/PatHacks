# Software 

Pygame Serial Controller for ESP32

#Deliverables
I want you to make the software that will be used to send network information to the hardware.  I also wish it to add the account to the hardware.  I want everything from this readme to be sent into software/ directory and NOWHERE ELSE.

I want this to be a seperate tool from the rest of the program.  It will have a GUI interface using Tauri Rust.  It will use svelte for frontend.  This software will interface with the ESP32 through a usb serial connection.

When you start this application, it will boot a title called "Learning Buddy Initalization" and have a Button called "Start". 

When the button "Start" is pressed, it will first ask for networking information.  This will be a menu that will list all availiable "WIFIs" and when one is selected, it will ask for any security information to allow the device to log in

This information will be sent an the hardware and if it connects, it will continue, otherwise it would fail to the user on the "Learning Buddy Initalization tool" would have to input again.

Once the hardware is connected to a wifi network, a new page will be brought up, it will have a title "Learning Buddy Initalized!" and then have a short paragraph stating "Your Learning Buddy has been successfully initalized"  And there will be 2 buttons, one to close the application, and one that will close the application and load up the manager

#File Organizations
<Any high level program and entry point for our software can go here>
software/

<This will contain all information that allows our program to interface with the hardware>
software/networking/

<front end code that has everything for the user experience>
software/front_end
software/front_end/html

<back end code that has everything for wifi gathering>
software/back_end

