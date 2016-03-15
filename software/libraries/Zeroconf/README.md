# Zeroconf
ESP8266 WiFi Connection manager with fallback web configuration portal

The configuration portal is of the captive variety, so on various devices it will present the configuration dialogue as soon as you connect to the created access point.

First attempt at a library. Lots more changes and fixes to do. Contributions are welcome.

#### This works with the ESP8266 Arduino platform with a recent stable release(2.0.0 or newer) https://github.com/esp8266/Arduino

## Contents
 - [How it works](#how-it-works)
 - [Wishlist](#wishlist)
 - [Quick start](#quick-start)
   - Installing
     - [Through Library Manager](#install-through-library-manager)
     - [From Github](#checkout-from-github)
   - [Using](#using)
 - [Documentation](#documentation)
   - [Access Point Password](#password-protect-the-configuration-access-point)
   - [Callbacks](#callbacks)
   - [Configuration Portal Timeout](#configuration-portal-timeout)
   - [On Demand Configuration](#on-demand-configuration-portal)
   - [Custom Parameters](#custom-parameters)
   - [Custom IP Configuration](#custom-ip-configuration)
   - [Filter Low Quality Networks](#filter-networks)
   - [Debug Output](#debug)
 - [Troubleshooting](#troubleshooting)
 - [Releases](#releases)
 - [Contributors](#contributions-and-thanks)


## How It Works
- when your ESP starts up, it sets it up in Station mode and tries to connect to a previously saved Access Point
- if this is unsuccessful (or no previous network saved) it moves the ESP into Access Point mode and sZeroconf up a DNS and WebServer (default ip 192.168.4.1)
- using any wifi enabled device with a browser (computer, phone, tablet) connect to the newly created Access Point
- because of the Captive Portal and the DNS server you will either get a 'Join to network' type of popup or get any domain you try to access redirected to the configuration portal
- choose one of the access points scanned, enter password, click save
- ESP will try to connect. If successful, it relinquishes control back to your app. If not, reconnect to AP and reconfigure.

## How It Looks
![ESP8266 WiFi Captive Portal Homepage](http://i.imgur.com/YPvW9eql.png) ![ESP8266 WiFi Captive Portal Configuration](http://i.imgur.com/oicWJ4gl.png)

## Wishlist
- ~~remove dependency on EEPROM library~~
- ~~move HTML Strings to PROGMEM~~
- ~~cleanup and streamline code~~ (although this is ongoing)
- if timeout is set, extend it when a page is fetched in AP mode
- ~~add ability to configure more parameters than ssid/password~~
- ~~maybe allow setting ip of ESP after reboot~~
- ~~add to Arduino Library Manager~~
- ~~add to PlatformIO~~
- add multiple sets of network credentials
- allow users to customize CSS

## Quick Start

### Installing
You can either install through the Arduino Library Manager or checkout the latest changes or a release from github

#### Install through Library Manager
__Currently version 0.8 works with release 2.0.0 or newer of the [ESP8266 core for Arduino](https://github.com/esp8266/Arduino)__
 - in Arduino IDE got to Sketch/Include Library/Manage Libraries
  ![Manage Libraries](http://i.imgur.com/9BkEBkR.png)

 - search for Zeroconf
  ![Zeroconf package](http://i.imgur.com/18yIai8.png)

 - click Install and start [using it](#using)

####  Checkout from github
__Github version works with release 2.0.0 or newer of the [ESP8266 core for Arduino](https://github.com/esp8266/Arduino)__
- Checkout library to your Arduino libraries folder

### Using
- Include in your sketch
```cpp
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <Zeroconf.h>
```

- Initialize library, in your setup function add
```cpp
Zeroconf Zeroconf;
```

- Also in the setup function add
```cpp
//first parameter is name of access point, second is the password
Zeroconf.autoConnect("AP-NAME", "AP-PASSWORD");
```
if you just want an unsecured access point
```cpp
Zeroconf.autoConnect("AP-NAME");
```
or if you want to use and auto generated name from 'ESP' and the esp's Chip ID use
```cpp
Zeroconf.autoConnect();
```

After you write your sketch and start the ESP, it will try to connect to WiFi. If it fails it starts in Access Point mode.
While in AP mode, connect to it then open a browser to the gateway IP, default 192.168.4.1, configure wifi, save and it should reboot and connect.

Also see [examples](https://github.com/tzapu/Zeroconf/tree/master/examples).

## Documentation

#### Password protect the configuration Access Point
You can and should password protect the configuration access point.  Simply add the password as a second parameter to `autoConnect`.
A short password seems to have unpredictable results so use one that's around 8 characters or more in length.
The guidelines are that a wifi password must consist of 8 to 63 ASCII-encoded characters in the range of 32 to 126 (decimal)
```cpp
Zeroconf.autoConnect("AutoConnectAP", "password")
```

#### Callbacks
##### Enter Config mode
Use this if you need to do something when your device enters configuration mode on failed WiFi connection attempt.
Before `autoConnect()`
```cpp
Zeroconf.setAPCallback(configModeCallback);
```
`configModeCallback` declaration and example
```cpp
void configModeCallback (Zeroconf *myZeroconf) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());

  Serial.println(myZeroconf->getConfigPortalSSID());
}
```

##### Save settings
This gets called when custom parameters have been set **AND** a connection has been established. Use it to set a flag, so when all the configuration finishes, you can save the extra parameters somewhere.

See [AutoConnectWithFSParameters Example](https://github.com/tzapu/Zeroconf/tree/master/examples/AutoConnectWithFSParameters).
```cpp
Zeroconf.setSaveConfigCallback(saveConfigCallback);
```
`saveConfigCallback` declaration and example
```cpp
//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
```

#### Configuration Portal Timeout
If you need to set a timeout so the ESP doesn't hang waiting to be configured, for instance after a power failure, you can add
```cpp
Zeroconf.setConfigPortalTimeout(180);
```
which will wait 3 minutes (180 seconds). When the time passes, the autoConnect function will return, no matter the outcome.
Check for connection and if it's still not established do whatever is needed (on some modules I restart them to retry, on others I enter deep sleep)

#### On Demand Configuration Portal
If you would rather start the configuration portal on demand rather than automatically on a failed connection attempt, then this is for you.

Instead of calling `autoConnect()` which does all the connecting and failover configuration portal setup for you, you need to use `startConfigPortal()`. __Do not use BOTH.__

Example usage
```cpp
void loop() {
  // is configuration portal requested?
  if ( digitalRead(TRIGGER_PIN) == LOW ) {
    Zeroconf Zeroconf;
    Zeroconf.startConfigPortal("OnDemandAP");
    Serial.println("connected...yeey :)");
  }
}
```
See example for a more complex version. [OnDemandConfigPortal](https://github.com/tzapu/Zeroconf/tree/master/examples/OnDemandConfigPortal)

### Custom Parameters
You can use Zeroconf to collect more parameters than just SSID and password.
This could be helpful for configuring stuff like MQTT host and port, [blynk](http://www.blynk.cc) or [emoncms](http://emoncms.org) tokens, just to name a few.
**You are responsible for saving and loading these custom values.** The library just collects and displays the data for you as a convenience.
Usage scenario would be:
- load values from somewhere (EEPROM/FS) or generate some defaults
- add the custom parameters to Zeroconf using
 ```cpp
 // id/name, placeholder/prompt, default, length
 ZeroconfParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
 Zeroconf.addParameter(&custom_mqtt_server);

 ```
- if connection to AP fails, configuration portal starts and you can set /change the values (or use on demand configuration portal)
- once configuration is done and connection is established [save config callback]() is called
- once Zeroconf returns control to your application, read and save the new values using the `ZeroconfParameter` object.
 ```cpp
 mqtt_server = custom_mqtt_server.getValue();
 ```  
This feature is a lot more involved than all the others, so here are some examples to fully show how it is done
- Save and load custom parameters to file system in json form [AutoConnectWithFSParameters](https://github.com/tzapu/Zeroconf/tree/master/examples/AutoConnectWithFSParameters)
- *Save and load custom parameters to EEPROM* (not done yet)


### Custom Access Point IP Configuration
This will set your captive portal to a specific IP should you need/want such a feature. Add the following snippet before `autoConnect()`
```cpp
//set custom ip for portal
Zeroconf.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
```

### Custom Station (client) Static IP Configuration
This will make use the specified IP configuration instead of using DHCP in station mode.
```cpp
Zeroconf.setSTAStaticIPConfig(IPAddress(192,168,0,99), IPAddress(192,168,0,1), IPAddress(255,255,255,0));
```
There are a couple of examples in the examples folder that show you how to set a static IP and even how to configure it through the web configuration portal.

#### Filter Networks
If you would like to filter low signal quality networks you can tell Zeroconf to not show networks below an arbitrary quality %;
```cpp
Zeroconf.setMinimumSignalQuality(10);
```
will not show networks under 10% signal quality. If you omit the parameter it defaults to 8%;

#### Debug
Debug is enabled by default on Serial. To disable add before autoConnect
```cpp
Zeroconf.setDebugOutput(false);
```

## Troubleshooting
If you get compilation errors, more often than not, you may need to install a newer version of the ESP8266 core for Arduino.

Changes added on 0.8 should make the latest trunk work without compilation errors. Tested down to ESP8266 core 2.0.0. **Please update to version 0.8**

I am trying to keep releases working with release versions of the core, so they can be installed through boards manager, but if you checkout the latest version directly from github, sometimes, the library will only work if you update the ESP8266 core to the latest version because I am using some newly added function.


If you connect to the created configuration Access Point but the configuration portal does not show up, just open a browser and type in the IP of the web portal, by default `192.168.4.1`.


## Releases
#### 0.9
 - fixed support for encoded characters in ssid/pass

##### 0.8
 - made it compile on older versions of ESP8266 core as well, tested down to 2.0.0
 - added simple example for Custom IP

##### 0.7
 - added static IP in station mode
 - added example of persisting custom IP to FS config.json
 - more option on portal homepage
 - added on PlatformIO

##### 0.6
 - custom parameters
 - prettier
 - on demand config portal
 - commit #100 :D

##### 0.5
 - Added to Arduino Boards Manager - Thanks Max
 - moved most stuff to PROGMEM
 - added signal quality and a nice little padlock to show which networks are encrypted

##### v0.4 - all of it user contributed changes - Thank you
 - added ability to password protect the configuration Access Point
 - callback for enter configuration mode
 - memory allocation improvements

##### v0.3
 - removed the need for EEPROM and works with the 2.0.0 and above stable release of the ESP8266 for Arduino IDE package
 - removed restart on save of credentials
 - updated examples

##### v0.2
needs the latest staging version (or at least a recent release of the staging version) to work

##### v0.1
works with the staging release ver. 1.6.5-1044-g170995a, built on Aug 10, 2015 of the ESP8266 Arduino library.


### Contributions and thanks
The support and help I got from the community has been nothing short of phenomenal. I can't thank you guys enough. This is my first real attept in developing open source stuff and I must say, now I understand why people are so dedicated to it, it is because of all the wonderful people involved.

__THANK YOU__

[Maximiliano Duarte](https://github.com/domonetic)

[alltheblinkythings](https://github.com/alltheblinkythings)

[Niklas Wall](https://github.com/niklaswall)

[Jakub Piasecki](https://github.com/zaporylie)

[Peter Allan](https://github.com/alwynallan)

[John Little](https://github.com/j0hnlittle)

[markaswift](https://github.com/markaswift)

[franklinvv](https://github.com/franklinvv)

[Alberto Ricci Bitti](https://github.com/riccibitti)

[SebiPanther](https://github.com/SebiPanther)

[jonathanendersby](https://github.com/jonathanendersby)

[walthercarsten](https://github.com/walthercarsten)

Sorry if i have missed anyone.

#### Inspiration
- http://www.esp8266.com/viewtopic.php?f=29&t=2520