
/*
* Getting Started example sketch for nRF24L01+ radios
* This is a very basic example of how to send data from one node to another
* Updated: Dec 2014 by TMRh20
*/

#include <SPI.h>
#include "RF24.h"

/****************** User Config ***************************/
/***      Set this radio as radio number 0 or 1         ***/
bool radioNumber = 1;

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
// RF24 radio(D2,D8);
RF24 radio(D8,D2);
/**********************************************************/

byte addresses[][6] = {"1Node","2Node"};

// Used to control whether this node is sending or receiving
bool role = 0;

void setup() {
  Serial.begin(115200);
  Serial.println(F("RF24/examples/GettingStarted"));
  Serial.println(F("*** PRESS 'T' to begin transmitting to the other node"));

  radio.begin();

  // Set the PA Level low to prevent power supply related issues since this is a
 // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  radio.setPALevel(RF24_PA_MAX);

  // Open a writing and reading pipe on each radio, with opposite addresses
  if(radioNumber){
    radio.openWritingPipe(addresses[1]);
    radio.openReadingPipe(1,addresses[0]);
  }else{
    radio.openWritingPipe(addresses[0]);
    radio.openReadingPipe(1,addresses[1]);
  }

  // Start the radio listening for data
  radio.startListening();
}

void loop() {


/****************** Ping Out Role ***************************/
if (role == 1)  {

    radio.stopListening();                                    // First, stop listening so we can talk.


    Serial.println(F("Now sending"));

    unsigned long start_time = micros();                             // Take the time, and send it.  This will block until complete
     if (!radio.write( &start_time, sizeof(unsigned long) )){
       Serial.println(F("failed"));
     }

    radio.startListening();                                    // Now, continue listening

    unsigned long started_waiting_at = micros();               // Set up a timeout period, get the current microseconds
    boolean timeout = false;                                   // Set up a variable to indicate if a response was received or not

    while ( ! radio.available() ){                             // While nothing is received
      if (micros() - started_waiting_at > 200000 ){            // If waited longer than 200ms, indicate timeout and exit while loop
          timeout = true;
          break;
      }
    }

    if ( timeout ){                                             // Describe the results
        Serial.println(F("Failed, response timed out."));
    }else{
        unsigned long got_time;                                 // Grab the response, compare, and send to debugging spew
        radio.read( &got_time, sizeof(unsigned long) );
        unsigned long end_time = micros();

        // Spew it
        Serial.print(F("Sent "));
        Serial.print(start_time);
        Serial.print(F(", Got response "));
        Serial.print(got_time);
        Serial.print(F(", Round-trip delay "));
        Serial.print(end_time-start_time);
        Serial.println(F(" microseconds"));
    }

    // Try again 1s later
    delay(1000);
  }



/****************** Pong Back Role ***************************/

  if ( role == 0 )
  {
    unsigned long got_time;

    if( radio.available()){
                                                                    // Variable for the received timestamp
      while (radio.available()) {                                   // While there is data ready
        radio.read( &got_time, sizeof(unsigned long) );             // Get the payload
      }

      radio.stopListening();                                        // First, stop listening so we can talk
      radio.write( &got_time, sizeof(unsigned long) );              // Send the final one back.
      radio.startListening();                                       // Now, resume listening so we catch the next packets.
      Serial.print(F("Sent response "));
      Serial.println(got_time);
   }
 }




/****************** Change Roles via Serial Commands ***************************/

  if ( Serial.available() )
  {
    char c = toupper(Serial.read());
    if ( c == 'T' && role == 0 ){
      Serial.println(F("*** CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK"));
      role = 1;                  // Become the primary transmitter (ping out)

   }else
    if ( c == 'R' && role == 1 ){
      Serial.println(F("*** CHANGING TO RECEIVE ROLE -- PRESS 'T' TO SWITCH BACK"));
       role = 0;                // Become the primary receiver (pong back)
       radio.startListening();

    }
  }


} // Loop


// //////////////  DEFINITIONS / CONFIGS  //////////////////////////////////////

