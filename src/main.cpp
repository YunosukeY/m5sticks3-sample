#include <M5Unified.h>

#include <boost/sml.hpp>

#include "events.h"
#include "menu.h"

namespace sml = boost::sml;

static MenuData menu_data{MenuCursor::Stopwatch};
static StopwatchData stopwatch_data{0, 0};
static TimerData timer_data{0, 0, 0, 0, false, false};

static sml::sm<app> machine{menu_data, stopwatch_data, timer_data};

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);

  M5.Display.setRotation(1);
  M5.Speaker.setVolume(255);
}

void loop() {
  M5.update();

  if (M5.BtnA.wasClicked()) machine.process_event(ClickA{});
  if (M5.BtnB.wasClicked()) machine.process_event(ClickB{});
  if (M5.BtnA.wasHold()) machine.process_event(HoldA{});
  if (M5.BtnB.wasHold()) machine.process_event(HoldB{});

  machine.process_event(Tick{});

  delay(10);
}
