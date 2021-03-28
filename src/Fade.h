#pragma once

// Include Guard
#ifndef FADE_h
#define FADE_h

// Static Defines
#define FADE_PIN_INVERTED     true
#define FADE_PIN_NOT_INVERTED false

// User Definable Brightness Range
#ifndef FADE_BRIGHTNESS_RANGE
#define FADE_BRIGHTNESS_RANGE 100
#endif

// User Definable PWM Bit Range
#ifndef FADE_PWM_BIT_RANGE
#define FADE_PWM_BIT_RANGE 1024
#endif

// User Definable Time Division
#ifndef FADE_MIN_TIME_DIVISION
#define FADE_MIN_TIME_DIVISION 10
#endif

// Required Includes
#include <Arduino.h>

class Fade {
 public:  // Typedefs
	typedef std::function<int(int)> gamma_callback;

 public:  // Constructor
	Fade(uint8_t _pin, bool _inverted = FADE_PIN_NOT_INVERTED) {
		if (_check_pin_num(_pin)) {
			pin      = _pin;
			inverted = _inverted;
			pinMode(_pin, OUTPUT);
#ifndef FADE_NO_GAMMA_CORRECTION
			g_callback = [](int _value) {
				return (int)(pow(((float)_value / (float)FADE_PWM_BIT_RANGE), 2.2) * (float)FADE_PWM_BIT_RANGE);  // 250us
			};
#endif
		}
	}
	~Fade() {}

 public:  // Loop Method
	void loop() {
		_handle_fade();
	}

 public:  // Configuration Methods
	void set_global_duration(unsigned long _global_duration) {
		global_duration = _global_duration;
	}
	void set_gamma_callback(gamma_callback _callback) {
		if (_callback) g_callback = _callback;
	}

 public:  // Checking Methods
	bool is_fading() {
		return !_fade_done;
	}

 public:  // Fade Methods
	void on(unsigned long _duration = -1) {
		state             = true;
		target_brightness = previous_brightness;
		_start_fade(_duration);
	}
	void off(unsigned long _duration = -1) {
		state             = false;
		target_brightness = 0;
		_start_fade(_duration);
	}
	void toggle(unsigned long _duration = -1) {
		state             = !state;
		target_brightness = (state) ? previous_brightness : 0;
		_start_fade(_duration);
	}
	void fade_to(uint8_t _brightness, unsigned long _duration = -1) {
		state             = bool(_brightness);
		target_brightness = _brightness;
		_start_fade(_duration);
	}

 protected:  // Handling Methods
	bool _check_pin_num(uint8_t _pin) {
		return (0 < _pin && _pin <= 16);
	}
	void _start_fade(unsigned long _duration = -1) {
		// Check if the pin is valid before doing anything
		if (_check_pin_num(pin)) {
			// Set the previous brightness from the last target brightness if it was not 0
			previous_brightness = (target_brightness != 0) ? target_brightness : ((previous_brightness != 0) ? previous_brightness : 100);

			// Get the duration of the fade
			_duration = ((_duration == (unsigned long)(-1)) ? global_duration : _duration);

			// If the duration is above 0 fade the LED else skip the calculations
			if (_duration > 0 && target_brightness != current_brightness) {
				// If the brightness steps are greater than the duration steps then increase the brightness per step to match the number of duration steps
				// Else if the duration steps are greater than the brightness steps decrease the duration steps by increasing the time between steps

				unsigned long _time_steps       = _duration / FADE_MIN_TIME_DIVISION;
				int           _brightness_steps = (int)target_brightness - (int)current_brightness;

				if (abs(_brightness_steps) > _time_steps) {
					_delta_brightness = (float)_brightness_steps / (float)_time_steps;  // Brightness steps will always be above 1
					_delta_time       = FADE_MIN_TIME_DIVISION;                         // Minimum time period will be kept
					_num_steps        = _time_steps;                                    // Number of steps will be the number of time steps
				} else {
					_delta_brightness = (_brightness_steps > 0) ? 1 : -1;                  // Brightness steps will always be greater than 1
					_delta_time       = (float)_duration / (float)abs(_brightness_steps);  // Time period will stretch to suit the number of brightness steps
					_num_steps        = abs(_brightness_steps);                            // Number of steps to take will be the number of brightness steps
				}

				// Serial.printf("_time_steps %lu\n", _time_steps);
				// Serial.printf("_brightness_steps %d\n", _brightness_steps);
				// Serial.printf("_delta_brightness %d\n", _delta_brightness);
				// Serial.printf("_delta_time %lu\n", _delta_time);
				// Serial.printf("_num_steps %d\n", _num_steps);

				//  Start handling the fade
				_current_steps = 0;
				_fade_done     = false;
				_handle_fade();
			} else {
				_set_pwm(target_brightness);
			}
		}
	}
	void _handle_fade() {
		if (!_fade_done && _current_steps < _num_steps && millis() - _fade_timer > _delta_time) {
			_fade_timer = millis();
			_set_pwm(current_brightness + _delta_brightness);
			_current_steps++;

			// Serial.printf("_current_steps %d\n", _current_steps);
			// Serial.printf("current_brightness %d\n", current_brightness);
		} else if (!_fade_done && _current_steps >= _num_steps) {
			_set_pwm((state) ? target_brightness : 0);  // On the last step set the brightness to its target due to integer math
			_fade_done     = true;
			_current_steps = 0;

			// Serial.printf("current_brightness %d\n", current_brightness);
			// Serial.printf("Done!\n");
		}
	}
	void _set_pwm(uint8_t _brightness) {
		if (_check_pin_num(pin) && _brightness != current_brightness) {
			// Set the new brightness
			current_brightness = constrain(_brightness, 0, FADE_BRIGHTNESS_RANGE);  // Constrain

			// Map the brightness value to a PWM value
			int pwm_value = map(current_brightness, 0, FADE_BRIGHTNESS_RANGE, 0, FADE_PWM_BIT_RANGE);  // Map
			pwm_value     = (g_callback) ? g_callback(pwm_value) : pwm_value;                          // Gamma
			pwm_value     = (inverted) ? FADE_PWM_BIT_RANGE - pwm_value : pwm_value;                        // Invert
			// Serial.printf("pwm_value %d\n", pwm_value);

			// Write to pin
			analogWrite(pin, pwm_value);  // Write
		}
	}

 protected:  // Storage
	// Pin number
	uint8_t pin      = -1;
	bool    inverted = false;

	// State
	bool state = false;

	// Brightness
	uint8_t current_brightness  = 0;  // Used to store the exact brightness at any time
	uint8_t previous_brightness = 100;
	uint8_t target_brightness   = 0;  // Used to store the current targeted brightness, can change anytime
	int     _delta_brightness   = 0;  // Used to store the change in brightness when fading

	// Duration
	unsigned long global_duration = 0;  // Used to set a global fade duration
	unsigned long _fade_timer     = 0;  // A timer to check elapsed time between fade steps
	unsigned long _delta_time     = 0;  // Used to store the amount of time between fade steps

	// Fade Calculations
	uint32_t _num_steps     = 0;
	uint32_t _current_steps = 0;
	bool     _fade_done     = true;

	// Gamma Correction
	gamma_callback g_callback;
};

#endif