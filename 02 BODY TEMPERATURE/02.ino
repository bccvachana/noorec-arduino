#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Adafruit_MLX90614.h>
#include <NewPing.h>

#define TRIGGER_PIN 6
#define ECHO_PIN 5
#define MAX_DISTANCE 200

const uint8_t pinCE = 7;
const uint8_t pinCSN = 8;
RF24 wirelessSPI(pinCE, pinCSN);
const uint64_t wAddress = 0xB00B1E50C3LL;
const uint8_t rFChan = 89;
const uint8_t rDelay = 7;
const uint8_t rNum = 5;

struct PayLoad
{
  int channel = 2;
  int heightValue;
  float temperatureValue;
  int bloodPressureHighValue;
  int bloodPressureLowValue;
  int rateValue;
  int oxygenValue;
};
PayLoad payload;

Adafruit_MLX90614 mlx = Adafruit_MLX90614();
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

int time = 0;
float temperature = 0;
float temperatureAvg = 0;
bool isTemperature = false;

void setup()
{
  wirelessSPI.begin();
  wirelessSPI.setChannel(rFChan);
  wirelessSPI.setRetries(rDelay, rNum);
  wirelessSPI.openWritingPipe(wAddress);
  wirelessSPI.stopListening();

  Serial.begin(9600);
  pinMode(3, OUTPUT);
  mlx.begin();
}

void loop()
{
  if (!isTemperature)
  {
    if (sonar.ping_cm() > 0 && sonar.ping_cm() < 20)
    {
      digitalWrite(4, HIGH);
      temperature = temperature + mlx.readObjectTempC() + 2.50;
      time++;
    }
    else
    {
      digitalWrite(4, LOW);
      time = 0;
      temperature = 0;
      temperatureAvg = 0;
    }
    Serial.print("...");
    Serial.println(time);
    delay(100);
  }
  else
  {
    temperatureAvg = temperature / 25.00;
    payload.temperatureValue = temperatureAvg;
    Serial.print("...temperatureAvg...");
    Serial.println(temperatureAvg);
    wirelessSPI.write(&payload, sizeof(payload));
    time = 0;
    temperature = 0;
    temperatureAvg = 0;
    isTemperature = false;
  }

  if (time == 25)
  {
    isTemperature = true;
    digitalWrite(4, LOW);
    delay(200);
    digitalWrite(4, HIGH);
    delay(100);
    digitalWrite(4, LOW);
    delay(200);
    digitalWrite(4, HIGH);
    delay(100);
    digitalWrite(4, LOW);
    delay(200);
    digitalWrite(4, HIGH);
    delay(100);
    digitalWrite(4, LOW);
    delay(200);
    digitalWrite(4, HIGH);
    time = 0;
  }
}