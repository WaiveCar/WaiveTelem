# Plug Firmware

### Preparation:

- Install git (or git bash for windows, https://gitforwindows.org/)
- Install Python 2.7 https://www.python.org (on Windows, add python.exe to Path when using the installer)
- Install Visual Studio Code (https://code.visualstudio.com/download)
- In VSCode, install PlatformIO IDE extension (File -> Preferences -> Extensions)
- In VSCode Terminal (on Windows, make sure Default Shell is bash)
  - Git clone WaiveTelem `git clone git@github.com:WaiveCar/WaiveTelem.git`
- In VSCode, File -> Open folder WaiveCar/packages/plug-firmware
- Connect to the plug via USB

### Building Binary (default mkr1000USB):

- pio run -v

### Building, Flash and Run Binary for mkrnb1500:

- pio run -v -e mkrnb1500 -t upload

### Run Binary and Start Serial Monitoring:

- pio run -v -t upload; sleep 3; pio device monitor

### Update Firmware Build Version:

- update it in platformio.ini
  
### MQTT Device Shadow Desired (Command):

e.g. for plug-1
- go to https://us-east-2.console.aws.amazon.com/iot/home?region=us-east-2#/thing/plug-1
- click on "Shadow" and edit the Shadow State to have one of the following JSONs:

```json
{ "desired": { "doors": "unlocked" } }
{ "desired": { "doors": "locked" } }
{ "desired": { "vehicle": "immoblized" } }
{ "desired": { "vehicle": "unimmoblized" } }
```

### MQTT Device Shadow Reported (Telemetry):

```json
{
  "doors": "unlocked",
  "vehicle": "unimmoblized",
  "gps": {
    "lat": 1.11,
    "long": 2.22,
    "time": "tbd"
  },
  ...
}
```
