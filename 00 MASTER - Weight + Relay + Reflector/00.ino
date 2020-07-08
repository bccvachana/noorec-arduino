#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <HX711.h>

// Relay & Reflector
#define relayPin 6
#define reflectorPin A0

// Weight
#define DOUT 3
#define CLK 2
HX711 scale;
float calibration_factor = -21500;

const uint8_t pinCE = 7;
const uint8_t pinCSN = 8;
RF24 wirelessSPI(pinCE, pinCSN);
const uint64_t rAddress = 0xB00B1E50C3LL;
const uint8_t rFChan = 89;

struct PayLoad
{
	int channel;
	int heightValue;
	float temperatureValue;
	int bloodPressureHighValue;
	int bloodPressureLowValue;
	int rateValue;
	int oxygenValue;
};
PayLoad payload;

char input = '0';
int time = 0;
String data = "";
float weightValue;
float weightTempNew;
float weightTempOld;
float weightSum;
int weightTime;
int heightValue;
bool isWeight = false;
bool isHeight = false;
float temperatureValue;
bool isData = false;
bool isBloodPressure = false;
bool isBloodPressureReceive = false;
int bloodPressureHighValue;
int bloodPressureLowValue;
int rateValue;
int oxygenValue;

void setup()
{
	wirelessSPI.begin();
	wirelessSPI.setChannel(rFChan);
	wirelessSPI.openReadingPipe(1, rAddress);
	wirelessSPI.startListening();
	Serial.begin(9600);
	Serial.println("NooRec Master is online...");

	scale.begin(DOUT, CLK);
	scale.set_scale(calibration_factor);
	scale.tare();

	pinMode(relayPin, OUTPUT);
	pinMode(reflectorPin, INPUT);
	digitalWrite(relayPin, HIGH);
}

void loop()
{
	if (Serial.available())
	{
		input = Serial.readString()[0];
		reset();
	}

	if (input == '0')
	{
		Serial.println("...");
		delay(1000);
	}

	else if (input == '1')
	{
		// if (wirelessSPI.available())
		// {
		// 	wirelessSPI.read(&payload, sizeof(payload));
		// 	if (payload.channel == 1)
		// 	{
		// 		if (payload.heightValue)
		// 		{
		// 			Serial.println(payload.heightValue);
		// 		}
		// 	}
		// }
		// delay(1000);

		if (!isData)
		{
			if (!isHeight)
			{
				if (wirelessSPI.available())
				{
					wirelessSPI.read(&payload, sizeof(payload));
					if (payload.channel == 1)
					{
						if (payload.heightValue)
						{
							heightValue = payload.heightValue;
							isHeight = true;
						}
					}
				}
			}
			if (!isWeight)
			{
				weightTempNew = scale.get_units();

				if (weightTempNew > 20 && abs(weightTempNew - weightTempOld) < 0.3)
				{
					weightSum += weightTempNew;
					weightTime++;
				}
				else
				{
					weightSum = 0;
					weightTime = 0;
				}

				if (weightTime == 30)
				{
					weightValue = weightSum / 30;
					weightSum = 0;
					weightTime = 0;
					isWeight = true;
				}

				weightTempOld = weightTempNew;
			}

			if (isWeight && isHeight)
			{
				isData = true;
			}

			data = "loading,weightHeight";

			delay(100);
			time++;
			if (time == 10)
			{
				Serial.println(data);
				time = 0;
			}
		}
		else
		{
			data = "done,weightHeight," + String(weightValue, 1) + "," + String(heightValue);
			delay(1000);
			Serial.println(data);
		}
	}

	else if (input == '2')
	{

		if (!isData)
		{
			if (wirelessSPI.available())
			{
				wirelessSPI.read(&payload, sizeof(payload));
				if (payload.channel == 2)
				{
					if (payload.temperatureValue > 0)
					{
						temperatureValue = payload.temperatureValue;
						isData = true;
					}
				}
			}
			data = "loading,temperature";
		}
		else
		{
			data = "done,temperature," + String(temperatureValue, 1);
		}
		time++;
		Serial.println(data);
		delay(1000);
	}

	else if (input == '3')
	{
		if (!isData)
		{
			if (analogRead(reflectorPin) < 100 && !isBloodPressure)
			{
				time++;
			}
			if (analogRead(reflectorPin) > 100 && !isBloodPressure)
			{
				time = 0;
			}
			if (analogRead(reflectorPin) < 100 && !isBloodPressure && time > 10)
			{
				isBloodPressure = true;
			}
			if (analogRead(reflectorPin) > 100 && isBloodPressure)
			{
				isBloodPressure = false;
				isBloodPressureReceive = true;
				digitalWrite(relayPin, LOW);
			}

			if (isBloodPressureReceive)
			{
				if (wirelessSPI.available())
				{
					wirelessSPI.read(&payload, sizeof(payload));
					if (payload.channel == 3)
					{
						if (payload.bloodPressureHighValue > 0 && payload.bloodPressureLowValue > 0)
						{
							bloodPressureHighValue = payload.bloodPressureHighValue;
							bloodPressureLowValue = payload.bloodPressureLowValue;
							digitalWrite(relayPin, HIGH);
							isData = true;
						}
					}
				}
			}

			data = "loading,bloodPressure";
		}
		else
		{
			data = "done,bloodPressure," + String(bloodPressureHighValue) + "," + String(bloodPressureLowValue);
		}
		Serial.println(data);
		delay(1000);
	}

	else if (input == '4')
	{
		if (!isData)
		{
			if (time == 1)
			{
				digitalWrite(relayPin, LOW);
			}

			if (wirelessSPI.available())
			{
				wirelessSPI.read(&payload, sizeof(payload));
				if (payload.channel == 3)
				{
					if (payload.rateValue > 0 && payload.oxygenValue > 0)
					{
						rateValue = payload.rateValue;
						oxygenValue = payload.oxygenValue;
						digitalWrite(relayPin, HIGH);
						isData = true;
					}
				}
			}
			data = "loading,rateOxygen";
		}
		else
		{
			data = "done,rateOxygen," + String(rateValue) + "," + String(oxygenValue);
		}
		time++;
		Serial.println(data);
		delay(1000);
	}
}

void reset()
{
	time = 0;
	data = "";
	isData = false;
	weightValue = 0;
	weightTempNew = 0;
	weightTempOld = 0;
	weightSum = 0;
	weightTime = 0;
	heightValue = 0;
	isWeight = false;
	isHeight = false;
	temperatureValue = 0;
	isBloodPressureReceive = false;
	digitalWrite(relayPin, HIGH);
	bloodPressureHighValue = 0;
	bloodPressureLowValue = 0;
	rateValue = 0;
	oxygenValue = 0;
}