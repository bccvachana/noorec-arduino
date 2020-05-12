#include <eHealth.h>
#include <PinChangeInt.h>
#include <EasyTransfer.h>

#define relayPin 2

struct DATA_STRUCTURE
{
  int bloodPressureHighValue;
  int bloodPressureLowValue;
  int rateValue;
  int oxygenValue;
};
EasyTransfer ET;
DATA_STRUCTURE data;

int cont = 0;
int time = 0;
bool rateOxygen = false;

void setup()
{
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  delay(3500);
  eHealth.readBloodPressureSensor();

  uint8_t numberOfData = eHealth.getBloodPressureLength();
  data.bloodPressureHighValue = 30 + eHealth.bloodPressureDataVector[numberOfData - 1].systolic;
  data.bloodPressureLowValue = eHealth.bloodPressureDataVector[numberOfData - 1].diastolic;
  delay(100);
  digitalWrite(4, LOW);

  Serial.begin(9600);

  delay(3000);
  eHealth.initPulsioximeter();
  PCintPort::attachInterrupt(6, readPulsioximeter, RISING);
  ET.begin(details(data), &Serial);
  digitalWrite(relayPin, LOW);
  delay(1000);
}

void loop()
{
  if (!rateOxygen)
  {
    data.rateValue = eHealth.getBPM();
    data.oxygenValue = eHealth.getOxygenSaturation();
    if (data.rateValue > 0 && data.oxygenValue > 0)
    {
      rateOxygen = true;
    }
  }
  ET.sendData();

  delay(1000);
}

void readPulsioximeter()
{

  cont++;

  if (cont == 50)
  {
    eHealth.readPulsioximeter();
    cont = 0;
  }
}