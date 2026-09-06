#pragma once

#include <M5Unified.h>

#include <boost/sml.hpp>

#include "events.h"

struct StopwatchData {
  uint32_t sw_base_ms;   // 停止した時点までに確定した経過
  uint32_t sw_start_ms;  // 計測中の起点
};

inline void drawElapsed(uint32_t elapsed_ms) {
  const uint32_t sec = elapsed_ms / 1000;
  M5.Display.setTextSize(3);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(75, 56);
  M5.Display.printf("%02u:%02u", sec / 60, sec % 60);
}

// SML のアクションはクラス型の callable でなければならず、自由関数は渡せない。
// ラムダにしておくと遷移表にそのまま書ける。

const auto drawStopped = [](const StopwatchData& d) {
  drawElapsed(d.sw_base_ms);
};
const auto drawRunning = [](const StopwatchData& d) {
  drawElapsed(d.sw_base_ms + (millis() - d.sw_start_ms));
};

// 入場時のみ通る。前回の計測値を残さず、メニューからレイアウトが変わるので消す。
const auto enterStopwatch = [](StopwatchData& d) {
  d = StopwatchData{0, 0};
  M5.Display.clear();
};

const auto startStopwatch = [](StopwatchData& d) { d.sw_start_ms = millis(); };

const auto stopStopwatch = [](StopwatchData& d) {
  d.sw_base_ms += millis() - d.sw_start_ms;
};

const auto resetStopwatch = [](StopwatchData& d) { d.sw_base_ms = 0; };

struct stopwatch {
  auto operator()() const noexcept {
    using namespace boost::sml;
    return make_transition_table(
        // running から stopped に戻るときはここを通らない。
        *"init"_s / enterStopwatch = "stopped"_s,

        "stopped"_s + event<Tick> / drawStopped,
        "stopped"_s + event<ClickA> / startStopwatch = "running"_s,
        "stopped"_s + event<ClickB> / resetStopwatch,
        // 戻るは停止時のみ。X に遷移してこのサブマシンを終わらせる。
        "stopped"_s + event<HoldB> = X,

        "running"_s + event<Tick> / drawRunning,
        "running"_s + event<ClickA> / stopStopwatch = "stopped"_s);
    // running の HoldB は書かない。X に行かないので親も動かない。
  }
};
