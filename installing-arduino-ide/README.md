#Installing Arduino IDE

* Install the Arduino IDE from [here](https://www.arduino.cc/en/main/software)
* Unzip the ide to a folder into your home directory
* chmod the directory

      ```cd arduino-1.6.5-r5
      chmod 777 * ```

* Execute the installer:

      ```./install.sh ```

* Now you can run the IDE through your desktop

* Once the IDE is loaded, go in ```File > Preferences```

* Add [http://arduino.esp8266.com/stable/package_esp8266com_index.json](http://arduino.esp8266.com/stable/package_esp8266com_index.json) in ```Additional Boards Manager URL```

* Now in ```Tools > Board: xxx > Boards Manager```

* Search for ```ESP8266```

* Click ```Install```

* Select your NodeMCU board in ```Tools > Board: xxx > <boards>```

To allow usb usage within Arduino IDE:
```sudo chmod 666 /dev/ttyUSB0```
