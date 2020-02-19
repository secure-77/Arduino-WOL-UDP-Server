# Arduino WOL UDP Server

## Description
Smal UDP Server running on a ESP8266 with simple authentification. Listen for input and send WOL packackes.


## Features

- self startup on power
- generating pin and send it via email
- send WOL signals after correct authentification
- accessible by mobile with termux and ncat
- check target device status via ping
- little bruteforce protection

## Requirements

- Arduino board with ESP8266  i used ![](https://img.shields.io/badge/ESP8266-v%202.3.0-green) https://github.com/esp8266/Arduino
- These two libs (you can find it in the libraries folder):
https://github.com/gpepe/esp8266-sendemail
https://github.com/dancol90/ESP8266Ping
- some tool to communicate with the UDP server (e.g. Ncat: https://nmap.org/ncat/)
- for mobile / *android*  usage, termux: https://termux.com/
- dyndns or static ip and port forwarding for external access 

##Usage

- change the config.h file to your settings.
- upload the sketch to your board
- power on the board

if erverything is fine, your serial output should look like this:

``` 
....... connected
UPD Server is up and running
nc yourDomain.dyndns.org 5444 -u 
Please use the following start code, to wake up the VPN Server: 9792
E-Mail send
AP connection established, listening on 192.168.178.4:5444
``` 


### connect to your udp server
- use your hostname or connect to the ip via ncat

``nc yourDomain.dyndns.org 5444 -u ``

- enter you start code / pin you received via email

- the server is now trying to wake up your target device
[![](https://github.com/secure-77/Arduino-WOL-UDP-Server/blob/master/Termux_example.png)](https://github.com/secure-77/Arduino-WOL-UDP-Server/blob/master/Termux_example.png)


##notes

- for email debugging you can turn on the follwing in the ``libraries\esp8266-sendemail-master\sendemail.h`` file.
```
#define DEBUG_EMAIL_PORT Serial
```
- as i used an older ESP8266 version, i need to change the line 30 in ``libraries\ESP8266Ping-master\src\ESP8266Ping.impl.h``
from
```
_min_time = INT_MAX;
```
to
```
_min_time = 0;
```



