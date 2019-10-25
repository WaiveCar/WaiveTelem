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

e.g. for 0123CCBCCC98B697EE
- go to https://us-east-2.console.aws.amazon.com/iot/home?region=us-east-2#/thing/0123CCBCCC98B697EE
- click on "Shadow" and edit the Shadow State to have one of the following JSONs:

```json
{"desired":{"lock":"open"}}
{"desired":{"lock":"close"}}
{"desired":{"immo":"lock"}}
{"desired":{"immo":"unlock"}}
{"desired":{"reboot":"true"}}
{"desired":{"remoteLog":"1"}}
{"desired":{"download":{"host":"waiveplug.s3.us-east-2.amazonaws.com", "from":"waive1000_1.3.2_fc277bc970ec26a53f33b8c8cf6874598d1fdbbc73f2a1d3cc0b69f3920ba23e", "to":"ETADPU.BIN"}}}
{"desired":{"download":{"host":"waiveplug.s3.us-east-2.amazonaws.com", "from":"config_0123CCBCCC98B697EE_e9d1dd7d396224ed9311b8b56ee321fbebf0118c0bad9fa2e914c30c3fb2455b", "to":"CONFIG.TXT"}}}
{"desired":{"copy":{"from":"DEFAULT.BIN", "to":"ETADPU.BIN"}}}
{"desired":{"copy":{"from":"DEFAULT.CFG", "to":"CONFIG.TXT"}}}
{"desired":{"can":"unlock_1"}}
{"desired":{"can":"unlock_all"}}
{"desired":{"can":"lock"}}
{"desired":{"can":"flash_lights"}}
{"desired":{"simIgnition":"on"}}
{"desired":{"simIgnition":"off"}}
```

### MQTT Device Shadow Reported (Telemetry):

```json
{
  "remoteLog": "4",
  "init": {
    "firmware": "1.3.2",
    "debug": 1,
    "configFreeMem": 239,
    "modem": "L0.0.00.00.05.08,A.02.04",
    "sd": 1,
    "eeprom": 1,
    "motion": -1,
    "cfg": 1
  },
  "lastCmd": "{\"immo\":\"lock\"}",
  "immo": "lock",
  "lastCmdDatetime": "2019-10-24T04:36:55Z",
  "crash": {
    "time": 1571861930,
    "backtrace": [
      "dda6",
      "de9c",
      "11544",
      "de7e",
      "116c2",
      "e14c"
    ]
  },
  "heartbeat": {
    "datetime": "2019-10-25T17:35:05Z",
    "lat": 34.1500035,
    "long": -118.0278152,
    "hdop": 670,
    "speed": 0,
    "heading": 0,
    "uptime": 34250,
    "temp": -1,
    "freeMem": 6587,
    "lastVin": 1229,
    "signal": 15,
    "carrier": "Verizon Wireless Sierra Wireless"
  },
  "lock": "open"
}
```

### Crash Report:

For each binary release, there should be a disassembly info in disassembly folder. You can search the code address (something like 698e or 9176) to see the source code.

Also, when you run `./run.sh` or `./debug.sh`, disassembly for the build is created in ./nosave/dis.txt

To clear crash report:
```json
{"reported":{"crash":null}}
```

### Remote Logging:

Remote Loggin is enabled for log level 4 (ERROR) by default. It can be changed to other levels (1: DEBUG for example) by sending
```json
{"desired":{"remoteLog":"1"}}
```
See packages/plug-log-forwarder for how to capture logs from all devices

### Functional Test for each release:

At very least, make sure
1. All BLE commands and status-read work from the App.
2. From Device Shadow, all commands should work promptly.