#!/usr/local/bin/expect -f
set timeout 10
log_user 0 
spawn pio device monitor --quiet
log_user 1
expect "END CERTIFICATE REQUEST-----"
close