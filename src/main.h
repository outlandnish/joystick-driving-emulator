#include <vector>
#include <string>
#include <sstream>

#include <Arduino.h>
#include <Joystick.h>
#include <SoftwareSerial.h>

enum JoystickSide {
  Disabled = 0,
  Left,
  Right
};

struct Pose {
  int16_t steering;
  uint8_t brakes;
  uint8_t accelerator;
  bool clutch;
  bool upshift;
  bool downshift;
  bool ebrake;
  bool rewind;
  bool view;
  bool menu;
};

static Joystick* joystick;
static Pose pose;

static bool newData = false;
bool sideChanged = false;

JoystickSide side = JoystickSide::Disabled;
static SoftwareSerial macchina(6, 7);

TaskHandle_t macchinaTaskHandle;
TaskHandle_t joystickTaskHandle;

void switchSide();
void lightJoystickSide();
void processMachinaData();
void updateJoystick();

char buffer[256];
int bIndex = 0;

std::vector<std::string> split(const std::string &s, char delim) {
  std::stringstream ss(s);
  std::string item;
  std::vector<std::string> elems;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
    // elems.push_back(std::move(item)); // if C++11 (based on comment from @mchiasson)
  }
  return elems;
}

enum XboxButtons : uint8_t {
  LEFT_X1 = 0,
  LEFT_X2,
  LEFT_THUMBSTICK,
  LEFT_BUMPER,
  A,
  B,
  LEFT_VIEW,
  LEFT_MENU,

  RIGHT_SELECT = 0,
  RIGHT_VIEW,
  RIGHT_THUMBSTICK,
  RIGHT_BUMPER,
  X,
  Y,
  RIGHT_X1,
  RIGHT_X2
};