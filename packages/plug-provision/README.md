# Plug Provision

This package automatically

- Generates a private key on the usb-connected plug
- Sends a CSR to AWS
- Stores resulting certificate and configuration to the inserted microSD that you can use on the plug.

## Preparation (should work on Windows, Linux, and MacOS):
#### NOTES: The steps assume that you have bash installed. On Windows, We recommend installing git-bash (see https://stackoverflow.com/questions/42606837/how-do-i-use-bash-on-windows-from-the-visual-studio-code-integrated-terminal, https://stackoverflow.com/questions/53015630/bash-aws-command-not-found-on-windows-7-in-git-bash) or adjust the shell scripts for DOS CMD.

- Install aws-cli (pip install awscli)
- Install Visual Studio Code (https://code.visualstudio.com/download)
- Install PlatformIO IDE extension (Code -> Preferences -> Extensions) in VSCode
- Git clone WaiveTelem
- Review plug-type, plug-group, and plug policy as they should be considered carefully. They were created with the following:
  - aws iot create-thing-type --thing-type-name plug-type
  - aws iot create-policy --policy-name AllowEverything --policy-document "{\"Version\": \"2012-10-17\",\"Statement\": [{\"Effect\": \"Allow\",\"Action\": [\"iot:*\"],\"Resource\": [\"*\"]}]}"
  - aws iot create-thing-group --thing-group-name plug-group
  - aws iot attach-policy --policy-name AllowEverything --target arn:aws:iot:us-east-2:179944132799:thinggroup/plug-group

## Provisioning:
#### NOTES: If you want to provision a plug that is provisioned already, delete its associated certificate first from the aws console as the private key will be changed.
- In VSCode Terminal, run `aws configure` if you are not logged-in aws for aws-cli
- File -> Open ./packages/plug-provision folder in VSCode
- Connect to the plug via USB, insert a microSD to the PC
- In VSCode Terminal

```bash
export PLUG_ID=plug-1
export SD_PATH=/Volumes/TELEMCONFIG/
export MQTT_BROKER=a2ink9r2yi1ntl-ats.iot.us-east-2.amazonaws.com
./run.sh
```

- Open config.txt on the microSD to adjust the configuration if necessary
- Insert the microSD to the plug
