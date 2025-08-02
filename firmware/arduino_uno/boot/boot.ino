const int green_led = 8;
const int red_led = 9;

void setup()
{
  pinMode(green_led, OUTPUT);
  pinMode(red_led, OUTPUT);

  digitalWrite(green_led, HIGH);
  digitalWrite(red_led, LOW);
}

void loop()
{

}