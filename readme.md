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

It is also possible to use a separate build directory:

```
mkdir build
cd build
qmake ../tek492.pro
make
```

or build with qtcreator

# Usage

On Linux systems, install `etc_udev_rules.d/42-usb492.rules` into
`/etc/udev/rules.d`, and run `sudo udevadm control --reload` before
connecting the USB adapter. On other OSes, ensure in other ways the
user who wants to run the tool got appropriate permissions granted to
access the USB device node.

Connect ciruitben's USB adapter to the Tek492 analyzer and your computer
(see "usb_492" subdirectory).

Start Qt application.

Choose File -> Connect to connect to the adapter.

Adjust Qt controls to match those on Tek492 analyzer.

Press "Acquire".

Enable and adjust markers as you like.

Data can be saved in an XML file and read back from there (this part works
without usb_492 connected as well).
