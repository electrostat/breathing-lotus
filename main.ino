#include <InternetButton.h>

// this lamp will pulse at a rate of 5 breaths/min (1 cycle / 12 s)

#include "math.h"

int breathperiod = 12000; // The # of milliseconds that it takes to cycle through one "breath"
const float pi = 3.1415927; // this is pi.
float brightnessfactor; // how bright the LEDs will be, as a fraction of full brightness.

int state;  // The current state of the device.
            // 0: lights off
            // 1: breathing pattern on

int mindfulTime;
int mindfulInterval=600000; // 5 minutes of mindful breathing

InternetButton b = InternetButton(); // We will refer to our Internet Button as b

void setup() {
    //Tell b to get everything ready to go
    b.begin();
    Serial.begin(9600); // turn on Serial so that we can see some data
    
    Particle.function("state", manualState);
    
    Time.zone(timeZone);

    RGB.control(true);
    RGB.color(0,0,0);

    state = 0;  // start at 0 state-- the lights are off
}

void loop() {
    int t=millis();
    
    if (b.buttonOn(3)) {
        if (state==0) {
            state = 1;
            mindfulTime=millis();
        } else if (state==1) {
            state = 0;
        }
    }
    
    if (state==1) {
        if (t<mindfulTime+mindfulInterval) {
            breathingPattern();
        } else {
            finished();
            Particle.publish("end",NULL,60,PRIVATE);
            state=4;
        }
    } else {
        b.allLedsOff();
    }
}

void breathingPattern() {
    int breatht = millis() % breathperiod;
    
    float brightnessfactor = .5 + .5 * sin(2*pi*breatht/breathperiod);

    int brightnessvalueint = brightnessfactor * 100;
    
    b.allLedsOn(brightnessvalueint, brightnessvalueint/2, 0);
}

int checkTime() {
    // turn light on
    RGB.control(false);
    // get the time
    if (Time.hour()==timeToGoOff) {
        return 1;
    }
    else {return 0;}
    RGB.control(true);
    RGB.color(0,0,0);
    lastChecked=millis();
}

void finished() {
    b.playSong("D4,4,G4,2,");
    // fade to black
    int i=100;
    while (i>0) {
        i--;
        b.allLedsOn(i,i,i);
    }
    b.allLedsOff();
}