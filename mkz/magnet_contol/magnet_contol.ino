// Pins that take data from RPi to MKZ
// These represent a 4-bit number
#define	PIN_OL0	6
#define	PIN_OL1	7
#define	PIN_OL2	8
#define	PIN_OL3	9

// Pins that take data from MKZ to RPi
// These represent a 6-bit number
#define PIN_IL0	10
#define PIN_IL1	11
#define PIN_IL2	12
#define PIN_IL3	13
#define PIN_IL4	14
#define PIN_IL5	A1

// Switches and buttons
#define	PIN_SW_ENABLE	0
#define	PIN_SW_UI	5

// Indicator LEDs
#define	PIN_LED0	LED_BUILTIN
#define	PIN_LED1	2
#define	PIN_LED2	3
#define	PIN_LED3	4

// Position Sensor
#define	PIN_SENSOR	A2

// Magnet Control
#define	PIN_MAGNET	A5

// RPi Clocking
#define	PIN_CLOCK	1

void setup() {
  pinMode(PIN_OL0, INPUT);
  pinMode(PIN_OL1, INPUT);
  pinMode(PIN_OL2, INPUT);
  pinMode(PIN_OL3, INPUT);

  pinMode(PIN_IL0, OUTPUT);
  pinMode(PIN_IL1, OUTPUT);
  pinMode(PIN_IL2, OUTPUT);
  pinMode(PIN_IL3, OUTPUT);
  pinMode(PIN_IL4, OUTPUT);
  pinMode(PIN_IL5, OUTPUT);

  pinMode(PIN_SW_ENABLE, INPUT_PULLUP);
  pinMode(PIN_SW_UI, INPUT_PULLUP);

  pinMode(PIN_LED0, OUTPUT);
  pinMode(PIN_LED1, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);
  pinMode(PIN_LED3, OUTPUT);

  pinMode(PIN_SENSOR, INPUT);
  pinMode(PIN_MAGNET, OUTPUT);
  pinMode(PIN_CLOCK, OUTPUT);

  analogReadResolution(12);
  (void)analogRead(PIN_SENSOR);		// first read initializes some stuff

  // Turn on the built-in LED.  We are awake!
  digitalWrite(PIN_LED0, HIGH);
}

#define	ADC_MAX	62	// biggest possible sensor output value

uint32_t loop_t;
uint8_t command;
uint8_t value;
uint16_t min_adc_value;
uint16_t adc_scale;
uint8_t enabled;

void getCommand() {
  command = 0;
  if (digitalRead(PIN_OL0))
    command = 0x1;
  if (digitalRead(PIN_OL1))
    command |= 0x2;
  if (digitalRead(PIN_OL2))
    command |= 0x4;
  if (digitalRead(PIN_OL3))
    command |= 0x8;
#ifdef PIN_OL4
  if (digitalRead(PIN_OL4))
    command |= 0x10;
#endif
}

//
// Read the ADC pin and convert it to an output value
void getValue() {
  if (!enabled) {
    value = 0;
    return;
  }

  value = analogRead(PIN_SENSOR);
  if (value < min_adc_value)
    value = 0;
  else
    value -= min_adc_value;

  if (value > adc_scale)
    value = ADC_MAX;
  else
    value = value * ADC_MAX / adc_scale;
  if (value > ADC_MAX)
    value = ADC_MAX;
  value += 1;
}

//
// This routine turns on the magnet, and arranged for an interrupt to tell us when to turn if off.
//
void setMagnet()
{
  if (!enabled || command == 0) {
    digitalWrite(PIN_MAGNET, LOW);
    return;
  }

  // Actual magnet control NYI
}

//
// Set value onto output pins
//
void setValue() {
  if (value & 0x1)
    digitalWrite(PIN_IL0, HIGH);
  else
    digitalWrite(PIN_IL0, LOW);
  if (value & 0x2)
    digitalWrite(PIN_IL1, HIGH);
  else
    digitalWrite(PIN_IL1, LOW);
  if (value & 0x4)
    digitalWrite(PIN_IL2, HIGH);
  else
    digitalWrite(PIN_IL2, LOW);
  if (value & 0x8)
    digitalWrite(PIN_IL3, HIGH);
  else
    digitalWrite(PIN_IL3, LOW);
  if (value & 0x10)
    digitalWrite(PIN_IL4, HIGH);
  else
    digitalWrite(PIN_IL4, LOW);
  if (value & 0x20)
    digitalWrite(PIN_IL5, HIGH);
  else
    digitalWrite(PIN_IL5, LOW);
}

// Phase Timing.  All times in microseconds
#define	P1_TIME		50
#define	P2_TIME		100
#define	A_SAMPLE_TIME	10
#define MAX_TIME (4000ul*1000000ul)

void loop() {
  loop_t = micros();

  if (loop_t > MAX_TIME) {
    // turn off all LEDs
    digitalWrite(PIN_LED0, LOW);
    digitalWrite(PIN_LED1, HIGH);
    digitalWrite(PIN_LED2, HIGH);
    digitalWrite(PIN_LED3, HIGH);

    for (;;) ;	// give up.
  }

  enabled = (digitalRead(PIN_SW_ENABLE) == 0);

  // Start by lowering clock and sampling the command
  digitalWrite(PIN_CLOCK, LOW);
  getCommand();

  // Now that we have the command, set the magnet
  setMagnet();

  // Wait to sample ADC
  while (micros() - loop_t < (P1_TIME - A_SAMPLE_TIME)) ;

  // Get the sensor value
  getValue();
  setValue();

  // Wait for P1
  while (micros() - loop_t < P1_TIME) ;

  // Signal the RPi
  digitalWrite(PIN_CLOCK, HIGH);

  // Wait for P2
  while (micros() - loop_t < P2_TIME) ;
}
