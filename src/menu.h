#pragma once

#include <M5Unified.h>

#include <boost/sml.hpp>

#include "events.h"
#include "stopwatch.h"
#include "timer.h"

enum class MenuCursor { Stopwatch, Timer };

struct MenuData {
  MenuCursor menu_cursor;
};

// 二行しかないので、下へも上へも同じ移動になる。
const auto moveMenuCursor = [](MenuData& d) {
  d.menu_cursor = d.menu_cursor == MenuCursor::Stopwatch
                      ? MenuCursor::Timer
                      : MenuCursor::Stopwatch;
};

// SML のアクションはクラス型の callable でなければならず、自由関数は渡せない。
// ラムダにしておくと遷移表にそのまま書ける。
const auto drawMenu = [](const MenuData& d) {
  M5.Display.setTextSize(3);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(21, 40);
  M5.Display.printf("%c STOPWATCH",
                    d.menu_cursor == MenuCursor::Stopwatch ? '>' : ' ');
  // STOPWATCH と桁を揃えて空白で埋め、前に描いた文字を消す。
  M5.Display.setCursor(21, 76);
  M5.Display.printf("%c TIMER    ",
                    d.menu_cursor == MenuCursor::Timer ? '>' : ' ');
};

const auto menuOnStopwatch = [](const MenuData& d) {
  return d.menu_cursor == MenuCursor::Stopwatch;
};
const auto menuOnTimer = [](const MenuData& d) {
  return d.menu_cursor == MenuCursor::Timer;
};

const auto clearDisplay = [] { M5.Display.clear(); };

struct app {
  auto operator()() const noexcept {
    using namespace boost::sml;
    return make_transition_table(
        *"menu"_s + event<Tick> / drawMenu,
        "menu"_s + event<ClickA> / moveMenuCursor,
        "menu"_s + event<ClickB> / moveMenuCursor,
        "menu"_s + event<HoldA>[menuOnStopwatch] = state<stopwatch>,
        "menu"_s + event<HoldA>[menuOnTimer] = state<timer>,

        // イベント無しの無名遷移。サブマシンが X に達したときだけ発火する。
        // 戻る条件はサブマシン側が持ち、親はレイアウトを消すだけ。
        state<stopwatch> / clearDisplay = "menu"_s,
        state<timer> / clearDisplay = "menu"_s);
  }
};
