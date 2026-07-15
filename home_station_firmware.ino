#define LED_BUILTIN 2

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("La LED est allumee");
  delay(1000);

  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("La LED est etteinte");
  delay(1000);
}