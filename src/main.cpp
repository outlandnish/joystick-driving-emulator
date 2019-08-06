#include <main.h>

void setup()
{
  pinMode(20, INPUT_PULLUP);
  led_pwm_init(LED_PRIMARY_IDX, LED_PRIMARY_PIN);
  led_pwm_init(LED_RGB_RED_IDX, LED_RGB_RED_PIN);
  led_pwm_init(LED_RGB_GREEN_IDX, LED_RGB_GREEN_PIN);
  led_pwm_init(LED_RGB_BLUE_IDX, LED_RGB_BLUE_PIN);

  joystick = new Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_GAMEPAD, 8, 0, true, true, true, true, true, true, false, false, false, false, false);
  joystick->setXAxisRange(-3600, 3600);
  joystick->setYAxisRange(-255, 255);
  joystick->begin(false);

  Serial.begin(115200);

  Serial1.setPins(_PINNUM(0, 8), _PINNUM(0, 6));
  Serial1.begin(115200);
  delay(2000);
  Serial1.flush();

  Serial.println("Started as a joystick...waiting for connection");

  Serial.println("Mounted joystick");
  attachInterrupt(20, switchSide, FALLING);

  // monitor serial from macchina
  xTaskCreate((TaskFunction_t)processMachinaData, "macchina", 500, nullptr, 1, &macchinaTaskHandle);
  xTaskCreate((TaskFunction_t)updateJoystick, "joystick", 500, nullptr, 2, &joystickTaskHandle);
}

void loop()
{
  delay(10);
}

void switchSide() {
  switch(side){
    case JoystickSide::Disabled:
      side = JoystickSide::Left;
      led_pwm_duty_cycle(LED_RGB_RED_IDX, 255);
      led_pwm_duty_cycle(LED_RGB_GREEN_IDX, 0);

      // reset pose
      pose.accelerator = 0;
      pose.brakes = 0;
      pose.steering = 0;
      pose.clutch = 0;
      pose.downshift = 0;
      pose.ebrake = 0;
      pose.menu = 0;
      pose.rewind = 0;
      pose.upshift = 0;
      pose.view = 0;
      break;
    case JoystickSide::Left:
      side = JoystickSide::Right;
      led_pwm_duty_cycle(LED_RGB_RED_IDX, 0);
      led_pwm_duty_cycle(LED_RGB_GREEN_IDX, 255);

      break;
    case JoystickSide::Right:
      side = JoystickSide::Disabled;
      led_pwm_duty_cycle(LED_RGB_RED_IDX, 0);
      led_pwm_duty_cycle(LED_RGB_GREEN_IDX, 0);
      
      break;
  }
  sideChanged = true;
}

void updateJoystick() {
  for (;;) {
    if (newData) {
      switch (side) {
        case JoystickSide::Left:
          // steering
          joystick->setXAxis(pose.steering);

          // brakes
          joystick->setYAxis(-pose.brakes);
          
          // clutch
          pose.clutch ? joystick->pressButton(XboxButtons::LEFT_BUMPER) : joystick->releaseButton(XboxButtons::LEFT_BUMPER);

          // e-brake
          pose.ebrake ? joystick->pressButton(XboxButtons::A) : joystick->releaseButton(XboxButtons::A);

          // upshift
          pose.upshift ? joystick->pressButton(XboxButtons::B) : joystick->releaseButton(XboxButtons::B);
          break;
        case JoystickSide::Right:
          // accelerator
          joystick->setYAxis(-pose.accelerator);

          // downshift
          pose.downshift ? joystick->pressButton(XboxButtons::X) : joystick->releaseButton(XboxButtons::X);

          // rewind
          pose.rewind ? joystick->pressButton(XboxButtons::Y) : joystick->releaseButton(XboxButtons::Y);
          break;
      }
      if (side != JoystickSide::Disabled)
        joystick->sendState();
      newData = false;
    }
    delay(10);
  }

  vTaskDelete(nullptr);
}

void processMachinaData() {
  Serial.println("Started macchina task");

  for (;;) {
    if (Serial1.available()) {
      std::string data = Serial1.readStringUntil('\n').c_str();
      auto parts = split(data, ',');
      if (parts.size() >= 10 && side != JoystickSide::Disabled) {
        led_pwm_duty_cycle(LED_RGB_BLUE_IDX, 255);
        try {
          pose.steering = atoi(parts[0].c_str());
          pose.accelerator = atoi(parts[1].c_str());
          pose.brakes = atoi(parts[2].c_str());
          pose.clutch = atoi(parts[3].c_str()) > 0;
          pose.upshift = atoi(parts[4].c_str()) > 0;
          pose.downshift = atoi(parts[5].c_str()) > 0;
          pose.ebrake = atoi(parts[6].c_str()) > 0;
          pose.rewind = atoi(parts[7].c_str()) > 0;
          pose.view = atoi(parts[8].c_str()) > 0;
          pose.menu = atoi(parts[9].c_str()) > 0;

          newData = true;
        }
        catch (err_t err) {
          Serial.println("error");
          newData = false;
        }
      }
    }
    else {
      led_pwm_duty_cycle(LED_RGB_BLUE_IDX, 0);
    }
    delay(10);
  }

  vTaskDelete(NULL);
}