FROM debian:bookworm-slim

# 基本ツールのインストール
RUN apt-get update && \
    apt-get install -y gcc make python3 procps && \
    rm -rf /var/lib/apt/lists/*

# 作業ディレクトリ
WORKDIR /app

# ファイルを追加
COPY . .

# ビルド
RUN gcc -O2 -o mt_predictor mt_predictor.c mt19937ar.c

# 実行エントリ
CMD ["/bin/bash"]
