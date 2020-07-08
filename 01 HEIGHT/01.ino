#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <HCSR04.h>
#include <NewPing.h>

#define MAX_DISTANCE 300

const uint8_t pinCE = 7;
const uint8_t pinCSN = 8;
RF24 wirelessSPI(pinCE, pinCSN);
const uint64_t wAddress = 0xB00B1E50C3LL;
const uint8_t rFChan = 89;
const uint8_t rDelay = 7;
const uint8_t rNum = 5;

struct PayLoad
{
    int channel = 1;
    int heightValue;
    float temperatureValue;
    int bloodPressureHighValue;
    int bloodPressureLowValue;
    int rateValue;
    int oxygenValue;
};
PayLoad payload;

NewPing sonar(2, 3, MAX_DISTANCE);

int time = 0;
bool isHeight = false;
int heightBefore, heightTempNew, heightTempOld, heightSum, height;

void setup()
{
    wirelessSPI.begin();
    wirelessSPI.setChannel(rFChan);
    wirelessSPI.setRetries(rDelay, rNum);
    wirelessSPI.openWritingPipe(wAddress);
    wirelessSPI.stopListening();

    Serial.begin(9600);
    heightBefore = 210;
}

void loop()
{
    // payload.heightValue = heightBefore - sonar.ping_cm();
    // wirelessSPI.write(&payload, sizeof(payload));
    // delay(1000);
    if (!isHeight)
    {
        heightTempNew = heightBefore - sonar.ping_cm();

        Serial.println(heightTempNew);

        if (heightTempNew != heightBefore && heightTempNew > 120 && abs(heightTempNew - heightTempOld) < 5)
        {
            heightSum += heightTempNew;
            time++;
        }
        else
        {
            if (heightTempNew != heightBefore)
            {
                heightSum = 0;
                time = 0;
            }
        }
        if (time == 3)
        {
            height = heightSum / 3;
            isHeight = true;
            heightSum = 0;
            time = 0;
        }
        if (heightTempNew != heightBefore)
        {
            heightTempOld = heightTempNew;
        }

        delay(50);
    }
    else
    {
        if (time == 2)
        {
            Serial.print(":::::::::: ");
            payload.heightValue = height;
            wirelessSPI.write(&payload, sizeof(payload));
        }

        Serial.println(height);
        time++;

        if (time == 15)
        {
            Serial.println("::::::::::");
            isHeight = false;
        }
        delay(1000);
    }
}