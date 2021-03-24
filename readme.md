Simple Qt/qwt application to read out and display data from a Tek492
spectrum analyzer.

# Requirements

```
sudo apt-get install build-essential qt5-default libqwt-qt5-dev libqt5svg5-dev
```

## optional

```
sudo apt-get install qtcreator
```

# Build

```
qmake tek492.pro
make
```

or build with qtcreator

# Usage

Connect ciruitben's USB adapter to the Tek492 analyzer and your computer
(see "usb_492" subdirectory).

Start Qt application.

Choose File -> Connect to connect to the adapter.

Adjust Qt controls to match those on Tek492 analyzer.

Press "Acquire".

Enable and adjust markers as you like.

Data can be saved in an XML file and read back from there (this part works
without usb_492 connected as well).
