# Two-Link Arm Reacher（2リンク・リーチング）

**2関節アームのリーチング課題を SB3（SAC / PPO 等）で学習・可視化するリポジトリ**  
就活ポートフォリオ用にまとめたデモです。CPU 環境で再現可能なように調整しています。

---

## 目次
1. 概要  
2. ディレクトリ構成  
3. 特徴・工夫点  
4. 使用技術（推奨環境）  
5. 実行方法（CPU 向け）  
6. 出力ファイル（`logs/` の説明）  
7. `requirements.txt`（推奨中身）  
8. トラブルシュート & 補足  
9. 提出物チェックリスト

---

## 1. 概要
本プロジェクトは、2関節の平面アーム（肩を原点とする planar two-link manipulator）に対して、リーチング（手先を目標に到達させる）タスクを強化学習で解くための実装です。報酬は到達距離・時間ボーナス・手先躍度（jerk）のペナルティ等を組み合わせています。ログと可視化機能を充実させ、学習過程や最終挙動を詳しく解析できるようになっています。

---

## 2. ディレクトリ構成（例）
```
portfolio/
├── one_rink_reacher/              # C言語版（基底関数近似の実験）
│   ├── main.c
│   ├── function.c
│   └── param.h
├── two_rink_reacher/              # Python / SB3 実装（メイン）
│   ├── notebooks/                 # Jupyter notebooks (train / plot)
│   │   ├── sac_two_link_reacher_train.ipynb
│   │   └── sac_two_link_reacher_plot.ipynb
│   ├── src/                       # 環境クラスやユーティリティ
│   │   └── envs.py                # TwoJointReachingEnv 等
│   ├── logs/                      # 学習・評価の出力（CSV・PNG・モデル等）
│   └── models/                    # 学習済みモデル（オプション）
└── requirements.txt               # 推奨パッケージ一覧
```

---

## 3. 特徴・工夫点
- 手先の位置・速度・加速度・躍度（jerk）を計算し、報酬やログに反映。  
- 時間ボーナス（ガウス等）を導入し、所要時間のトレードオフを制御可能。  
- 直近 100 エピソードの成功率に応じて `sigma_T` を自動調整する仕組みを実装（カリキュラム的効果）。  
- 各エピソードの時系列データ（関節角・手先位置・報酬成分）を CSV に記録し、複数のプロットを自動保存。  
- 可視化：手先軌道＋アーム姿勢、関節動的変数プロット、手先ノルム（||v||, ||a||, ||jerk||）など。

---

## 4. 使用技術（推奨環境）
- Python 3.9 ～ 3.11 を推奨（3.12+ は依存で不整合が出る場合あり）  
- CPU 実行（GPU は不要）  
- ライブラリ例：PyTorch（CPU）、stable-baselines3、gymnasium、numpy、pandas、matplotlib など

---

## 5. 実行方法（CPU 向け）
1. リポジトリをクローン
```bash
git clone https://github.com/<your-username>/<repo-name>.git
cd <repo-name>
```

2. 仮想環境を作る（任意だが推奨）  
Linux / macOS:
```bash
python -m venv .venv
source .venv/bin/activate
```
Windows (PowerShell):
```powershell
python -m venv .venv
.venv\Scripts\Activate.ps1
```

3. まず CPU 用の PyTorch を入れる（推奨）
> PyTorch は環境によってバイナリが異なるため、先に CPU 用 wheel を指定してインストールするのがおすすめです。下は一例（OS/Python によりコマンドは変わるので、必要なら公式サイトの「Get Started」で生成されたコマンドを使ってください）。
```bash
pip install torch --index-url https://download.pytorch.org/whl/cpu
```

4. Stable-Baselines3（SB3） とその extras をインストール
```bash
pip install 'stable-baselines3[extra]'
# sb3-contrib を使っている場合は次も
pip install sb3-contrib
```

5. （あるいは一括で）`requirements.txt` を使う
```bash
pip install -r requirements.txt
```

6. Jupyter を起動してノートブックを実行
```bash
jupyter notebook
# ブラウザで次を実行:
# notebooks/sac_two_link_reacher_train.ipynb   -> 学習
# notebooks/sac_two_link_reacher_plot.ipynb    -> 結果可視化
```

7. 学習済みモデルを使って評価する（スクリプト例）
```bash
python scripts/eval.py --model logs/<run>/best_model.zip --episodes 10 --out_dir logs/last_eval
```
（`scripts/eval.py` を用意していれば上のように実行できます。ノートブック内の評価セルでも同様に評価可能です）

---

## 6. 出力ファイル（`logs/` の説明）
`logs/` には学習中・評価時に生成した画像・CSV・モデル等を保存しています。主要なファイルと説明は次のとおりです（ファイル名は実行ごとに異なる場合があります）。

