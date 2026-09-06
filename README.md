# M5StickS3 Sample

## 技術スタック

ファームウェア構成

- デバイス：M5StickS3
- プラットフォーム：pioarduino platform-espressif32
- フレームワーク：Arduino
- ライブラリ：M5Unified、M5GFX
- 状態遷移：Boost.SML

開発用途

- エージェント：Claude Code
- パッケージマネージャー、タスクランナー：devbox / direnv
- ビルド：PlatformIO Core
- Formatter：clang-format
- Linter：shellcheck
- 単体テスト：Unity
- CI：GitHub Actions

## セットアップ

以下がインストールされているとします。

- devbox
- direnv

フック追加

```sh
echo 'eval "$(direnv hook zsh)"' >> ~/.zshrc
```

シェルを再読み込み

```sh
exec $SHELL
```

環境を有効化

```sh
direnv allow
```

## コマンド

| コマンド | 内容 |
| --- | --- |
| `devbox run build` | ビルド |
| `devbox run upload` | ビルドして実機に書き込み |
| `devbox run monitor` | シリアルモニタ |
| `devbox run test` | ホスト上で単体テストを実行 |
| `devbox run format` | `src` / `lib` / `include` / `test` を clang-format で整形 |
| `devbox run format:check` | 整形済みかを確認する（変更はしない） |
| `devbox run lint:sh` | `scripts/*.sh` を shellcheck で検査 |
