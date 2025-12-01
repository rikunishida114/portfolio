# Two-Link Arm Reacher（2リンク・リーチング）

**2関節アームのリーチング課題を SB3（SAC / PPO 等）で学習・可視化するリポジトリ**  

---

## 目次
1. 概要  
2. ディレクトリ構成  
3. 特徴・工夫点  
4. 使用技術（推奨環境）  
5. 実行方法（CPU 向け）  
6. 出力ファイル（`logs/` の説明）  
---

## 1. 概要
本プロジェクトは、2関節の平面アーム（肩を原点とする planar two-link manipulator）に対して、リーチング（手先を目標に到達させる）タスクを強化学習で解くための実装です。報酬は到達距離・時間ボーナス・手先躍度（jerk）のペナルティ等を組み合わせています。これによって、キネマティクスのみで躍度最小化軌道に近しい滑らかな手先軌道を実現しています。また、ログと可視化機能を充実させ、学習過程や最終挙動を詳しく解析できるようになっています。

---

## 2. ディレクトリ構成
```
portfolio/
├── one_rink_reacher/              # C言語版（基底関数近似の実験）
│   ├── main.c
│   ├── function.c
│   └── param.h
├── two_rink_reacher/              # Python / SB3 実装（メイン）
│   ├── jerk_min.ipynb             # 学習・評価
│   ├── logs/                      # 学習・評価の出力（CSV・PNG・モデル等）
│   └── models/                    # 学習済みモデル（オプション）
└──-------------------             
```

---

## 3. 特徴・工夫点
- 手先の位置・速度・加速度・躍度（jerk）を計算し、報酬やログに反映。  
- 時間ボーナス（ガウス関数）を導入し、所要時間のトレードオフを制御。  
- 直近 100 エピソードの成功率に応じて ゴールした際の報酬を自動調整する仕組みを実装（カリキュラム的効果）。  
- 各エピソードの時系列データ（関節角・手先位置・報酬成分）を CSV に記録し、複数のプロットを自動保存。  
- 可視化：手先軌道＋アーム姿勢、関節動的変数プロット、手先ノルム（||v||, ||a||, ||jerk||）など。

---

## C言語バージョン
one_rink_reacher/ ディレクトリに含まれる C 言語版では、強化学習の方策を複数の基底関数を用いて近似しています。C言語が扱えることの証明として成果物に含めています。

使用ファイル：

main.c：エントリーポイント

function.c：報酬・行動選択・学習更新などの関数定義

param.h：ハイパーパラメータの定義

全ファイルを同一ディレクトリに配置すればコンパイル・実行が可能です。

※現在のバージョンではハイパーパラメータ調整が課題となり、十分な性能は出ていません。これを解決するために、深層強化学習を採用したバージョンを新たに作成しました。

## 4. 使用技術（推奨環境）
- Python 3.9 ～ 3.11 を推奨（3.12+ は依存で不整合が出る場合あり）  
- CPU 実行 
- ライブラリ例：PyTorch（CPU）、stable-baselines3、gymnasium、numpy、pandas、matplotlib など

---

## 5. 実行方法（CPU 向け）
1. リポジトリをクローン
```bash
git clone https://github.com/rikunishida114/portfolio.git
cd portfolio
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
5. Jupyter を起動してノートブックを実行
```bash
jupyter notebook
# ブラウザで次を実行:
# jerk_min.ipynb   -> 学習、結果可視化
```

## 6. 出力ファイル（`logs/` の説明）
`logs/` には学習中・評価時に生成した画像・CSV・モデル等を保存しています。主要なファイルと説明は次のとおりです（ファイル名は実行ごとに異なる場合があります）。

- `all_reward_components_overlay_ma_w10.png`  
  → 各報酬成分（距離改善・躍度・時間コスト・終端ペナルティ等）を **移動平均（window=10エピソード）** でオーバーレイ表示した図。報酬成分ごとの寄与推移を比較できます。

- `group_reward_components_ma_w10.png`  
  → 成分群（例：shaping 成分・terminal 成分）ごとにまとめた移動平均プロット。グループ間の振る舞いを俯瞰できます。

- `episode_jerk_sum_ma_w10.png`  
  → 各エピソードの累積手先躍度（jerk_sum）の移動平均（window=10）推移図。運動の滑らかさの変化を確認できます。

- `episode_sigma_T_ma_w10.png`  
  → 直近 100 エピソードに基づいて自動更新された `sigma_T` の時間推移（移動平均）。`sigma_T` の先鋭化／鈍化挙動を確認可能。ゴールした際の報酬の自動調節に関わるパラメータ

- `end_result_2joint_with_handnorms_trajectory_with_arms.png`  
  → 最終評価エピソードの手先軌道に加え、時刻ごとのアーム姿勢（関節を丸、リンクをやや太めの線で描画）を重ねた図。肩を原点とし、角度変化が視覚的に分かるようになっています。

- `end_result_2joint_with_handnorms_hand_norms.png`  
  → 最終エピソードにおける手先のノルム（速度 ||v||、加速度 ||a||、躍度 ||jerk||）の時間推移図。

- `end_result_2joint_with_handnorms_joint_dynamics.png`  
  → 各関節（joint1, joint2）の角度（deg）、角速度（deg/s）、角加速度、角躍度の時間推移をまとめた図。

---

