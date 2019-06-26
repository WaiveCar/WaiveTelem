
//declare immo file name
const char *immofile = "/immo.txt";


void unlock_lock_doors(int pin) {
  digitalWrite(pin, HIGH);
  delay(100);
  digitalWrite(pin, LOW);
}


void immobilize(bool &immobilized, int immo_pin) {
  digitalWrite(immo_pin, LOW);
  immobilized = true;
  saveImmobilizer(immofile, immobilized);
}

void unimmobilize(bool &immobilized, int immo_pin) {
  digitalWrite(immo_pin, HIGH);
  immobilized = false;
  saveImmobilizer(immofile, immobilized);
}
