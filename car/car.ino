//////////////  DEFINITIONS / CONFIGS  //////////////////////////////////////

#define RECEIVER_PIN 12
#define OS_GAIN_REFRESH_DELAY 0
/* Gain refresh time of SRX882 module is around 100 milliseconds.
   If only one pair of SRX and STX are used to connect 2 devices in SIMPLEX
   mode, there is no need to refresh receiver's gain, being communication
   mono-directional. */

#define MOTO_DRIVER_I2C_ADDR 0x30
#define MOTO_DRIVER_PWM_FREQ 1000



//////////////  INCLUDES  //////////////////////////////////////
#include <PJON.h>
#include "WEMOS_Motor.h"



// <Strategy name> bus(selected device id)
PJON<OverSampling> bus(44);

//Motor shiled I2C Address: 0x30
//PWM frequency: 1000Hz(1kHz)
Motor M1(MOTO_DRIVER_I2C_ADDR,_MOTOR_A, MOTO_DRIVER_PWM_FREQ);//Motor A
Motor M2(MOTO_DRIVER_I2C_ADDR,_MOTOR_B, MOTO_DRIVER_PWM_FREQ);//Motor B


void setup() {
  bus.set_communication_mode(PJON_SIMPLEX);
  bus.strategy.set_pins(RECEIVER_PIN , PJON_NOT_ASSIGNED);
  bus.begin();

  bus.set_receiver(receiver_function);

  Serial.begin(115200);
};

void loop() {
	CAR_CONTROL();
	PJON_RECV();
}


void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info) {
 // Do nothing to avoid affecting speed analysis
}


int pwm;
void CAR_CONTROL(){
	for (pwm = 20; pwm <= 100; pwm++)
  {
    M1.setmotor( _CW, pwm);
    M2.setmotor(_CW, 100-pwm);
    Serial.printf("A:%d%, B:%d%, DIR:CW\r\n", pwm,100-pwm);
  }

  M1.setmotor(_STOP);
  M2.setmotor( _STOP);
  Serial.println("Motor A&B STOP");
  delay(200);

  for (pwm = 20; pwm <=100; pwm++)
  {
    M1.setmotor(_CCW, pwm);
    M2.setmotor(_CCW, 100-pwm);
    Serial.printf("A:%d%, B:%d%, DIR:CCW\r\n", pwm,100-pwm);

  }

  M1.setmotor(_STOP);
  M2.setmotor( _STOP);
  delay(200);
  Serial.println("Motor A&B STOP");

  M1.setmotor(_SHORT_BRAKE);
  M2.setmotor( _SHORT_BRAKE);
  Serial.println("Motor A&B SHORT BRAKE");
  delay(1000);

  M1.setmotor(_STANDBY);
  M2.setmotor( _STANDBY);
  Serial.println("Motor A&B STANDBY");
  delay(1000);
}



float test;
float mistakes;
int busy;
int fail;

void PJON_RECV(){
  Serial.println("Starting 1 second communication speed test...");
  long time = millis();
  unsigned int response = 0;
  while(millis() - time < 1000) {
    response = bus.receive();
    if(response == PJON_ACK)
      test++;
    if(response == PJON_NAK)
      mistakes++;
    if(response == PJON_BUSY)
      busy++;
    if(response == PJON_FAIL)
      fail++;
  }

  Serial.print("Absolute com speed: ");
  Serial.print(test * 25);
  Serial.println("B/s");
  Serial.print("Practical bandwidth: ");
  Serial.print(test * 20);
  Serial.println("B/s");
  Serial.print("Packets sent: ");
  Serial.println(test);
  Serial.print("Mistakes (error found with CRC) ");
  Serial.println(mistakes);
  Serial.print("Fail (no answer from receiver) ");
  Serial.println(fail);
  Serial.print("Busy (Channel is busy or affected by interference) ");
  Serial.println(busy);
  Serial.print("Accuracy: ");
  Serial.print(100 - (100 / (test / mistakes)));
  Serial.println(" %");
  Serial.println(" --------------------- ");

  test = 0;
  mistakes = 0;
  busy = 0;
  fail = 0;
};