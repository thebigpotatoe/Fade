#include <Fade.h>

Fade fader(16, FADE_PIN_INVERTED);

void setup() {
	Serial.begin(115200);
	Serial.println();

	Serial.println(sizeof(fader));
    fader.set_global_duration(1000);
    fader.fade_to(100);
}

void loop() {
	fader.loop();
    if(!fader.is_fading()) {
        fader.toggle();
    }
}