// #define RECEIVER_PIN 12
// #define OS_GAIN_REFRESH_DELAY 0
// /* Gain refresh time of SRX882 module is around 100 milliseconds.
//    If only one pair of SRX and STX are used to connect 2 devices in SIMPLEX
//    mode, there is no need to refresh receiver's gain, being communication
//    mono-directional. */

// #define MOTO_DRIVER_I2C_ADDR 0x30
// #define MOTO_DRIVER_PWM_FREQ 1000



// //////////////  INCLUDES  //////////////////////////////////////
// #include <PJON.h>
// #include "WEMOS_Motor.h"



// // <Strategy name> bus(selected device id)
// PJON<OverSampling> bus(44);

// //Motor shiled I2C Address: 0x30
// //PWM frequency: 1000Hz(1kHz)
// Motor M1(MOTO_DRIVER_I2C_ADDR,_MOTOR_A, MOTO_DRIVER_PWM_FREQ);//Motor A
// Motor M2(MOTO_DRIVER_I2C_ADDR,_MOTOR_B, MOTO_DRIVER_PWM_FREQ);//Motor B


// void setup() {
//   bus.set_communication_mode(PJON_SIMPLEX);
//   bus.strategy.set_pins(RECEIVER_PIN , PJON_NOT_ASSIGNED);
//   bus.begin();

//   bus.set_receiver(receiver_function);

//   Serial.begin(115200);
// };

// void loop() {
// 	// CAR_CONTROL();
// 	PJON_RECV();
// }


// void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info) {
//  // Do nothing to avoid affecting speed analysis
// }


// int pwm;
// void CAR_CONTROL(){
// 	for (pwm = 20; pwm <= 100; pwm++)
//   {
//     M1.setmotor( _CW, pwm);
//     M2.setmotor(_CW, 100-pwm);
//     Serial.printf("A:%d%, B:%d%, DIR:CW\r\n", pwm,100-pwm);
//   }

//   M1.setmotor(_STOP);
//   M2.setmotor( _STOP);
//   Serial.println("Motor A&B STOP");
//   delay(200);

//   for (pwm = 20; pwm <=100; pwm++)
//   {
//     M1.setmotor(_CCW, pwm);
//     M2.setmotor(_CCW, 100-pwm);
//     Serial.printf("A:%d%, B:%d%, DIR:CCW\r\n", pwm,100-pwm);

//   }

//   M1.setmotor(_STOP);
//   M2.setmotor( _STOP);
//   delay(200);
//   Serial.println("Motor A&B STOP");

//   M1.setmotor(_SHORT_BRAKE);
//   M2.setmotor( _SHORT_BRAKE);
//   Serial.println("Motor A&B SHORT BRAKE");
//   delay(1000);

//   M1.setmotor(_STANDBY);
//   M2.setmotor( _STANDBY);
//   Serial.println("Motor A&B STANDBY");
//   delay(1000);
// }



// float test;
// float mistakes;
// int busy;
// int fail;

// void PJON_RECV(){
//   Serial.println("Starting 1 second communication speed test...");
//   long time = millis();
//   unsigned int response = 0;
//   while(millis() - time < 1000) {
//     response = bus.receive();
//     if(response == PJON_ACK)
//       test++;
//     if(response == PJON_NAK)
//       mistakes++;
//     if(response == PJON_BUSY)
//       busy++;
//     if(response == PJON_FAIL)
//       fail++;
//   }

//   Serial.print("Absolute com speed: ");
//   Serial.print(test * 25);
//   Serial.println("B/s");
//   Serial.print("Practical bandwidth: ");
//   Serial.print(test * 20);
//   Serial.println("B/s");
//   Serial.print("Packets sent: ");
//   Serial.println(test);
//   Serial.print("Mistakes (error found with CRC) ");
//   Serial.println(mistakes);
//   Serial.print("Fail (no answer from receiver) ");
//   Serial.println(fail);
//   Serial.print("Busy (Channel is busy or affected by interference) ");
//   Serial.println(busy);
//   Serial.print("Accuracy: ");
//   Serial.print(100 - (100 / (test / mistakes)));
//   Serial.println(" %");
//   Serial.println(" --------------------- ");

//   test = 0;
//   mistakes = 0;
//   busy = 0;
//   fail = 0;
// };