# Fade

Fade is a boilerplate library designed to make fading LED's simpler with less up front code. It is aimed at doing one thing and one thing only; control a PWM output through time.

The key features of Fade are; asyncronous handling of fade effects, fault tolerant states, and transparent operation of transitions.

## Quick Start

Fade could not be easier, just create an instance, use the API, and enure that the main loop() method is being called. Here is an example of a breathing LED on pin 16;

``` c++

#include <Fade.h>

Fade fader(16);

void setup() {
  fader.set_global_duration(1000);
  fader.fade_to(100);
}

void loop() {
  if (!fader.is_fading()) fader.toggle();
  fader.loop();
}

```

## Install

You can insatall this library through the Arduino Library interface of via PlatformIO:

## API

The API of Fade is intentionally simple. Below is the complete list of function calls available:

| Function                                      | Arguments | Return |
| --------------------------------------------- |-------------------------------------------------------------------------------------------------------------| ---------------------------- |
| `Fade(uint8_t, bool = FADE_PIN_NOT_INVERTED)` | 1: The pin number to controll <br />2: A bool if the pin should be inverted or not                          | An object of the Fade class |
| `void loop()`                                 | none                                                                                                        | none                          |
| `void set_global_duration(unsigned long)`     | 1: The global duration in milliseconds any fade will take                                                   | none                          |
| `void set_gamma_callback(gamma_callback)`     | 1: A callback used to manipulate the gamma value                                                            | none                          |
| `void on(unsigned long = -1)`                 | (optional) 1: The duration in milliseconds this fade will take                                              | none                          |
| `void off(unsigned long = -1)`                | (optional) 1: The duration in milliseconds this fade will take                                              | none                          |
| `void toggle(unsigned long = -1)`             | (optional) 1: The duration in milliseconds this fade will take                                              | none                          |
| `void fade_to(uint8_t, unsigned long = -1)`   | 1: The brightness to fade to (0 to 255)<br />(optional) 2: The duration in milliseconds this fade will take | none                          |
