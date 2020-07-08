#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <EasyTransfer.h>

const uint8_t pinCE = 7;
const uint8_t pinCSN = 8;
RF24 wirelessSPI(pinCE, pinCSN);
const uint64_t wAddress = 0xB00B1E50C3LL;
const uint8_t rFChan = 89;
const uint8_t rDelay = 7;
const uint8_t rNum = 5;

struct PayLoad
{
    int channel = 3;
    int heightValue;
    float temperatureValue;
    int bloodPressureHighValue;
    int bloodPressureLowValue;
    int rateValue;
    int oxygenValue;
};
PayLoad payload;

struct DATA_STRUCTURE
{
    int bloodPressureHighValue;
    int bloodPressureLowValue;
    int rateValue;
    int oxygenValue;
};
EasyTransfer ET;
DATA_STRUCTURE data;

bool isBloodPressure = false;
bool isRateOxygen = false;

void setup()
{
    wirelessSPI.begin();
    wirelessSPI.setChannel(rFChan);
    wirelessSPI.setRetries(rDelay, rNum);
    wirelessSPI.openWritingPipe(wAddress);
    wirelessSPI.stopListening();
    ET.begin(details(data), &Serial);
    Serial.begin(9600);
}

void loop()
{
    if (ET.receiveData())
    {
        payload.bloodPressureHighValue = data.bloodPressureHighValue;
        payload.bloodPressureLowValue = data.bloodPressureLowValue;
        payload.rateValue = data.rateValue;
        payload.oxygenValue = data.oxygenValue;
    }

    Serial.print(payload.bloodPressureHighValue);
    Serial.print(" , ");
    Serial.print(payload.bloodPressureLowValue);
    Serial.print(" , ");
    Serial.print(payload.rateValue);
    Serial.print(" , ");
    Serial.println(payload.oxygenValue);

    if (payload.bloodPressureHighValue > 0 && payload.bloodPressureLowValue > 0 && !isBloodPressure)
    {
        payload.rateValue = 0;
        payload.oxygenValue = 0;
        wirelessSPI.write(&payload, sizeof(payload));
        isBloodPressure = true;
    }

    if (payload.rateValue > 0 && payload.rateValue < 300 && payload.oxygenValue > 0 && payload.oxygenValue <= 100 && !isRateOxygen)
    {
        payload.bloodPressureHighValue = 0;
        payload.bloodPressureLowValue = 0;
        wirelessSPI.write(&payload, sizeof(payload));
        isRateOxygen = true;
    }

    delay(1000);
}