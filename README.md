# One-Joint Arm Reacher (強化学習課題)

本プロジェクトは、1関節アームによるリーチング課題に強化学習（Soft Actor-Critic）を適用したものです。学習エージェントは、手先を目標座標に最短時間かつ滑らかに到達させることを目的とします。

## 使用技術
- Python 2.90
- TensorFlow 2.19
- NumPy, matplotlib
- OpenAI Gym風オリジナル環境

## 実行方法

1. 仮想環境の作成（任意）


2. 学習の実行
```bash
python src/train.py
```

3. 結果の表示
```bash
jupyter notebook notebooks/plot_results.ipynb
```

## ディレクトリ構成

