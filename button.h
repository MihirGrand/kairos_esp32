#define DEBOUNCE_DELAY 50
#define LONG_CLICK_TIME 1500
#define BUZZER_PIN 18

class ButtonHandler {
  private:
    uint8_t buttonPin;
    bool buttonState;
    bool lastButtonState;
    unsigned long lastDebounceTime;
    unsigned long buttonPressTime;
    bool longClickReported;

  public:
    ButtonHandler(uint8_t pin) {
      buttonPin = pin;
      pinMode(buttonPin, INPUT_PULLUP);
      pinMode(BUZZER_PIN, OUTPUT);
      buttonState = HIGH;
      lastButtonState = HIGH;
      lastDebounceTime = 0;
      buttonPressTime = 0;
      longClickReported = false;
    }
    int checkButton() {
      int event = 0;
      bool reading = digitalRead(buttonPin);
      if (reading != lastButtonState) {
        lastDebounceTime = millis();
      }

      if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
        if (reading != buttonState) {
          buttonState = reading;
          if (buttonState == LOW) {
            buttonPressTime = millis();
            digitalWrite(BUZZER_PIN, HIGH);
            longClickReported = false;
          } 
          else {
            digitalWrite(BUZZER_PIN, LOW);

            if (!longClickReported) {
              event = 1;
            }
          }
        }
      }

      if (buttonState == LOW && (millis() - buttonPressTime) >= LONG_CLICK_TIME) {
        if (!longClickReported) {
          longClickReported = true;
          event = 2;
          digitalWrite(BUZZER_PIN, LOW);
        }
      }

      lastButtonState = reading;
      return event;
    }
};