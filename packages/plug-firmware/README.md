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
{"desired":{"download":{"host":"waiveplug.s3.us-east-2.amazonaws.com", "from":"1000_1.0.4_9ccd5d8eb348015145a4df52cd29e7e233768a9d2fc475e4aab4773fc5cccb66", "to":"UPDATE.BIN"}}}
{"desired":{"download":{"host":"waiveplug.s3.us-east-2.amazonaws.com", "from":"plug1_config_e6cd5276eb396159eebce1005be1b5e6926d394d9cf6bf7e0e7c4c70cad43341", "to":"CONFIG.TXT"}}}
{"desired":{"copy":{"from":"DEFAULT.BIN", "to":"UPDATE.BIN"}}}
{"desired":{"copy":{"from":"DEFAULT.CFG", "to":"CONFIG.TXT"}}}
```

### MQTT Device Shadow Reported (Telemetry):

```json
{
  "inRide":"true",
  "firmware":"1.0.1",
  "lock":"open",
  "immo":"false",
  "gps":{
    "lat":36.149893,
    "long":-115.027760,
    "time":"2019-07-12T18:33:01Z"
  },
  ...
}
```
