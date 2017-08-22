
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
RF24 radio(9,10);
// RF24 radio(7,8);
// RF24 radio(10,9);
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


// #include <SPI.h>
// #include "RF24.h"

// #include <Wire.h>

// #define TRANSMITER_PIN E5


// #define PJON_INCLUDE_OS
// #define OS_GAIN_REFRESH_DELAY 0
// /* Gain refresh time of SRX882 module is around 100 milliseconds.
//    If only one pair of SRX and STX are used to connect 2 devices in SIMPLEX
//    mode, there is no need to refresh receiver's gain, being communication
//    mono-directional. */

// #include <PJON.h>

// float test;
// float mistakes;
// int busy;
// int fail;

// // <Strategy name> bus(selected device id)
// PJON<OverSampling> bus(45);

// int packet;
// char content[] = "01234567890123456789";

// void setup() {
//   bus.set_communication_mode(PJON_SIMPLEX);
//   bus.strategy.set_pins(PJON_NOT_ASSIGNED, TRANSMITER_PIN);
// //   bus.strategy.set_pins(PJON_NOT_ASSIGNED, TX1);

//   bus.begin();
//   packet = bus.send(44, content, 20);

//   Serial.begin(115200);
//   Serial.println("PJON - Network analysis");
//   Serial.println("Starting a 1 second communication test..");
//   Serial.println();
// }

// void loop() {
//   bus.update();

//   if(!bus.packets[packet].state)
//     packet = bus.send(44, content, 20);
// };