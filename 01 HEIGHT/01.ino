#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <HCSR04.h>
#include <NewPing.h>

#define TRIGGER_PIN 6
#define ECHO_PIN 5
#define MAX_DISTANCE 250

const uint8_t pinCE = 7;
const uint8_t pinCSN = 8;
RF24 wirelessSPI(pinCE, pinCSN);
const uint64_t wAddress = 0xB00B1E50C3LL;
const uint8_t rFChan = 89; //Set channel default (chan 84 is 2.484GHz to 2.489GHz)
const uint8_t rDelay = 7;  //this is based on 250us increments, 0 is 250us so 7 is 2 ms
const uint8_t rNum = 5;    //number of retries that will be attempted

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

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

int time = 0;
bool isHeight = false;
int heightBefore, heightTemp, heightSum, height;

void setup()
{
    wirelessSPI.begin();
    wirelessSPI.setChannel(rFChan);
    wirelessSPI.setRetries(rDelay, rNum);  //if a transmit fails to reach receiver (no ack packet) then this sets retry attempts and delay between retries
    wirelessSPI.openWritingPipe(wAddress); //open writing or transmit pipe
    wirelessSPI.stopListening();           //go into transmit mode

    Serial.begin(9600);
    heightBefore = 212;
}

void loop()
{
    if (!isHeight)
    {
        heightTemp = heightBefore - sonar.ping_cm();
        if (heightTemp > 120)
        {
            time++;
            heightSum += heightTemp;
        }
        else
        {
            time = 0;
            heightSum = 0;
        }
        if (time == 3)
        {
            height = heightSum / 3;
            time = 0;
            heightSum = 0;
            isHeight = true;
        }
        Serial.println(heightTemp);
    }
    else
    {
        Serial.print("...yayyyyy... ");
        Serial.println(height);
        time++;

        if (time == 2)
        {
            payload.heightValue = height;
            wirelessSPI.write(&payload, sizeof(payload));
        }

        if (time == 15)
        {
            isHeight = false;
        }
    }

    delay(1000);
}