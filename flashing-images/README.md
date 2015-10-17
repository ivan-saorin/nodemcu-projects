# Flashing Node MCU images

To flash a new NodeMCU image follow these steps:

* Download the image from: [here](https://github.com/nodemcu/nodemcu-firmware/releases)
* Clone the [https://github.com/themadinventor/esptool.git](https://github.com/themadinventor/esptool.git) repository.


      git clone https://github.com/themadinventor/esptool.git

* Install phyton:

      apt-get install python

* Ensure phyton serial is also installed (it should be already there):

      apt-get install python-serial

* Enter the esptool directory:

      cd git/esptool

* Flash the image with the following command:

      sudo python esptool.py --port /dev/ttyUSB0  write_flash 0x00000 <path to your image file>

* You should see the following output on the console:

      Connecting...
      Erasing flash...
      Wrote 462848 bytes at 0x00000000 in 45.6 seconds (81.3 kbit/s)...

      Leaving...
