#Flashing lua scripts

To flash a a lua script follow these steps:

* Create a directory for your project:

      ```mkdir /esp8266test```

* Enter the esp8266test directory:

      ```cd /esp8266test```

* download the following phyton script [https://raw.githubusercontent.com/kmpm/nodemcu-uploader/master/nodemcu-uploader.py](https://raw.githubusercontent.com/kmpm/nodemcu-uploader/master/nodemcu-uploader.py).


      ```wget https://raw.githubusercontent.com/kmpm/nodemcu-uploader/master/nodemcu-uploader.py```


* Flash the script with the following command:

      ```sudo python nodemcu-uploader.py upload init.lua```

* You should see the following output on the console:

      ```Connecting...
      Erasing flash...
      Wrote 462848 bytes at 0x00000000 in 45.6 seconds (81.3 kbit/s)...

      Leaving... ```
