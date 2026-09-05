#include <M5Unified.h>

static bool beeping = false;

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);

  M5.Speaker.setVolume(255);

  Serial.println("BtnA: toggle beep");
}

void loop() {
  M5.update();

  // wasClicked() は長押し閾値(既定 500ms)未満で離したときに true になる。
  if (M5.BtnA.wasClicked()) {
    beeping = !beeping;
    if (beeping) {
      // duration 省略時は停止するまで鳴り続ける。
      M5.Speaker.tone(1000);
    } else {
      M5.Speaker.stop();
    }
    Serial.printf("beep: %s\n", beeping ? "on" : "off");
  }

  delay(10);
}
