# Plug Provision

This package automatically

- Generates a private key on the device, and saves cerficiation signing request in ./csr (generateCSR.sh)
- Sends CSR to AWS, register device, and saves certificate in ./cert (registerAWS.sh)
- Generate ./config/\${DEVICE_ID}/CONFIG.TXT (generateConfig.sh)
- Format the SD (formatSD.sh)
- Copy CONFIG.TXT, DEFAULT.BIN, DEFAULT.CFG, CONFIG.TXT to the SD
- Flash DEFAULT.BIN on the device

## Preparation:

### Step 1: (this step is the same for plug-firmware as well)
- Install git (or git bash for windows, https://gitforwindows.org/)
- Install Python 3.7 https://www.python.org (on Windows, check Add python 3.7 when using the installer)
- Install Visual Studio Code (https://code.visualstudio.com/download)
- In VSCode, install PlatformIO IDE extension (File -> Preferences -> Extensions)
- In VSCode Terminal (on Windows, make sure Default Shell is bash)
  - install jq `curl -L https://github.com/stedolan/jq/releases/download/jq-1.6/jq-win64.exe --output ~/bin/jq.exe`
  - install aws-cli `pip3 install awscli`
  - login aws `aws configure`
  - Git clone WaiveTelem `git clone git@github.com:WaiveCar/WaiveTelem.git`
- In VSCode, File -> Open Folder WaiveTelem/packages/plug-provision
- Open VSCode Terminal, allow this workspace to modifiy the terminal shell if prompted, exit Terminal.
- Open VSCode Terminal again, type 'pio' and make sure it is runnable
- Add waive1000 BSP and increase binary upload speed:
```bash
pio run -e mkrnb1500
ln -F -s $PWD/../plug-firmware/bsp/boards ~/.platformio
ln -F -s $PWD/../plug-firmware/bsp/waive1000 ~/.platformio/packages/framework-arduinosam/variants
sed -i'.bak' -e 's/adapter_khz\ 400/adapter_khz\ 5000/g' ~/.platformio/packages/tool-openocd/scripts/target/at91samdXX.cfg
```

### Step 2:
- Review plug-type, plug-group, and plug-policy on AWS IoT console as they should be considered carefully. They were created with the following:
  - aws iot create-thing-type --thing-type-name type-plug-wifi
  - aws iot create-policy --policy-name plug-policy --policy-document '{"Version": "2012-10-17","Statement": [{"Effect": "Allow","Action": ["iot:Connect"],"Resource": ["arn:aws:iot:us-east-2:179944132799:client/${iot:Connection.Thing.ThingName}"]},{"Effect": "Allow","Action": ["iot:Publish"],"Resource": ["arn:aws:iot:us-east-2:179944132799:topic/$aws/things/${iot:Connection.Thing.ThingName}/shadow/update"]},{"Effect": "Allow","Action": ["iot:Subscribe"],"Resource": ["arn:aws:iot:us-east-2:179944132799:topicfilter/$aws/things/${iot:Connection.Thing.ThingName}/shadow/update/delta"]},{"Effect": "Allow","Action": ["iot:Receive"],"Resource": ["*"]},{"Effect": "Allow","Action": ["iot:UpdateThingShadow"],"Resource": ["arn:aws:iot:us-east-2:179944132799:thing/${iot:Connection.Thing.ThingName}"]}]}'
  - aws iot create-thing-group --thing-group-name plug-group
  - aws iot attach-policy --policy-name plug-policy --target arn:aws:iot:us-east-2:179944132799:thinggroup/plug-group

## Provisioning:

#### NOTES: If you want to provision a plug that is provisioned already, delete its associated certificate first from the aws console as the private key will be changed.

- Connect to the device via Atmel-Ice, and insert the SD
  
```bash
./provision.sh
```

## Config Templates:

- hyukia.txt is one example of config templates. Make sure the can bus message ids (.can.bus[].status[].id) are in ascending order

- to generate a config file after the template file is changed
  
```bash
export TEMPLATE=./config/hyukia.txt; export DEVICE_ID=0123836A0984CB6FEE; ./generateConfig.sh
```