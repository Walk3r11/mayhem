const int ledPin = 2;
const int relayPin = 12;

void setup()
{
    pinMode(ledPin, OUTPUT);
    pinMode(relayPin, OUTPUT);

    digitalWrite(ledPin, LOW);
    digitalWrite(relayPin, LOW);

    delay(1000);

    digitalWrite(ledPin, HIGH);

    digitalWrite(relayPin, HIGH);
}

void loop()
{
}