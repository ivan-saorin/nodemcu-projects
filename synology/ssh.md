# Setting up ssh on Synology

* go to ```Control Panel > Terminal and SMTP```
* check the ```Activate SSH service``` checkbox
* Tip: choose a ```Port``` in the range 49152 to 65535. See [ephemeral port](https://en.wikipedia.org/wiki/Ephemeral_port).
* In a console connect through ```ssh``` to the Synology Station

      ```ssh <synology address> -p <port> -l <synology admin user>
      ```
