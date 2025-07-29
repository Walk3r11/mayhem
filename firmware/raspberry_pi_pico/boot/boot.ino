#define GREEN_LED 12
#define RED_LED 13
#define SPEAKER_PIN 15

void setup()
{
  pinMode(GREEN_LED, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT);

  digitalWrite(GREEN_LED, HIGH);
  playSuccessTone();
}

void loop()
{
}

void playSuccessTone()
{
  tone(SPEAKER_PIN, 880, 200);
  delay(250);
  tone(SPEAKER_PIN, 988, 200);
  delay(250);
  tone(SPEAKER_PIN, 1047, 200);
  delay(250);
  noTone(SPEAKER_PIN);
}