# 2018-junior-lab
Code for 2018 Junior Lab Project

This code transmits ECG data from a Raspberry Pi, over WiFi, to an android application that graphs the data.
The C++ code runs on an RPi, reads the analog signals detected by the ECG using an analog-to-digital converter (ADC), and
sends the data over TCP socket to the android application. The android application graphs the data sent by the RPi in 
real time. 

The user is able actively change which channel of the ADC is being sampled. The C++ code comes with a sine
wave generator the user can change to by changing the filter number. When transmitting the sine wave the ADC is not being
sampled. Instead changing the channel changes the amplitude of the sine wave.

Changes to the android manifest and gradle file are required to run this program.
