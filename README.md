# One-Joint Arm Reacher (強化学習課題)

本プロジェクトは、1関節アームによるリーチング課題に強化学習（Soft Actor-Critic）を適用したものです。学習エージェントは、手先を目標座標に最短時間かつ滑らかに到達させることを目的とします。

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
jupyter notebook notebooks/plot_results.ipynb
```

## ディレクトリ構成

