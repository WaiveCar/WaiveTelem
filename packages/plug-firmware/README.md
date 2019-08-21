# Plug Firmware

### Preparation:

### Step 1: (this step is the same for plug-provision as well)
- Install git (or git bash for windows, https://gitforwindows.org/)
- Install Python 3.7 https://www.python.org (on Windows, check Add python 3.7 when using the installer)
- Install Visual Studio Code (https://code.visualstudio.com/download)
- In VSCode, install PlatformIO IDE extension (File -> Preferences -> Extensions)
- In VSCode Terminal (on Windows, make sure Default Shell is bash)
  - install jq `curl -L https://github.com/stedolan/jq/releases/download/jq-1.6/jq-win64.exe --output ~/bin/jq.exe`
  - install aws-cli `pip3 install awscli`
  - login aws `aws configure`
  - Git clone WaiveTelem `git clone git@github.com:WaiveCar/WaiveTelem.git`
- In VSCode, File -> Open Folder WaiveTelem/packages/plug-firmware
- Open VSCode Terminal, allow this workspace to modifiy the terminal shell if prompted, exit Terminal.
- Open VSCode Terminal again, type 'pio' and make sure it is runnable
- Add waive1000 BSP and increase binary upload speed:
```bash
pio run -e mkrnb1500
ln -s -f $PWD/bsp/boards ~/.platformio
ln -s -f $PWD/bsp/waive1000 ~/.platformio/packages/framework-arduinosam/variants
sed -i'.bak' -e 's/adapter_khz\ 400/adapter_khz\ 5000/g' ~/.platformio/packages/tool-openocd/scripts/target/at91samdXX.cfg
```
### Step 2:
- Update and run setEnv.sh in terminal shell to export env variables:
- Connect to device via micro-USB
- Connect Atmel-Ice if you want to use the debugger

### Run Release Binary and Start Serial Monitoring:

```bash
./run.sh
```
note: Since release binary is sleeping most of the time in loop(), you might want to upload binary during setup(), right after reset.

### Run Debug Binary and Start Serial Monitoring:

```bash
./debug.sh
```

### Flash firmware binary:

```bash
./flash.sh s3/mkr1000USB_1.0.8_21b44a51771a67675c5b9c747855eb09fef0b086604f45b43ec7d077d7f5cc5b
```

### Upload firmware binaries and sync it with S3 bucket:

```bash
./uploadRelease.sh
```

### MQTT Device Shadow Desired (Command):

e.g. for plug-1
- go to https://us-east-2.console.aws.amazon.com/iot/home?region=us-east-2#/thing/plug-1
- click on "Shadow" and edit the Shadow State to have one of the following JSONs:

```json
{"desired":{"lock":"open"}}
{"desired":{"lock":"close"}}
{"desired":{"immo":"lock"}}
{"desired":{"immo":"unlock"}}
{"desired":{"inRide":"true"}}
{"desired":{"inRide":"false"}}
{"desired":{"reboot":"true"}}
{"desired":{"download":{"host":"waiveplug.s3.us-east-2.amazonaws.com", "from":"waive1000_1.0.9_d84e070505e00ce74e9ab35de951a41d1712f4e32e9541df5b9b488ff80a46e9", "to":"ETADPU.BIN"}}}
{"desired":{"download":{"host":"waiveplug.s3.us-east-2.amazonaws.com", "from":"mkr1000USB_1.0.9_58f91825e8cdf93e5c0d1b2d3594f17c1748f97aa5c058db57af43da4c6c78c4", "to":"ETADPU.BIN"}}}
{"desired":{"download":{"host":"waiveplug.s3.us-east-2.amazonaws.com", "from":"config_waive-1_dd22d948fbd671c5751640a11dec139da46c5997bb3f20d0b6ad5bd61ac7e0cc", "to":"CONFIG.TXT"}}}
{"desired":{"copy":{"from":"DEFAULT.BIN", "to":"ETADPU.BIN"}}}
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
