#!/usr/bin/env bash
# hooks の if は「1ツール名 + 1グロブ」しか書けず OR が使えないため、
# 拡張子を判定したうえで検査する。

set -euo pipefail

path=$(jq -r '.tool_input.file_path')
case "$path" in
  *.sh) ;;
  *) exit 0 ;;
esac

# 指摘は stdout に出るが、Claude に返るのは stderr なので寄せる。
# また PostToolUse をブロックする終了コードは 2 だけなので変換する。
devbox run lint:sh >&2 || exit 2
