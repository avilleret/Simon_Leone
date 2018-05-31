#include <Arduino.h>

#include "constants.h"

int led_pins[] = {
  LED_RED_PIN,
  LED_GREEN_PIN,
  LED_YELLOW_PIN,
  LED_BLUE_PIN
};

int btn_pins[] = {
  BTN_RED_PIN,
  BTN_GREEN_PIN,
  BTN_YELLOW_PIN,
  BTN_BLUE_PIN
};

int out_pins[] = {
  PAD_SEL_PIN,
  SWITCH_SEL_PIN,
  START_BTN_PIN
};

int tones[] = {
  TONE_RED,
  TONE_GREEN,
  TONE_YELLOW,
  TONE_BLUE
};

bool isPlaying{false};

bool previous_state[3][4]{};
bool previous_state_on;
// ms elapsed since last trig
unsigned long on_duration[3][5]{};
unsigned long on_duration_on;

void win_tone()
{
  tone(SPEAKER_PIN,TONE_WIN);
  delay(100);
  noTone(SPEAKER_PIN);
  delay(100);

  tone(SPEAKER_PIN,TONE_WIN);
  delay(100);
  noTone(SPEAKER_PIN);
  delay(100);

  tone(SPEAKER_PIN,TONE_WIN);
  delay(100);
  noTone(SPEAKER_PIN);
  delay(100);

  tone(SPEAKER_PIN,TONE_WIN);
  delay(550);
  noTone(SPEAKER_PIN);
}

void lose_tone()
{
  noTone(SPEAKER_PIN);
  double period = 12000;
  for(int i=0;i<16;i++)
  {
    period -= i*1000/16.;
    digitalWrite(SPEAKER_PIN,HIGH);
    delayMicroseconds(period/2.);
    digitalWrite(SPEAKER_PIN,LOW);
    delayMicroseconds(period/2.);
  }
  delay(100);

  period = 12000;
  for(int i=0;i<16;i++)
  {
    period += i*1000/16.;
    digitalWrite(SPEAKER_PIN,HIGH);
    delayMicroseconds(period/2.);
    digitalWrite(SPEAKER_PIN,LOW);
    delayMicroseconds(period/2.);
  }
  delay(100);

  period = 14000;
  for(int i=0;i<16;i++)
  {
    digitalWrite(SPEAKER_PIN,HIGH);
    delayMicroseconds(period/2.);
    digitalWrite(SPEAKER_PIN,LOW);
    delayMicroseconds(period/2.);
  }
  delay(100);

  period = 13000;
  for(int i=0;i<30;i++)
  {
    period -= i*2000/30.;
    digitalWrite(SPEAKER_PIN,HIGH);
    delayMicroseconds(period/2.);
    digitalWrite(SPEAKER_PIN,LOW);
    delayMicroseconds(period/2.);
  }
  for(int i=0;i<15;i++)
  {
    period += i*1000/15.;
    digitalWrite(SPEAKER_PIN,HIGH);
    delayMicroseconds(period/2.);
    digitalWrite(SPEAKER_PIN,LOW);
    delayMicroseconds(period/2.);
  }

}

void test()
{

  for (auto i : led_pins)
  {
    digitalWrite(i,HIGH);
    delay(100);
    digitalWrite(i,LOW);
    delay(100);
  }

  for (auto t : tones)
  {
    tone(SPEAKER_PIN,t);
    delay(500);
    noTone(SPEAKER_PIN);
    delay(100);
  }

  win_tone();

  delay(1000);

  lose_tone();
}

/***********/
/*  SETUP  */
/***********/
void setup() {

  for(int y=0; y<3 ; y++)
  {
    pinMode(out_pins[y], OUTPUT);
    digitalWrite(out_pins[y], HIGH);
    for (int i=0; i<4; i++)
    {
      pinMode(btn_pins[i],INPUT);
      digitalWrite(btn_pins[i],HIGH); // use internal pull-up resistor
      previous_state[y][i] = digitalRead(btn_pins[i]);
      on_duration[y][i] = millis();
    }
    digitalWrite(out_pins[y], LOW);
  }

  for (auto i : led_pins)
  {
    pinMode(i,OUTPUT);
    digitalWrite(i,LOW);
  }

  pinMode(ON_SWITCH_PIN, INPUT);
  digitalWrite(ON_SWITCH_PIN, HIGH);

  test();

  Serial.begin(115200);

  while(!Serial) // Wait for serial connection (kind of 32u4 bug)
  {;}
}

