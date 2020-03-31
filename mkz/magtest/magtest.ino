#define	MAGNET_PIN	A6

static char lbuf[128];
static uint8_t nbytes;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(MAGNET_PIN, OUTPUT);
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Type \"on\" or \"off\"");
  nbytes = 0;
}

static void on() {
  Serial.println("Turning ON");
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(MAGNET_PIN, HIGH);
}

static void off() {
  Serial.println("Turning Off");
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(MAGNET_PIN, LOW);
}

static void process_line() {
  if (strcmp(lbuf, "on") == 0)
    on();
  else if (strcmp(lbuf , "off") == 0)
    off();
  else {
    Serial.print("Bad command: ");
    Serial.println(lbuf);
  }
}

void loop() {
  int c;

  if (Serial.available()) {
    c = Serial.read();
    if (c == '\n') {
      lbuf[nbytes] = '\0';
      process_line();
      nbytes = 0;
      return;
    }
    if (c >= 0 && nbytes < sizeof lbuf) {
      if (c >= 'A' && c <= 'Z')
        c += 'a' - 'A';
      lbuf[nbytes++] = c;
    }
  }
}
