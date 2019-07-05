# Plug Provision

This package automatically

- Generates a private key on the usb-connected plug
- Sends a CSR to AWS
- Stores resulting certificate and configuration to ./config/\${PLUG_ID}/config.txt that that you can copy to plug microSD.

## Preparation (should work on Windows, Linux, and MacOS):

- Install git (or git bash for windows, https://gitforwindows.org/)
- Install Python 2.7 https://www.python.org (on Windows, add python.exe to Path when using the installer)
- Install Visual Studio Code (https://code.visualstudio.com/download)
- In VSCode, install PlatformIO IDE extension (File -> Preferences -> Extensions)
- In VSCode Terminal (on Windows, make sure Default Shell is bash)
  - install aws-cli `pip install awscli`
  - login aws `aws configure`
  - Git clone WaiveTelem `git clone git@github.com:WaiveCar/WaiveTelem.git`
- Review plug-type, plug-group, and plug policy on AWS IoT console as they should be considered carefully. They were created with the following:
  - aws iot create-thing-type --thing-type-name plug-type
  - aws iot create-policy --policy-name AllowEverything --policy-document "{\"Version\": \"2012-10-17\",\"Statement\": [{\"Effect\": \"Allow\",\"Action\": [\"iot:*\"],\"Resource\": [\"*\"]}]}"
  - aws iot create-thing-group --thing-group-name plug-group
  - aws iot attach-policy --policy-name AllowEverything --target arn:aws:iot:us-east-2:179944132799:thinggroup/plug-group

## Provisioning:

#### NOTES: If you want to provision a plug that is provisioned already, delete its associated certificate first from the aws console as the private key will be changed.

- In VSCode, File -> Open Folder WaiveTelem/packages/plug-provision
- Connect to the plug via USB
- Open run.sh, update PLUG_ID
- Open VSCode Terminal, allow this workspace to modifiy the terminal shell when prompted, exit Terminal
- Open VSCode Terminal again, pio CLI should be available now, run

```bash
./run.sh
```

- Open ./config/\${PLUG_ID}/config.txt to adjust the configuration if necessary
- Copy the config.txt file to the plug microSD
