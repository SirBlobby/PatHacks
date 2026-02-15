# Software 

Turijs 2.0 Rust Serial Controller for ESP32

#Deliverables
I want you to make the software that will be used to send network information to the hardware. I want everything from this readme to be sent into software/ directory and NOWHERE ELSE.

It will have a GUI interface using Tauri Rust. It will use svelte 5 for frontend. This software will interface with the ESP32 through a usb serial connection.

When you start this application, it will boot a title called "Learning Buddy Initalization" and have a Button called "Start". 

When the button "Start" is pressed, it will first ask for networking information. This will be a menu that will list all availiable "WIFIs" and when one is selected, it will ask for any security information to allow the device to log in

This information will be sent an the hardware and if it connects, it will continue, otherwise it would fail to the user on the "Learning Buddy Initalization tool" would have to input again.

Once the hardware is connected to a wifi network, a new page will be brought up, it will have a title "Learning Buddy Initalized!" and then have a short paragraph stating "Your Learning Buddy has been successfully initalized"  And there will be 2 buttons, one to close the application, and one that will close the application and load up the manager
