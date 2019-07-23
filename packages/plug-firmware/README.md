# Plug Firmware

### Preparation:

- Install git (or git bash for windows, https://gitforwindows.org/)
- Install Python 3.7 https://www.python.org (on Windows, check Add python 3.7 when using the installer)
- Install Visual Studio Code (https://code.visualstudio.com/download)
- In VSCode, install PlatformIO IDE extension (File -> Preferences -> Extensions)
- In VSCode Terminal (on Windows, make sure Default Shell is bash)
```bash
git clone git@github.com:WaiveCar/WaiveTelem.git
```
- In VSCode, File -> Open folder WaiveCar/packages/plug-firmware
- Open VSCode Terminal, allow this workspace to modifiy the terminal shell when prompted, exit Terminal
- Open VSCode Terminal again, pio CLI should be available now
- Update and run setEnv.sh in terminal shell to export env variables:
- Connect to the plug via USB

### Run Binary and Start Serial Monitoring (default mkr1000USB):

```bash
./run.sh
```

### Flash firmware binary:

```bash
./flash.sh bin/1000_1.0.3_06cdba4ab3c940d63f7ca4b2689e34003c18efbbaa5a7c2849d9df9661153ab8
```

### Upload firmware binaries and sync it with S3 bucket:

```bash
./uploadRelease.sh
```

### Building, Flash and Run Binary for mkrnb1500:

```bash
pio run -v -e mkrnb1500 -t upload
```

### MQTT Device Shadow Desired (Command):

e.g. for plug-1
- go to https://us-east-2.console.aws.amazon.com/iot/home?region=us-east-2#/thing/plug-1
- click on "Shadow" and edit the Shadow State to have one of the following JSONs:

```json
{"desired":{"reboot":"true"}}
{"desired":{"lock":"open"}}
{"desired":{"lock":"close"}}
{"desired":{"immo":"lock"}}
{"desired":{"immo":"unlock"}}
{"desired":{"inRide":"true"}}
{"desired":{"inRide":"false"}}
{"desired":{"download":{"host":"waiveplug.s3.us-east-2.amazonaws.com", "from":"1500_1.0.5_b824c05bbd9ebd19c5eb9546ef46615607a7da4f3e435c97fbb286969d8fc2d8", "to":"UPDATE.BIN"}}}
{"desired":{"download":{"host":"waiveplug.s3.us-east-2.amazonaws.com", "from":"config_waive-1_dd22d948fbd671c5751640a11dec139da46c5997bb3f20d0b6ad5bd61ac7e0cc", "to":"CONFIG.TXT"}}}
{"desired":{"copy":{"from":"DEFAULT.BIN", "to":"UPDATE.BIN"}}}
{"desired":{"copy":{"from":"DEFAULT.CFG", "to":"CONFIG.TXT"}}}
```

### MQTT Device Shadow Reported (Telemetry):

```json
{
  "inRide": "false",
  "system": {
    "firmware": "1.0.5",
    "lastInfo": "BLE Disconnected",
    "signalStrength": -46,
    "heapFreeMem": 6655,
    "statusFreeMem": 768,
    "lastCmd": "{\"immo\":\"lock\"}",
    "uptime": 14401
  },
  "gps": {
    "lat": 34.1499433,
    "long": -118.0278233,
    "speed": 0.023015579,
    "heading": 207.3500061,
    "dateTime": "2019-07-23T00:01:21Z"
  },
  "lock": "open",
  "immo": "lock"
}
```
