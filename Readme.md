# Xbox Adaptive Controller Driving Emulator

Emulates an HID device that can be plugged into either the left or right Xbox Adaptive Controller ports. When paired with https://github.com/nsamala/m2-can-forwarder, this can be used to control an Xbox Controller with your car.

Built for the Particle Xenon or any other nRF52840 that can use the Adafruit's nrF52 library.

## Usage

Mode button: toggles between Disabled, Left Half, and Right Half.

* Left (red light): controls the brakes, steering, e-brake, clutch, and upshift
* Right (green light): controls the accelerator, downshift, and rewind buttons (Forza)

Plug it into either the left or right USB port on the Xbox Adaptive Controoller and set the mode accordingly.

## Credits

Arduino Joystick Library: https://github.com/MHeironimus/ArduinoJoystickLibrary
