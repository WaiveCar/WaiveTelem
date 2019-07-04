# Plug Firmware

### Preparation (should work on Windows, Linux, and MacOS):

- Install Visual Studio Code (https://code.visualstudio.com/download)
- Install PlatformIO IDE extension (Code -> Preferences -> Extensions) in VSCode
- Git clone WaiveTelem
- File -> Open ./packages/plug-firmware folder in VSCode
- Connect to the plug via USB

### Building Binary:

- pio run

### Building, Flash and Run Binary:

- pio run -t upload

### Run Binary and Start Serial Monitoring:

- pio run -t upload; sleep 3; pio device monitor

### MQTT Device Shadow Desired (Command):

- go to https://us-east-2.console.aws.amazon.com/iot/home?region=us-east-2#/thing/plug-1
- click on "Shadow" and edit the Shadow State to have one of the following JSON:

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
  }
}
```
