# Plug Firmware

### Preparation:

- Install git (or git bash for windows, https://gitforwindows.org/)
- Install Python 3.7 https://www.python.org (on Windows, check Add python 3.7 when using the installer)
- Install Visual Studio Code (https://code.visualstudio.com/download)
- In VSCode, install PlatformIO IDE extension (File -> Preferences -> Extensions)
- In VSCode Terminal (on Windows, make sure Default Shell is bash)
  - Git clone WaiveTelem `git clone git@github.com:WaiveCar/WaiveTelem.git`
- In VSCode, File -> Open folder WaiveCar/packages/plug-firmware
- Open VSCode Terminal, allow this workspace to modifiy the terminal shell when prompted, exit Terminal
- Open VSCode Terminal again, pio CLI should be available now
- Connect to the plug via USB

### Run Binary and Start Serial Monitoring:

- pio run -v -t upload && sleep 5 && pio device monitor

### Building, Flash and Run Binary for mkrnb1500:

- pio run -v -e mkrnb1500 -t upload

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
