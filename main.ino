// This #include statement was automatically added by the Particle IDE.
#include <InternetButton.h>

// this lamp will pulse at a rate of 5 breaths/min (1 cycle / 12 s)

#include "math.h"

int breathperiod = 12000; // The # of milliseconds that it takes to cycle through one "breath"
const float pi = 3.1415927; // this is pi.
float brightnessfactor; // how bright the LEDs will be, as a fraction of full brightness.

int state;  // The current state of the device.
            // 0: lights off
            // 1: reminder lights and sound on
            // 2: waiting for you to pick it up
            // 3: entering breathing pattern
            // 4: exiting breathing pattern

int moving=0;   // assume it is not moving to start
int movingThreshold=100;

int timeToGoOff=14;  // in hours
int timeZone=-4; // est

int lastChecked=0;
int checkInterval=300000;

int mindfulTime;
int mindfulInterval=300000; // 5 minutes of mindful breathing

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
    
    if (state==0) {
        // wait checkInterval ms, then check the loop
        if (t>lastChecked+checkInterval) {
            if (checkTime()==1) {
                state=1;
            }
            else {
                state=0;
            }
        }
    }
    else if (state==1) {
        // play reminder lights and sound, progress to 2
        reminder();
        state=2;
    }
    else if (state==2) {
        // wait for you to pick it up
        if (isItMoving()==1) {
            mindfulTime=millis();
            Particle.publish("start",NULL,60,PRIVATE);
            state=3;
        }
    }
    else if (state==3) {
        if (t<mindfulTime+mindfulInterval) {
            breathingPattern();
        }
        else {
            finished();
            Particle.publish("end",NULL,60,PRIVATE);
            state=4;
        }
    }
    else if (state==4) {
        // wait an hour
        if (t>mindfulTime+3600000) {
            state=0;
        }
    }
    
    if (checkForShake()==1) {
        advanceState();
    }
    
}
    
int checkForShake() {
    if (abs(b.readX())>100) {
        delay(500);
        return 1;
    }
    else {
        return 0;
    }
}

void advanceState() {
    if (state==4) {
        mindfulTime=millis()-3600000;
        b.allLedsOff();
        state=0;
    }
    else {
        state++;
    }
}


void breathingPattern() {
    // this modulo will tell you where to be in the wave function
    int breatht = millis() % breathperiod;
    
    // Remember trigonometry?
    // waveperiod is 2pi, so you want to do wavet*2*pi/waveperiod
    
    // set brightness to this
    float brightnessfactor = .5 + .5 * sin(2*pi*breatht/breathperiod);

    // now round it to an integer between 1 and 255 and set it to that
    int brightnessvalueint = brightnessfactor * 100;
    
    b.allLedsOn(0,brightnessvalueint,brightnessvalueint);
}


int isItMoving() {
    // if ((abs(b.readX())>movingThreshold) || (abs(b.readY())>movingThreshold) || (abs(b.readZ())>movingThreshold)) {
    if (abs(b.readZ())>movingThreshold) {
        return 1;
    }
    else {
        return 0;
    }
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

void reminder() {
    b.allLedsOn(255,255,255);
    b.playSong("B5,8,G4,4,");
    delay(500);
    b.allLedsOff();
    delay(500);
    b.allLedsOn(255,255,255);
    delay(500);
    b.allLedsOff();
    delay(500);
    b.allLedsOn(100,100,100);
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

int manualState(String command) {
    // sets state manually through a Particle.function
    char inputStr[64];
    command.toCharArray(inputStr,64);
    state=atoi(inputStr);
    return state;
}