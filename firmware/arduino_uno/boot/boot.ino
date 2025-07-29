const int relayPin = 7;
const int green_led = 8;
const int red_led = 9;
const int yellow_led = 10;
const int sensorPin = A0;

void setup()
{
  pinMode(relayPin, OUTPUT);
  pinMode(green_led, OUTPUT);
  pinMode(red_led, OUTPUT);
  pinMode(yellow_led, OUTPUT);

  digitalWrite(relayPin, HIGH);
  digitalWrite(green_led, HIGH);
  digitalWrite(red_led, LOW);
  digitalWrite(yellow_led, LOW);
}

void loop()
{
  int sensorValue = analogRead(sensorPin);
  float voltage = sensorValue * (5.0 / 1023.0);

  Serial.print("Voltage: ");
  Serial.println(voltage);

  int flickerDelay = 0;
  if (voltage >= 4.0)
  {
    flickerDelay = 100;
  }
  else if (voltage >= 2.5)
  {
    flickerDelay = 500;
  }
  else
  {
    flickerDelay = 1000;
  }

  digitalWrite(yellow_led, HIGH);
  delay(flickerDelay);
  digitalWrite(yellow_led, LOW);
  delay(flickerDelay);
}
