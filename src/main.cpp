#include <M5Unified.h>

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);

  Serial.println("Press BtnA/B");
}

void loop() {
  M5.update();

  // wasClicked() は長押し閾値(既定 500ms)未満で離したとき、
  // wasHold() は押下が閾値を超えた時点で 1 度だけ true になる。
  if (M5.BtnA.wasClicked()) {
    Serial.println("BtnA short");
  } else if (M5.BtnA.wasHold()) {
    Serial.println("BtnA long");
  } else if (M5.BtnB.wasClicked()) {
    Serial.println("BtnB short");
  } else if (M5.BtnB.wasHold()) {
    Serial.println("BtnB long");
  }

  delay(10);
}