/**********/
/*  LOOP  */
/**********/
bool lock = false;

void loop() {
  while(Serial.available())
  {
    char a = Serial.read();

    switch(a)
    {
      case RED_ON:
        digitalWrite(LED_RED_PIN, HIGH);
        break;
      case RED_OFF:
        digitalWrite(LED_RED_PIN, LOW);
        break;
      case RED_TONE:
        tone(SPEAKER_PIN, TONE_RED);
        break;
      case GREEN_ON:
        digitalWrite(LED_GREEN_PIN, HIGH);
        break;
      case GREEN_OFF:
        digitalWrite(LED_GREEN_PIN, LOW);
        break;
      case GREEN_TONE:
        tone(SPEAKER_PIN, TONE_GREEN);
        break;
      case YELLOW_ON:
        digitalWrite(LED_YELLOW_PIN, HIGH);
        break;
      case YELLOW_OFF:
        digitalWrite(LED_YELLOW_PIN, LOW);
        break;
      case YELLOW_TONE:
        tone(SPEAKER_PIN, TONE_YELLOW);
        break;
      case BLUE_ON:
        digitalWrite(LED_BLUE_PIN, HIGH);
        break;
      case BLUE_OFF:
        digitalWrite(LED_BLUE_PIN, LOW);
        break;
      case BLUE_TONE:
        tone(SPEAKER_PIN, TONE_BLUE);
        break;
      case LOSE_TONE:
        lose_tone();
        break;
      case WIN_TONE:
        win_tone();
        break;
      case TONE_OFF:
        noTone(SPEAKER_PIN);
        break;
      default:
      ;
    }

    switch(a)
    {
      case RED_ON:
      case RED_TONE:
      case GREEN_ON:
      case GREEN_TONE:
      case YELLOW_ON:
      case YELLOW_TONE:
      case BLUE_ON:
      case BLUE_TONE:
      case LOSE_TONE:
      case WIN_TONE:
        lock=true;
        break;
      default:
        lock=false;
    }

    switch(a)
    {
      case WIN_TONE:
      case LOSE_TONE:
        for(int i=0; i<4; i++)
        {
          digitalWrite(led_pins[i],LOW);
        }
        break;
      default:
        ;
    }
  }

  for (int y=0; y<3; y++)
  {
    digitalWrite(out_pins[y], LOW);
    char offset = (y+1) << 4;
    for (int i=0; i<4; i++)
    {
      bool a = !digitalRead(btn_pins[i]); // invert input since it's pulled-up,
                                // it will be LOW when button is pressed
      if(a != previous_state[y][i]
         && (millis() - on_duration[y][i]) > DEBOUNCE_THRESH)
      {
        previous_state[y][i] = a;
        on_duration[y][i] = millis(); // reset time counter
        if(a)
          Serial.print(static_cast<char>(i+offset));
      }
    }
    digitalWrite(out_pins[y], HIGH);
  }

  if(!lock)
  {
    bool shutup = true;
    for(int i=0; i<4; i++)
    {
      if(previous_state[0][i])
      {
        tone(SPEAKER_PIN, tones[i]);
        shutup=false;
        digitalWrite(led_pins[i],HIGH);
      } else {
        digitalWrite(led_pins[i],LOW);
      }
    }
    if(shutup)
      noTone(SPEAKER_PIN);
  }
  bool on = !digitalRead(ON_SWITCH_PIN); // invert input since it's pulled-up,
                            // it will be LOW when button is pressed
  if(on != previous_state_on
     && (millis() - on_duration_on) > DEBOUNCE_THRESH)
  {
    previous_state_on = on;
    on_duration_on = millis(); // reset time counter
    Serial.print(static_cast<char>(on));
  }
}
