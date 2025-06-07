//シミュレーション条件
#define MAX_STEP 1000//step数
#define MAX_EPISODE 500000//episode数
#define DT 0.02//時間の刻み幅
#define L1 0.5//上腕の腕の長さ
#define L2 0.5//前腕の腕の長さ
#define TEMP 5 //softmaxの温度パラメータ
#define TIME_PUNISH 0.27 //時間制限

//手先関係
#define GOAL_X 0.0
#define GOAL_Y 1.0
#define THRESHOLD 0.03 //目標地点と手先座標の距離の閾値

//角度関係
#define INITIAL_SHOULDER 60
#define INITIAL_ELBOW 0
#define TARGET_SHOULDER 90
#define TARGET_ELBOW 0
#define THETA_VEL_MAX 4.0
#define MAX_THETA_SHOULDER_ARRAY 135//肩関節の上限値
#define MAX_THETA_ELBOW_ARRAY 145//肘関節の上限値
#define MIN_THETA_SHOULDER_ARRAY 0//肩関節の上限値
#define MIN_THETA_ELBOW_ARRAY 0//肘関節の上限値

//基底関数関係
#define STATE 8  // 状態側の基底関数の数
#define ACTION 7 // 行動側の基底関数の数
#define WIDTH_STATE 3.2 //状態側の基底関数の幅
#define WIDTH_ACTION 0.5 //行動側の基底関数の幅
#define ACTION_SELECT 4.0 //最大のaction
#define MAX_ACTION_NUM 400 //actionの刻む数
#define D_ACTION 0.01 //actionの刻み幅


//強化学習関係
#define GAMMA 0.99       // 割引率
#define ALPHA_CRITIC 0.05 // Criticの学習率
#define ALPHA_ACTOR 0.01 // Actorの学習率
#define PI 3.1415926535 //PI
#define VELOCITY_SUM_P 15.0 //手先の速さに関する報酬の重み
#define COUNT_JERK_SUM_P 0.000000002//ゴールした際のjerkの総和による報酬に対する重み
#define JERK_SUM_P 0.000000000005
#define REWARD_P 100 //ゴールした際の報酬
#define REWARD_DP 1.0 //位置誤差に関する報酬に対する重み
#define TIME_PUNISH_P 10 //時間に関する報酬に対する重み
#define SWITCH_V_R 0//1のときはr_vあり
#define SWITCH_J_R 0