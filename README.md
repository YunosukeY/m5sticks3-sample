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
