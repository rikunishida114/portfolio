# One-Joint Arm Reacher (強化学習課題)

本プロジェクトは、1関節アームによるリーチング課題に強化学習を適用したものです。学習エージェントは、手先を目標座標に決まった時間で滑らかに到達させることを目的とします。

## ディレクトリ構成

```plaintext
portfolio/
├── one_joint_reacher/  　# 報酬を最大化するような方策を複数の基底関数を用いて近似することを目的としたリーチングの強化学習（C言語）⇨ 基底関数の幅や中心、数などのハイパーパラメータの調整が難しい
├── sac_one_joint_reacher/        # Soft Actor-Critic（SAC）を用いて、深層強化学習で様々な目的に応じた報酬を最大化するように学習させるリーチングの強化学習
└── requirements.text/           # 必要なライブラリ
```

## 特徴・工夫点
- **物理ダイナミクスの再現**（角速度・加速度・躍度を計算）
- **報酬関数の調整**（時期に応じて躍度ペナルティの重みを変化）
- **GPU対応のSAC実装**（TensorFlow）
- **可視化・ログ保存**（CSV出力とmatplotlibグラフ）

## 使用技術
- Python 2.90
- TensorFlow 2.19
- NumPy, matplotlib
- OpenAI Gym風オリジナル環境

## 実行方法
1. 必要なライブラリのインストール：requirements.txtに書かれているパッケージをインストールしてください
2. GPUが使える環境にある場合は、適宜 cuda 12.5, cudnn 9.3 をインストールしてください

3. 学習の実行
```bash
jupyter notebook sac_one_joint_reacher/plot_results.ipynb
```

4. 結果の表示
```bash
jupyter notebook sac_one_joint_reacher/plot_results.ipynb
```
## 進捗状況
1. 決まった時間に到達するように学習させることはできた
2. 報酬に躍度を入れ、滑らかな軌道を学習させる → 重みのパラメータの調整が難しい
3. 最初にある程度、目標地点に手先が届くように学習させてから、躍度の重みを徐々に学習とともに変化させていくカリキュラム学習の実装



