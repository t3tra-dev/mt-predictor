# mt-predictor

`mt-predictor` は実行中の Python プロセスのメモリをスキャンして、`random.random()` が次に出力する値を **完全に予測** するためのものです。  
教育・研究・セキュリティ検証を目的としています。

詳細は以下の記事で解説しています:

https://zenn.dev/t3tra/articles/f3f9040788c790

## Features

- CPython の `Mersenne Twister` の内部状態 (`mt[624]` + `index`)をメモリから直接解析
- 完全な再現性: `random.random()` の出力が **100% 一致**
- Linux 向け (Docker で簡単に再現可)

## Usage

### 1. ビルド

```bash
docker build -t mt-predictor .
```

### 2. Docker コンテナを起動

```bash
docker run -it --rm --cap-add=SYS_PTRACE --security-opt seccomp=unconfined mt-predictor
```
> `--cap-add=SYS_PTRACE` と `--security-opt=unconfined` は他プロセスのメモリにアクセスするために必要です。

### 3. Python を起動

```bash
python3 victim.py &
```
> `victim.py` は `os.getpid()` を出力した後、`time.sleep(10)` で 10 秒間スリープし `random.random()` の結果を出力します。
> `&` はバックグラウンドで実行するためのものです。

### 4. `mt-predictor` を実行

`victim.py` の待機時間(`time.sleep(10)`)中に、ターミナルで以下のコマンドを実行します。
```bash
./mt-predictor <PID>
```
> `<PID>` は `victim.py` のプロセス ID です。先ほどの `os.getpid()` の出力を確認してください。
> `mt-predictor.py` は対象プロセスのメモリアドレスを検索し `random.random()` の出力を 3 回先まで予測します。

### 5. 出力を確認

例えば以下のような出力が得られます。

```rb
root@920d7178d451:/app# python3 victim.py &
[1] 8
root@920d7178d451:/app# PID: 8
./mt_predictor 8
[+] state @ offset 0x84b0
    next[1] random.random() = 0.23343851299819518
    next[2] random.random() = 0.28635628575684968
    next[3] random.random() = 0.14805106308346927
root@920d7178d451:/app# random.random(): 0.23343851299819518

[1]+  Done                    python3 victim.py
root@920d7178d451:/app# 
```

## Directory

```py
Dockerfile      # Docker 構成用
README.md       # 本ファイル
mt19937ar.c     # Mersenne Twister 実装 (広島大学提供 + 修正)
mt19937ar.h     # 上記実装のヘッダ
mt_predictor.c  # 本体の C プログラム
victim.py       # 予測対象の Python プログラム
```

## Caution / Disclaimer

- このツールは教育・研究・セキュリティ検証を目的としています。
- ご利用は自己責任でお願いします。

## License

このプログラムは MIT ライセンスに基づいています。

広島大学の Mutsuo Saito 氏らによる Mersenne Twister 実装については BSD 3 条項ライセンスに基づいています。

- - -

```
MIT License

Copyright (c) 2025-present t3tra

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

- - -

```
Copyright (c) 2011 Mutsuo Saito, Makoto Matsumoto, Hiroshima
University and The University of Tokyo. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of the Hiroshima University nor the names of
      its contributors may be used to endorse or promote products
      derived from this software without specific prior written
      permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```
