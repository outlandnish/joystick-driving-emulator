#include <main.h>

void setup()
{
  pinMode(20, INPUT_PULLUP);
  led_pwm_init(LED_PRIMARY_IDX, LED_PRIMARY_PIN);
  led_pwm_init(LED_RGB_RED_IDX, LED_RGB_RED_PIN);
  led_pwm_init(LED_RGB_GREEN_IDX, LED_RGB_GREEN_PIN);
  led_pwm_init(LED_RGB_BLUE_IDX, LED_RGB_BLUE_PIN);

  Serial.begin(115200);
  macchina.begin(9600);

  Serial.println("Started as a joystick...waiting for connection");

  joystick = new Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_GAMEPAD, 8, 0, true, true, false, false, false, false, false, false, false, false, false);
  joystick->setXAxisRange(-3600, 3600);
  joystick->setYAxisRange(0, 255);
  joystick->begin(false);

  Serial.println("Mounted joystick");
  attachInterrupt(20, switchSide, FALLING);

  // monitor serial from macchina
  xTaskCreate((TaskFunction_t)processMachinaData, "macchina", 500, nullptr, 1, &macchinaTaskHandle);
  xTaskCreate((TaskFunction_t)updateJoystick, "joystick", 500, nullptr, 1, &joystickTaskHandle);
}

void loop()
{
  // light up side indicator
  lightJoystickSide();

  delay(10);
}

void switchSide() {
  switch(side){
    case JoystickSide::Disabled:
      side = JoystickSide::Left;
      break;
    case JoystickSide::Left:
      side = JoystickSide::Right;
      break;
    case JoystickSide::Right:
      side = JoystickSide::Disabled;
      break;
  }
  sideChanged = true;
}

void lightJoystickSide() {
  if (sideChanged) {
    switch (side) {
      case JoystickSide::Disabled:
        Serial.println("Joystick side: diabled");
        led_pwm_duty_cycle(LED_RGB_RED_IDX, 0);
        led_pwm_duty_cycle(LED_RGB_GREEN_IDX, 0);
        break;
      case JoystickSide::Left:
        Serial.println("Joystick side: left");
        led_pwm_duty_cycle(LED_RGB_RED_IDX, 255);
        led_pwm_duty_cycle(LED_RGB_GREEN_IDX, 0);
        break;
      case JoystickSide::Right:    
        Serial.println("Joystick side: right");
        led_pwm_duty_cycle(LED_RGB_RED_IDX, 0);
        led_pwm_duty_cycle(LED_RGB_GREEN_IDX, 255);
        break;
    }
    sideChanged = false;
  }
}

void updateJoystick() {
  for (;;) {
    if (newData) {
      switch (side) {
        case JoystickSide::Left:
          // steering
          joystick->setXAxis(pose.steering);

          // brakes
          joystick->setYAxis(pose.brakes);
          
          // clutch
          pose.clutch ? joystick->pressButton(XboxButtons::LEFT_BUMPER) : joystick->releaseButton(XboxButtons::LEFT_BUMPER);

          // e-brake
          pose.ebrake ? joystick->pressButton(XboxButtons::A) : joystick->releaseButton(XboxButtons::A);

          // upshift
          pose.upshift ? joystick->pressButton(XboxButtons::B) : joystick->releaseButton(XboxButtons::B);
          break;
        case JoystickSide::Right:
          // accelerator
          joystick->setYAxis(pose.accelerator);

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
    if (macchina.available()) {
      std::string data = macchina.readStringUntil('\n').c_str();
      auto parts = split(data, ',');
      led_pwm_duty_cycle(LED_RGB_BLUE_IDX, 255);
      if (parts.size() == 10) {
        try {
          pose.steering = atoi(parts[0].c_str());
          pose.accelerator = atoi(parts[1].c_str());
          pose.brakes = atoi(parts[2].c_str());
          pose.clutch = atoi(parts[3].c_str());
          pose.upshift = atoi(parts[4].c_str());
          pose.downshift = atoi(parts[5].c_str());
          pose.ebrake = atoi(parts[6].c_str());
          pose.rewind = atoi(parts[7].c_str());
          pose.view = atoi(parts[8].c_str());
          pose.menu = atoi(parts[9].c_str());

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
  }

  vTaskDelete(NULL);
}