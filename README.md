# esp32-temp-reporter

Reads temperature from sensors and reports the data via MQTT

## Background

I have a boat, I'm a Software Engineer, and I am ~~cheap~~ frugal.
NMEA2000 is pretty nice from a raw capabilities perspective but it has a number of limitations:

1. Everything is isolated to the NMEA2000 ecosystem. This is both a strength from a simplicity / reslience perspective but a weakness in that your visualization, alarming, logging, access, etc. is also limited.
1. The sensors used in NMEA2000 networks aren't anything really "new". Things like temperature sensors, tank sensors, flow sensors, etc. have existed in industrial automation for years.
1. The componenets / bridges to connect to NMEA2000 are more expensive than an ESP32 microcontroller (Hundreds to Thousands of USD vs. ~$10-15USD)

There are a lot of other frameworks / projects that could tackle this problem but they also have their drawbacks (some of them are just personal to me):

1. SignalK aimed to address some of the above constraints but there is fairly tight coupling between the open data standard and the reference implementation.
1. The SignalK standard itself (at least from what I could decipher) didn't lend itself to extensibility - e.g. I wanted to define arbitrary temperature sensors in locations of my choosing and the spec only seemed to allow for pre-defined temperature locations without any ability to extend it.
1. The reference implementation is implemented using NodeJS / Javascript which is not my personal preference for programming language. More concerning is that as with most JS based projects dependencies are managed using npm that has a tendency to pull in a LOT of unvetted 3P dependencies. I didn't want to put something on my boat that took on that level of risk.
1. ESPHome / HomeAssistant is viable as well and there is an awesome ecosystem there. This is the road I initially went down, but when I started to try to get a ESP32-C6 to work I ran into a lot of issues. HA/ESPHome are highly complex OSS projects and are working on having the most impact to the smarthome community. As such, there are a number of layers of complexity to add new microcontroller support / tweak the behavior of the sytem to work well in a marine environment / etc. tldr - for me, this complexity didn't provide the value-add I was looking for in my case. I would imagine that many others would decide the opposite.

## My High Level Requirements

So I thought about what my requirements were and have at least these in mind for now:

1. Use commercially proven OSS solutions / technologies
1. Target linux since I am running a mini-pc 'server' on my boat running linux
1. Support 'real-time' visualization of data
1. Support archiving of all data received with export to something off-boat when there is connectivity for analytics/backup
1. Follow the KISS principal - have components do a small set of well-defined things and do it in a reliable / resilient manner
1. Minimize the attack / complexity surface where possible
1. Support getting all data from my existing NMEA2000 network into this platform (I am thinking readonly at this point)

## Capabilities

Here are some things I know that I want to 'solve' with this system. This may grow / shrink over time.
(This is somewhat of an addendum of specifics onto the high level requirements)

1. Monitor the temperature of different rooms on the boat
1. Monitor the temperature of fridges / freezers
1. Monitor the dry and wet exhaust temperatures of all engines
1. Monitor tank levels (Fuel, Water, Waste)
1. Monitor vacuum levels and/or fluid flow rate (Fuel Filtration, Cooling Water)
1. Monitor voltages (Different Battery Banks)

## Approach / HLD

I wanted to start with monitoring room and fridge/freezer temperature since that was straightforward. I initially was going to reuse some old 433MHz sensors I had lying around but most of those limited me to a max of ~3 different locations.
I then discovered that BLE "Beacon" type sensors were extremely cheap and did exactly what I was looking for - you could place one of these sensors wherever and as long as you could receive their BLE beacon messages you were good to go. No need to worry about wires going through fridge / freezer doors and just as cheap as the 433MHz sensors with no limit on the number of sensors you can have in a single installation.  I ended up selecting these two sensors:

* <https://www.amazon.com/dp/B08S3CGZ3Q> << Does temp only which is fine for fridge/freezer
* <https://www.amazon.com/dp/B08S34C5X9> << Does temp and humidity for an extra $5

Since these sensors are buried away in different spots that BLE can't reach the server we need an intermediary. That's where the esp32-temp-reporter comes in. It reads data from these sensors and then reports it upstream to the server. I'll continue to apply this pattern for other future sensor types.

I *could* have used a few (maybe 2 or 3) RaspberryPi devices could do this as well and considered that. The downside of that is that while only slightly more expensive you then have an entire linux OS to contend with. That sure does make a lot of things easier, but it also adds a bunch of complexity such as patching OS's / dealing with computer hardware failures / debugging / etc. In my case, the ESP32 was a better choice. If one dies I can just drop in a replacement. I don't need to patch an OS and there is little to no debugging you need to do for the device itself.

