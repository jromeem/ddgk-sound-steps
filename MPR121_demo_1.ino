// jerome martinez

#include <Wire.h>
#include <Audio.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#include "Adafruit_MPR121.h"
#include "notes.h"
#include "FastLED.h"

#define NUM_LEDS 8
#define DATA_PIN 7
#define CLOCK_PIN 14
CRGB leds[NUM_LEDS];

// GUItool: begin automatically generated code
AudioSynthWaveformSineModulated sine_fm2;       //xy=123.28571319580078,385.1428527832031
AudioSynthWaveform       waveform1;      //xy=218,470
AudioSynthWaveformSineModulated sine_fm1;       //xy=271.8571472167969,385.4285888671875
AudioMixer4              mixer2;         //xy=444,454
AudioEffectEnvelope      envelope1;      //xy=608.4284973144531,453.4285888671875
//AudioEffectDelay         delay1;         //xy=770,614
AudioMixer4              mixer1;         //xy=771,471.5714416503906
AudioAnalyzePeak         peak1;          //xy=921,321
AudioOutputAnalog        dac1;           //xy=1001.4285659790039,486.8571300506592
AudioConnection          patchCord1(sine_fm2, sine_fm1);
AudioConnection          patchCord2(waveform1, 0, mixer2, 1);
AudioConnection          patchCord3(sine_fm1, 0, mixer2, 0);
AudioConnection          patchCord4(mixer2, envelope1);
AudioConnection          patchCord5(envelope1, 0, mixer1, 0);
//AudioConnection          patchCord6(delay1, 0, mixer1, 1);
AudioConnection          patchCord6(mixer1, dac1);
AudioConnection          patchCord7(mixer1, peak1);
//AudioConnection          patchCord9(mixer1, delay1);
// GUItool: end automatically generated code

long prev1;
int touch_readings[12] = {};
int threshold1 = 200;
Adafruit_MPR121 cap = Adafruit_MPR121();

long prevM1 = 0;
long prevM2 = 0;
long prevM3 = 0;
long prevM4 = 0;
int button1pin = 2;
int prevbutton1;
int button1;
int freq1;
int freq2;
int touch1;
int prevtouch1;
//int threshold1 = 1050;
int touch1level;
int rate1;
int seq_step1;
int seq_step2;
int seq_trig;
long env_timer;
int env_trig;
float freq_out1;
float freq_out2;
int seq_tick1;
float seq1[8] = {note_c5, note_d5, note_e5, note_f5, note_g5, note_a5, note_b5, note_c6};
float mod_amp;
float mod_freq=1;

void setup() {
  Serial.begin(9600);
  delay(500);
  pinMode(13, OUTPUT);
  if (!cap.begin(0x5A)) {
    delay(500);
    Serial.println("MPR121 not found, check wiring?");
    while (1);
  }
  Serial.println("MPR121 found!");

  FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, RGB, DATA_RATE_KHZ(100)>(leds, NUM_LEDS);
  AudioMemory(75);
  sine_fm2.amplitude(0);
  sine_fm1.amplitude(1);
  waveform1.begin(1, 440, WAVEFORM_SQUARE);
  mixer2.gain(0, .7);
  mixer2.gain(1, .3);
  envelope1.attack(1);
  envelope1.release(5);
  mixer1.gain(0, 1);
  mixer1.gain(1, 0);
}

void loop() {
  if ((millis() - prev1) > 100 ) {
    prev1 = millis();
    int contact = 0;
    for (byte i = 0; i < 12; i++) {
      touch_readings[i] = cap.filteredData(i);
      Serial.print(i); Serial.print(" "); Serial.print(touch_readings[i]); Serial.print("\t");
      if (touch_readings[11] < threshold1) {
        digitalWrite(13, 1);
        Serial.print("!!!");
        seq_trig = 1;
        seq_step1 = 0;
        prevM1 = millis();
        envelope1.noteOn();
        env_trig = 1;
        env_timer = prevM1;
      }
      else  {
        digitalWrite(13, 0);
      }
    }


    // start seq4 stuff
    if (millis() - prevM4 > rate1 / 10) {
      prevM4 = millis();
      mod_amp += .002;
      mod_freq += (mod_freq * .1);
    }
  
     sine_fm2.amplitude(mod_amp);
     sine_fm2.frequency(mod_freq);
  
    ///////////
  
    int env_len = rate1 / 3;
  
    if (millis() - env_timer > env_len && env_trig == 1) {
      envelope1.noteOff();
      env_trig = 0;
    }
  
    //////////
  
    int pot2 = analogRead(A2);
    float offset1 = pot2 / 256.00;
    freq_out1 = seq1[seq_step1] * offset1;
    sine_fm1.frequency(freq_out1);
    freq_out2 = (seq1[seq_step2] * offset1) / 4;
    waveform1.frequency(freq_out2);
  
    /////////////
  
    if (millis() - prevM3 > 30) { //update the led strip
      prevM3 = millis();
  
      if (seq_trig == 1) {
        for (int j = 0; j < NUM_LEDS; j ++ ) {
          leds[j].setHSV( 0, 0 , random(16, 24));
        }
        leds[seq_step1].setHSV( freq_out1 / 30, 255 , 64);
      }
      
      if (seq_trig == 0) {
        for (int j = 0; j < NUM_LEDS; j ++ ) {
          leds[j].setHSV( 0, 0 , random(16, 24) );
        }
      }
  
      FastLED.show();
    }
  
    ///////////////
  
    rate1 = (analogRead(A1));
  
    if (millis() - prevM1 > rate1) {
      prevM1 = millis();
  
      seq_step1++;
      if (seq_step1 > 7) {
        seq_step1 = 0;
        mod_freq = .1;
        mod_amp = 0;
        seq_trig = 0;
      }
  
      if (seq_step1 % 1 == 0) {
        seq_step2 += 2;
        if (seq_step2 > 7) {
          seq_step2 = 0;
        }
      }
  
      if (seq_trig == 1) {
        envelope1.noteOn();
        env_trig = 1;
        env_timer = prevM1;
      }
  
      Serial.print("rate1 ");
      Serial.println(rate1);
      Serial.print("freq_out1 ");
      Serial.println(freq_out1);
      Serial.println();
    }
    Serial.println();
  }
}

