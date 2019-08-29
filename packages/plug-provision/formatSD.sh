#!/bin/bash

echo ""
echo "Running formatSD.sh"

case "$(uname -s)" in
   Darwin)
      read -p "Formating SD-Card at /dev/disk2, press ENTER to continue"
      diskutil eraseDisk FAT32 SDCARD MBRFormat /dev/disk2
      ;;
   *)
      echo "Formating SD-Card at E:"
      format.com E: //FS:FAT32 //Q //V:SDCARD
      ;;
esac
