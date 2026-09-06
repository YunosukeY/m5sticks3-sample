#pragma once

// ボタンイベント。短押しが画面の中身、長押しが画面移動に対応する。
struct ClickA {};
struct ClickB {};
struct HoldA {};
struct HoldB {};

// 毎ループ送る。時間の再計算と再描画のきっかけ。
struct Tick {};
