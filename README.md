# M5StickS3 Sample

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
| `devbox run format` | `src` / `lib` / `include` を clang-format で整形 |
| `devbox run format:check` | 整形済みかを確認する（変更はしない） |
| `devbox run lint:sh` | `scripts/*.sh` を shellcheck で検査 |
