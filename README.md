# simple_clock
This is a script to turn your arduino into a very simple digital clock. All you need is:
* an Atmega328 based arduino (for library compatibility)
* a 4 digit seven segment display
* 7 resistors (8 if you want to use the dot) (I suggest you use >1000 Ohm) 
* (optionally) 2 pushbuttons to set the time

The 7 segment display is driven directly from the arduino, no controller necessary. The 7 resistors act as current limiting transistors for each segment. Make sure you use a high value, because you'll need to drive 7 LEDs from 1 I/O pin! I used 1000 Ohm resistors so even with all segments lit, current is about 20mA. If you want to power your display to max brightness, you'll need additional transistors to drive the digit pins. 

You can set the time using a serial connection, or you can optionally use 2 pushbuttons to set the time for a standalone situation. 

This program uses the FrequencyTimer2 library to multiplex the display. It uses the Adafruit RTCLib to keep track of time without using a real time clock. 
Formerly this script had some driftcorrection and timer overflow recovery in it. It never worked very well so I gave up and added an RTC module a long time ago, just like any sane person would do. I just cleaned up the code a bit since.

![image of assembled clock](https://cloud.githubusercontent.com/assets/5601853/12264188/40d789d8-b935-11e5-9a99-9ecfd0f05aa7.jpg)