- `all_reward_components_overlay_ma_w10.png`  
  → 各報酬成分（距離改善・躍度・時間コスト・終端ペナルティ等）を **移動平均（window=10エピソード）** でオーバーレイ表示した図。報酬成分ごとの寄与推移を比較できます。

- `group_reward_components_ma_w10.png`  
  → 成分群（例：shaping 成分・terminal 成分）ごとにまとめた移動平均プロット。グループ間の振る舞いを俯瞰できます。

- `episode_jerk_sum_ma_w10.png`  
  → 各エピソードの累積手先躍度（jerk_sum）の移動平均（window=10）推移図。運動の滑らかさの変化を確認できます。

- `episode_sigma_T_ma_w10.png`  
  → 直近 100 エピソードに基づいて自動更新された `sigma_T` の時間推移（移動平均）。`sigma_T` の先鋭化／鈍化挙動を確認可能。

- `end_result_2joint_with_handnorms_trajectory_with_arms.png`  
  → 最終評価エピソードの手先軌道に加え、時刻ごとのアーム姿勢（関節を丸、リンクをやや太めの線で描画）を重ねた図。肩を原点とし、角度変化が視覚的に分かるようになっています。

- `end_result_2joint_with_handnorms_hand_norms.png`  
  → 最終エピソードにおける手先のノルム（速度 ||v||、加速度 ||a||、躍度 ||jerk||）の時間推移図。

- `end_result_2joint_with_handnorms_joint_dynamics.png`  
  → 各関節（joint1, joint2）の角度（deg）、角速度（deg/s）、角加速度、角躍度の時間推移をまとめた図。

> これらの図は `notebooks/sac_two_link_reacher_plot.ipynb` の可視化セルから生成されます。`logs/` に含めておくと、レビュー時に即座に結果を確認してもらえます。

---

## 7. `requirements.txt`（推奨中身）
以下を `requirements.txt` に保存しておくと再現しやすいです（推奨バージョン）。環境に応じて調整してください。

```
# Python
python>=3.9,<3.12

# 基本数値・可視化
numpy>=1.23
pandas>=1.5
matplotlib>=3.6
seaborn>=0.12
scipy>=1.8
tqdm>=4.64

# PyTorch: 事前に CPU 用 wheel を入れることを推奨
torch>=2.1.0

# 強化学習ライブラリ
stable-baselines3>=1.8.0
sb3-contrib>=2.0.0   # optional

# Gym
gymnasium>=0.28.1

# Jupyter / Notebook
jupyterlab
notebook
tensorboard    # optional

# utils
click
```

**備考**  
- `torch` は `pip install torch --index-url https://download.pytorch.org/whl/cpu` のように CPU 用 wheel を先に入れておくと依存解決で失敗しにくくなります（GPU を使わない場合）。  
- `stable-baselines3` は `pip install 'stable-baselines3[extra]'` で extras（TensorBoard 等）も入りますが、`torch` を先に入れると安全です。

---

## 8. トラブルシュート & 補足
- **SB3 インストールでエラーが出る**：まず `torch` を先に入れてから `stable-baselines3` を再試行してください。  
- **Python バージョンの問題**：3.9～3.11 を推奨。3.12 以降だと一部パッケージで wheel が未対応の場合があります。  
- **models/ に小さな学習済みファイルを入れておくと便利**：レビュアーがすぐに結果を確認できます（トレーニング時間を待たずに可視化が可能）。  
- **再現性**：`config.py`（存在する場合）で `seed` を固定して実行すると再現性が高まります。README に学習シードや主要ハイパーパラメータを記しておくのが親切です。  
- **logs** フォルダの主要画像は README に記載の通り説明を付けています。追加で説明が必要なファイルがあれば README に追記してください。

---

## 9. 提出物チェックリスト（ポートフォリオ準備）
- [ ] `README.md` をルートに配置（このファイル）  
- [ ] `requirements.txt` をルートに配置（上記の内容）  
- [ ] `notebooks/` に学習・可視化ノートブックを用意（デバッグ用に短い実行セルを含める）  
- [ ] `logs/` に代表的 PNG（`trajectory_with_arms.png` 等）を配置（レビュアー向け）  
- [ ] `models/` に小さめの学習済みモデルを1つ置く（任意だが便利）  

---

必要であれば、この README をさらに短くした「一行サマリ」や、履歴書／ポートフォリオに貼る形式（短縮版）へ整形します。また、`notebooks/` に「すぐに動かせる」短縮セル（デバッグ用）を追加することもできます。どれを先に作りましょうか？