I'm using MQTT over TLS to send messages to the server. Right now I am using username/password based authentication because I didn't want to setup a private CA to issue certs to devices and I don't *think* that you can get device certs issued from Let's Encrypt. I am using Let's Encrypt for all the certs on my boat. I have a PrivateCA at home where I do development.
The server runs the Mosquitto MQTT Broker from Eclipse (<https://mosquitto.org/>) which is extremely proven technology.

The ESP32 code should be simple, predictable, and auditable. It needs to support WiFi connectivity, MQTT, and the needed code to read from the sensors. No more, no less. I want to focus more of my energy on reliability so handling flaky network/sensors/etc. is of higher importance than supporting a lot of different 'features' on the ESP32 itself.

In order to pull in my existing N2K data (as well as some bonus data) I am using the CerboGX on my boat. I connected the Cerbo to the N2K network and enabled MQTT functionality in the Cerbo. Voila I can get my N2K data over MQTT in my system. (Note that the MQTT integration into the platform is still pending and I will add a repo here once I have that in a state where I can share it).

On the 'server' I am running:

* Ubuntu as the base OS
* Mosquitto as mentioned for MQTT
* Telegraf for processing MQTT messages and shipping them to downstream services (<https://www.influxdata.com/time-series-platform/telegraf/>)
* InfluxDB for timeseries data storage(<https://www.influxdata.com/products/influxdb/>)
* Grafana for visualization(<https://grafana.com/>)
* MariaDB for non-time-series data storage (<https://mariadb.org/>)
* HAProxy as a forward proxy to support SNI and hostname-based routing (<https://www.haproxy.org/>)
* Certbot/Let's Encrypt for Certificates (<https://certbot.eff.org/>)
* Bind9 for local DNS (<https://bind9.net/>)
* PiHole for Ad Blocking (<https://pi-hole.net/>)
* AWS for backup/off-boat analytics (<https://aws.amazon.com/>)

All of these technologies are commercially proven and would likely be considered by most to be 'boring' technology (which is a good thing) <https://boringtechnology.club/>.

The entire server is setup with a single ansible playbook. If I can cleanup the don-specific content in that I can share it.

This project is tackling the ESP side of things. As I do more and have things of value to share with whatever code I need to write on the server and/or other configuration stuff I'll create additional repos and share them here.

## Getting Started

I tried to make forking / reusing this pretty straightforward.  This project uses platformio (<https://platformio.org/>).

You should be able to use/fork this by doing the following:

1. Pull down the code
1. Install platformio on your machine (using the VSCode extension makes this pretty trivial)
1. Create your own appconfig.ini based on the example. Define the build flags in that file applicable to you. You can create multiple environments in that file for each of your ESP32's.
1. Right now I am working with ESP32-C6 devkits so that *should* work out of the box. However adding support for new hardware shouldn't be too hard. I have defined a heirarchical sdkconfig structure for the IDF. So sdkconfig.defaults has all the config common to all ESP32s. sdkconfig.esp32-c6.defaults has the C6-specific config. This is referenced in platformio.ini. It should be 'easy' to just add other extensions to [common] to support other platform-specific needs.
1. That said, right now I am focused on supporting ESP32 chips using the ESP IDF. I don't want to support the Arduino framework nor do I see value in supporting hardware outside of the ESP32 ecosystem. This could of course change with time.
1. Once your config is setup you should be able to run ```pio run -t upload``` and then ```pio device monitor``` to see the code in action

## TODO

* ~~Cleanup the WiFi Utils code~~
* ~~Support Static IP/DNS~~
* ~~Write data as JSON~~
* Add SNTP Support (Needed for TLS among other things)
* Change BLE Implementation to be Sync not async since it is task based which will be cleaner
* Move BLE Listener to own task that enqueues messages for main task
* Add reconnect logic for WiFi and MQTT
* ~~Add topic structure / config to MQTT~~
* ~~MQTT Auth~~
* ~~Support private CA signed certs~~
* Try to get MQTT Client to use the embedded CA Trust Store
* Confirm that multiple BLE sensors works
* Add thermocouple support
* Add device heartbeat / status
* Add PT RTD support
* Test integration with Telegraf
* Network logging (Syslog)
* Add external ADC support for 4-20ma sensors
* Store some log buffer locally to send over network on reconnect
* OTA Support
* Make config pulled from URL/JSON???
* Add an actual architecture diagram in here
