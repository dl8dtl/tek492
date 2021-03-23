Simple Qt/qwt application to read out and display data from a Tek492
spectrum analyzer.

Usage:

Connect ciruitben's USB adapter to the Tek492 analyzer and your computer
(see "usb_492" subdirectory).

Compile and start Qt application.

Choose File -> Connect to connect to the adapter.

Adjust Qt controls to match those on Tek492 analyzer.

Press "Acquire".

Enable and adjust markers as you like.

Data can be saved in an XML file and read back from there (this part works
without usb_492 connected as well).
