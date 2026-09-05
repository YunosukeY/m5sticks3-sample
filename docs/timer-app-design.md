# タイマーアプリ設計

仕様は `timer-app-spec.md` を参照。

## ファイル構成

機能ごとにファイルを分ける。全体で 250 行前後を見込む。

```
src/main.cpp     setup / loop、ボタン→イベント変換、sm の構築
src/events.h     イベント型。全体で共有する唯一の定義
src/menu.h       親の遷移表とメニューの描画
src/stopwatch.h  stopwatch サブマシン + StopwatchData + 描画
src/timer.h      timer サブマシン + TimerData + 描画
```

サブマシンが親を知らないという Boost.SML の性質を活かすための分割であり、
「タイマーのことは `timer.h` だけ見れば分かる」状態にする。

`lib/` ではなく `src/` に置くのは、`platformio.ini` のコメントにあるとおり
`build_src_flags` が `src/` にしか効かないため。`lib/` に移すと `-Wall` などが外れる。

拡張子が `.h` なのは、SML のサブマシンが `operator()` で遷移表を返すテンプレートで、
実体をヘッダに置く必要があるため。

## 状態の持ち方

状態遷移は **Boost.SML** で書く。ヘッダオンリーの C++14 ライブラリで、`lib_deps` に
git URL で追加する。遷移を遷移表として宣言的に書けること、サブマシンが親を知らないため
機能単位で切り離せることが採用理由。

### 構造

ストップウォッチとタイマーをサブマシンにし、親はメニューとサブマシンの出入りだけを持つ。

```cpp
struct app {
  auto operator()() const noexcept {
    using namespace sml;
    return make_transition_table(
       *"menu"_s + event<ClickA> / next_cursor      = "menu"_s
      , "menu"_s + event<ClickB> / prev_cursor      = "menu"_s
      , "menu"_s + event<HoldA> [is_stopwatch]      = state<stopwatch>
      , "menu"_s + event<HoldA> [is_timer]          = state<timer>
      , state<stopwatch> + event<HoldB>             = "menu"_s
      , state<timer>     + event<HoldB>             = "menu"_s
    );
  }
};
```

イベントはボタンの4種類 `ClickA` / `ClickB` / `HoldA` / `HoldB` と、時間経過を伝える
`Tick`。`loop()` は `M5.update()` の結果をこれらのイベントに変換して `process_event()` に
渡すだけにする。

### サブマシンから親への遷移

**サブマシンが処理しなかったイベントだけが親の遷移表に流れる。** ホストでコンパイルして
実測した挙動で、これにより仕様書の「戻る」がそのまま書ける。

- 設定（秒）で B 長押し → `timer` が処理し、分の設定へ戻る。親は動かない
- 設定（分）で B 長押し → `timer` は処理しない → 親が処理してメニューへ戻る

つまりサブマシン内の戻り先は内側に書き、サブマシンを抜ける戻りは書かないでおけば、
自動的に親のメニューへ戻る。

### データ

SML の状態は値を持てないため、サブマシンごとにデータ構造体を用意して `sm` の構築時に
注入する。宣言はサブマシンと同じ場所に置き、触るのはそのサブマシンのアクションだけにする。
親から渡した実体がサブマシンのアクションにも届くことは実測で確認した。

```cpp
struct StopwatchData { uint32_t sw_base_ms, sw_start_ms; bool sw_running; };
struct TimerData     { int set_min, set_sec;
                       uint32_t tm_base_ms, tm_start_ms; bool tm_running, tm_beeping; };
struct MenuData      { int menu_cursor; };

MenuData md; StopwatchData swd; TimerData tmd;
sml::sm<app> sm{md, swd, tmd};
```

計測画面の中の「計測中」「満了（ビープ中）」「満了後」は、残り時間と `tm_beeping` から
判定する。状態としては分けない。

## 時間の計測

`millis()` ベースで、**開始時刻と開始時の値を持ち、毎ループ差分を足し引きする**。

