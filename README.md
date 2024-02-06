This respository contains codes that enable messages can be sent and received from a LILYGO T-SIM7000G or T-SIM7070G wireless module and a mobile phone, and displayed on a OLED screen. The assoicated video can be found on youtube

These modules use the TinyGSM Arduino library, which doesn’t have a function included to receive SMS. This makes it difficult to control peripherals. Here, three scripts are shown which enables SMS to be received (and control an LED).

1.	`simple_display.ino` Simply displaying a SMS a message sent from a phone 
2.	`scrolling_display.ino` Scrolling the received SMS message across the screen, and using SMS to toggle an LED. A SMS can also be sent to the module which replies with a text including the battery charge level.
3.	`two_core_display.ino` A script in which both cores of the ESP32 are used. One core is used for scrolling the message across the display, the second is used for checking for received messages. This means the two processes can be run concurrently, which avoids the code that scrolls the message from blocking checking for new messages.

The video also shows how to fix the error caused by using an out-of-date USB driver and MacOS, which can occur with T-SIM7070G.

The T-SIM7070G is a newer version of the T-SIM7000G, and has an improved power-saving mode, which makes it better suited for battery powered applications.

