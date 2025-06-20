# One-Joint Arm Reacher (強化学習課題)

本プロジェクトは、1関節アームによるリーチング課題に強化学習を適用したものです。学習エージェントは、手先を目標座標に決まった時間で滑らかに到達させることを目的とします。

## ディレクトリ構成

```plaintext
portfolio/
├── one_joint_reacher/              # 基底関数を用いたC言語版の強化学習（シンプルな関数近似による方策学習）
│   ├── main.c                      # メイン処理
│   ├── function.c                  # 方策や更新式などの関数群
│   └── param.h                     # 各種定数・ハイパーパラメータ
│
├── sac_one_joint_reacher/         # SAC（Soft Actor-Critic）による深層強化学習（Python）
│   ├── sac.ipynb                  # SAC実装（ノートブック）
│   └── graph.ipynb                # 解析・可視化用ノートブック
│
└── requirements.txt               # Pythonで必要なライブラリ
```

## 特徴・工夫点
- **物理ダイナミクスの再現**（角速度・加速度・躍度を計算）
- **報酬関数の調整**（時期に応じて躍度ペナルティの重みを変化）
- **GPU対応のSAC実装**（TensorFlow）
- **可視化・ログ保存**（CSV出力とmatplotlibグラフ）
- C言語版の併設：強化学習の方策を複数の基底関数で近似。ハイパーパラメータ（幅・中心・数など）の調整が難しく、精度向上に限界があったため、後述の深層強化学習（SAC）に展開。

## C言語バージョン
one_joint_reacher/ ディレクトリに含まれる C 言語版では、強化学習の方策を複数の基底関数を用いて近似しています。C言語が扱えることの証明として成果物に含めています。

使用ファイル：

main.c：エントリーポイント

function.c：報酬・行動選択・学習更新などの関数定義

param.h：ハイパーパラメータの定義

全ファイルを同一ディレクトリに配置すればコンパイル・実行が可能です。

※現在のバージョンではハイパーパラメータ調整が課題となり、十分な性能は出ていません。これを解決するために、深層強化学習（SAC）を採用したバージョンを新たに作成しました。

## 使用技術
- Python 2.90
- TensorFlow 2.19
- NumPy, matplotlib
- OpenAI Gym風オリジナル環境
- C言語（GCCコンパイル確認済み）

## 実行方法
1. 必要なライブラリのインストール：

```bash
pip install -r requirements.txt
```

2. （GPU使用時）CUDA 12.5, cuDNN 9.3 の環境が整っていることを確認

3. 学習を実行：

```bash
jupyter notebook sac_one_joint_reacher/sac.ipynb
```

4. 結果を可視化：

```bash
jupyter notebook sac_one_joint_reacher/plot_results.ipynb
```
---
## 進捗状況
1. 時間制約下での目標到達は学習可能であることを確認  
2. 滑らかな軌道（低躍度）を目指す報酬関数を設計  
3. 躍度ペナルティの重みを徐々に増やす**カリキュラム学習**を実装（タスクの段階的難易度調整）

---



