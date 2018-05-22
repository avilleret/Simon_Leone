Simon Leone
===========

Compiling
---------

Make sure that [of](https://github.com/ofnode/of) and ofApp share the same folder.
If that's not the case, you should use `OF_ROOT` CMake variable to tell cmake where to find `of`.
For example : `cmake .. -DOF_ROOT=~/dev/of -G Ninja`

Change directory to ofApp and perform these steps:

```bash
git submodule update --init
mkdir build
cd build
cmake .. -G Ninja
ninja
```

You can also use your prefered IDE to load the project (Qtcreator for example.)
It's also possible to generate Xcode project with Ninja, this have not been tested though.

This is intended to work with a hacked Simon Pocket from MB Electronics.
Refer to "Arduino_projects" folder, it contains PlatformIO project with code for Adafruit ItsyBitsy 32u4 5V 16MHz.

The application can run on Raspberry Pi. There is a service file to enable autostart. Link it and enable it :

    sudo ln -s /home/pi/simonleone.service /lib/systemd/system/
    sudo systemctl enable simonleone
    sudo systemctl start simonleone

But it appears that stopping the app with `systemctl stop simonleone` doesn't work. Instead you can unplug the USB device or run `sudo killall Simon_Leone-armv7-Release`.

Instead of starting simonleone at boot, you can start it when Simon Pocket is plugged in thanks to the udev rules `95-simonleone.rules`. Just put it in `/etc/udev/rules.d`. Of course if Simon Pocket is already plugged in when RPi starts, then Simon Leone start right after.