```cpp
// ストップウォッチ（加算）
uint32_t cur  = sw_running ? sw_base_ms + (millis() - sw_start_ms) : sw_base_ms;
// タイマー（減算）
uint32_t rest = tm_running ? tm_base_ms - (millis() - tm_start_ms) : tm_base_ms;
```

`sw_base_ms` が「開始時の値」、`sw_start_ms` が「開始時刻」にあたる。
停止したときに `sw_base_ms` へその時点の値を畳み込み、再開時に `sw_start_ms` を
`millis()` で取り直す。畳み込みは ms 単位の整数のまま行うので端数が出ず、
停止・再開を何度挟んでも誤差は蓄積しない。

`delay()` の回数を数える方式（毎ループ 10 を足す）は取らない。
loop 一周の実時間は `delay(10)` より必ず長いため、確実に遅れていく。

`millis()` は約 49.7 日で uint32_t が一周するが、
差分を uint32_t の引き算で求める限り一周をまたいでも正しい値になる。

## 入力

`M5.BtnA.wasClicked()` と `M5.BtnA.wasHold()` を使う。
前者は長押し閾値（既定 500ms）未満で離したとき、後者は閾値到達時に1回だけ発火する。
両方が同時に true になることはないため、短押しと長押しは競合しない。

## 描画

毎ループ、固定位置に全要素を描き直す。前回値との比較はしない。

`setTextColor(前景色, 背景色)` の2引数版を使うと、前景色と背景色が異なる場合に
文字セルの背景も塗られる（`lgfx_fonts.cpp:48`）。
そのため `clear()` を挟まずに同じ位置へ上書きでき、ちらつかない。
ちらつきの原因は `clear()` による一瞬の消去であって、再描画そのものではない。
1引数版は前景色と背景色が同じ値になり背景が塗られないので使わない。

メニューのカーソルも、行頭に `> ` と `  ` を書き分けるだけで前の位置が消える。

画面レイアウトが変わる遷移のときだけ `clear()` する。
遷移を起こすのはボタン処理なので、その場で呼べば「遷移したか」を覚える変数は要らない。

当初は前回の文字列と比較して変化時のみ描画する案だったが、これは早すぎる最適化だった。
比較用バッファと「画面遷移時にそれを無効化する」責務が増え、
無効化を忘れると前の画面の文字が残るというバグを作りやすい。
毎ループ SPI 転送が発生するトレードオフはあるが、描画領域は最大でもメニューの
198x48px、16bit 色で約 19KB/回。100回/秒でも 1.9MB/s であり、
40MHz 駆動の ST7789 に他の転送もないため問題にならない。

スプライトは使わない。ちらつかない以上、二重バッファを持つ理由がない。

## 画面レイアウト（決定・240x135 横向き）

textSize は全画面で **3** に統一する。

```
【メニュー】                   【時間表示】
 ┌────────────────────┐        ┌────────────────────┐
 │                    │        │                    │
 │  > STOPWATCH       │        │       12:34        │
 │    TIMER           │        │       ‾‾           │← 設定画面のみ
 │                    │        │                    │  カーソル
 └────────────────────┘        └────────────────────┘
```

統一サイズの上限を決めるのはメニューの最長行 `> STOPWATCH` で、11文字ある。
textSize 3 では 1文字 18x24px なので 198px となり 240px に収まる。
4 にすると 264px で溢れるため 3 が最大。

同じ textSize 3 で `00:00` は 90x24px になる。
設定画面のカーソルは編集中の桁の下に線を引く。

## ビープ

連続音にする。`tone()` は duration を省略すると停止するまで鳴り続けるため、
満了時に一度呼び、停止操作で `stop()` するだけでよい。

```cpp
M5.Speaker.tone(2000);   // 満了時
M5.Speaker.stop();       // ビープ停止操作
```

音量は既存の `src/main.cpp` と同じ `M5.Speaker.setVolume(255)` を引き継ぐ。
StickS3 は `magnification = 1` かつ音量が2乗で効くため、255 でないと聞こえにくい。
