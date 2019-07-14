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
{"desired": {"doors": "unlocked"}}
{"desired": {"doors": "locked"}}
{"desired": {"immobilized": "true"}}
{"desired": {"immobilized": "false"}}
{"desired": {"inRide": "true"}}
{"desired": {"inRide": "false"}}
{"desired": {"firmware": "1.0.3", "downloadHost": "waiveplug.s3.us-east-2.amazonaws.com", "downloadFile": "1000_1.0.3_81bac42b67cde96a56f086a7c5c396592bca67cdc30a900fee96f6b8f075d065"}}
```

### MQTT Device Shadow Reported (Telemetry):

```json
{
  "inRide":"true",
  "firmware": "1.0.1",
  "doors": "unlocked",
  "immobilized": "false",
  "gps": {
    "lat": 36.149893,
    "long": -115.027760,
    "time": "2019-07-12T18:33:01Z"
  },
  ...
}
```
