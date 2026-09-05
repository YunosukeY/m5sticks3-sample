#!/usr/bin/env bash
# hooks の if は「1ツール名 + 1グロブ」しか書けず OR が使えないため、
# 拡張子を判定したうえでフォーマットする。

set -euo pipefail

path=$(jq -r '.tool_input.file_path')
case "$path" in
  *.h | *.cpp) ;;
  *) exit 0 ;;
esac
devbox run format

# Stop フックは編集ファイル情報を受け取れないため、ここでビルド要求を残す
# ただしテストの変更で再ビルドは不要
case "$path" in
  */test/*) exit 0 ;;
esac
mkdir -p .pio && touch .pio/.needs-build
