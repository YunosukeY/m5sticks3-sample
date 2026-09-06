#pragma once

#include <M5Unified.h>

#include <algorithm>
#include <boost/sml.hpp>

#include "events.h"

struct TimerData {
  int set_min;           // 設定値 00〜99
  int set_sec;           // 設定値 00〜59
  uint32_t tm_base_ms;   // 計測開始時点の残り
  uint32_t tm_start_ms;  // 計測開始時刻
  bool tm_running;
  bool tm_beeping;
};

inline uint32_t timerRemain(const TimerData& d) {
  if (!d.tm_running) {
    return d.tm_base_ms;
  }
  const uint32_t elapsed = millis() - d.tm_start_ms;
  // 経過が残りを超えるとアンダーフローするので、引く側を残り時間で抑える。
  return d.tm_base_ms - std::min(elapsed, d.tm_base_ms);
}

enum class TimerCursor { Min, Sec };

inline void drawTimerSet(int mm, int ss, TimerCursor cursor) {
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(8, 8);
  M5.Display.print("TIMER SET");
  M5.Display.setTextSize(3);
  M5.Display.setCursor(75, 56);
  M5.Display.printf("%02d:%02d", mm, ss);
  // 使わない側も毎回黒で塗るので、前の位置のカーソルが残らない。
  M5.Display.fillRect(75, 84, 36, 3,
                      cursor == TimerCursor::Min ? TFT_WHITE : TFT_BLACK);
  M5.Display.fillRect(129, 84, 36, 3,
                      cursor == TimerCursor::Sec ? TFT_WHITE : TFT_BLACK);
}

inline void drawTimerRun(uint32_t rest_ms) {
  // 切り上げる。切り捨てると残り 0.1 秒未満の間ずっと 00:00.0 と出てしまう。
  const uint32_t tenth = (rest_ms + 99) / 100;
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(8, 8);
  M5.Display.print("TIMER");
  M5.Display.setTextSize(3);
  M5.Display.setCursor(57, 56);
  M5.Display.printf("%02u:%02u.%u", tenth / 600, tenth / 10 % 60, tenth % 10);
}

const auto timerIsSet = [](const TimerData& d) {
  // 00:00 のままでは計測に進ませない。
  return d.set_min != 0 || d.set_sec != 0;
};
const auto timerIsRunning = [](const TimerData& d) { return d.tm_running; };
const auto timerIsBeeping = [](const TimerData& d) { return d.tm_beeping; };
const auto timerHasRemain = [](const TimerData& d) {
  // 満了時に残りが 0 に畳まれるので、これで満了後かどうかも分かる。
  return d.tm_base_ms != 0;
};

// 入場時のみ通る。前回の設定値を残さず、メニューからレイアウトが変わるので消す。
const auto enterTimer = [](TimerData& d) {
  d = TimerData{0, 0, 0, 0, false, false};
  M5.Display.clear();
};

const auto drawSetMin = [](const TimerData& d) {
  drawTimerSet(d.set_min, d.set_sec, TimerCursor::Min);
};
const auto drawSetSec = [](const TimerData& d) {
  drawTimerSet(d.set_min, d.set_sec, TimerCursor::Sec);
};

const auto incMin = [](TimerData& d) { d.set_min = (d.set_min + 1) % 100; };
const auto decMin = [](TimerData& d) { d.set_min = (d.set_min + 99) % 100; };
const auto incSec = [](TimerData& d) { d.set_sec = (d.set_sec + 1) % 60; };
const auto decSec = [](TimerData& d) { d.set_sec = (d.set_sec + 59) % 60; };

const auto enterRun = [](TimerData& d) {
  d.tm_base_ms = static_cast<uint32_t>(d.set_min * 60 + d.set_sec) * 1000;
  d.tm_start_ms = millis();
  d.tm_running = true;
  // 設定画面から桁数も画面名も変わるので消す。
  M5.Display.clear();
};

// 残りを再計算し、0 になった瞬間にビープを始めて描き直す。
const auto tickTimer = [](TimerData& d) {
  const uint32_t rest = timerRemain(d);
  if (d.tm_running && rest == 0) {
    d.tm_running = false;
    d.tm_base_ms = 0;
    d.tm_beeping = true;
    M5.Speaker.tone(2000);
  }
  drawTimerRun(rest);
};

const auto stopBeep = [](TimerData& d) {
  M5.Speaker.stop();
  d.tm_beeping = false;
};

const auto pauseTimer = [](TimerData& d) {
  d.tm_base_ms = timerRemain(d);
  d.tm_running = false;
};

const auto resumeTimer = [](TimerData& d) {
  d.tm_start_ms = millis();
  d.tm_running = true;
};

// 設定画面へ戻るときに鳴りっぱなしにしない。桁数も画面名も変わるので消す。
const auto leaveRun = [](TimerData& d) {
  if (d.tm_beeping) {
    M5.Speaker.stop();
    d.tm_beeping = false;
  }
  M5.Display.clear();
};

struct timer {
  auto operator()() const noexcept {
    using namespace boost::sml;
    return make_transition_table(
        *"init"_s / enterTimer = "setMin"_s,

        "setMin"_s + event<Tick> / drawSetMin,
        "setMin"_s + event<ClickA> / incMin,
        "setMin"_s + event<ClickB> / decMin,
        "setMin"_s + event<HoldA> = "setSec"_s,
        // 分の設定から戻るとメニューへ。X
        // に遷移してこのサブマシンを終わらせる。
        "setMin"_s + event<HoldB> = X,

        "setSec"_s + event<Tick> / drawSetSec,
        "setSec"_s + event<ClickA> / incSec,
        "setSec"_s + event<ClickB> / decSec,
        "setSec"_s + event<HoldA>[timerIsSet] / enterRun = "run"_s,
        "setSec"_s + event<HoldB> = "setMin"_s,

        "run"_s + event<Tick> / tickTimer,
        // 上から順に、最初に一致した行だけが実行される。
        // 満了後はどれにも一致せず何も起きない。
        "run"_s + event<ClickA>[timerIsBeeping] / stopBeep,
        "run"_s + event<ClickA>[timerIsRunning] / pauseTimer,
        "run"_s + event<ClickA>[timerHasRemain] / resumeTimer,
        // 戻るは停止時のみ。計測中はどの行にも当てはまらず何も起きない。
        "run"_s + event<HoldB>[!timerIsRunning] / leaveRun = "setMin"_s);
  }
};
