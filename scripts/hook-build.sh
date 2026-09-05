#!/usr/bin/env bash
# Stop の入力には編集ファイルの情報が無いので、hook-format.sh が置いた
# マーカーの有無で「対象ファイルが編集されたか」を判定してビルドする。

# set -e は入れない。devbox run build の失敗は 1 や 127 で、Stop をブロックする 2
# ではないため、|| exit 2 で明示的に変換する必要がある。
set -uo pipefail

[ -f .pio/.needs-build ] || exit 0
rm -f .pio/.needs-build

devbox run build || exit 2
