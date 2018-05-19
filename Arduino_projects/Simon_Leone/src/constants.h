#define DEBOUNCE_THRESH 50

// Tone frequency in Hz
// Original freq are 110, 220, 440, 880
// but 110Hz has a little delay so change for working freqs
#define TONE_RED 220
#define TONE_GREEN 330
#define TONE_YELLOW 440
#define TONE_BLUE 880
#define TONE_WIN 698


// Keep it simple and stupid
#define RED_ON 10
#define RED_OFF 11
#define RED_TONE 12
#define GREEN_ON 20
#define GREEN_OFF 21
#define GREEN_TONE 22
#define YELLOW_ON 30
#define YELLOW_OFF 31
#define YELLOW_TONE 32
#define BLUE_ON 40
#define BLUE_OFF 41
#define BLUE_TONE 42
#define LOSE_TONE 50
#define WIN_TONE 51
#define TONE_OFF 52

#define STATUS_PLAY 50
#define STATUS_WAIT 51
#define STATUS_OFF 0

// Physical PINS
#define PAD_SEL_PIN 2
#define SWITCH_SEL_PIN 1
#define ON_SWITCH_PIN 4
#define START_BTN_PIN 6

#define BTN_RED_PIN 8
#define BTN_GREEN_PIN 7
#define BTN_YELLOW_PIN 12
#define BTN_BLUE_PIN 13

// PWM pins
#define LED_RED_PIN 5
#define LED_GREEN_PIN 9
#define LED_YELLOW_PIN 10
#define LED_BLUE_PIN 11

#define SPEAKER_PIN 3
