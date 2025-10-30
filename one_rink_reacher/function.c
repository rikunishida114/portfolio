#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "param.h"


void copy_file(const char *source_file, const char *destination_file) {
    FILE *source, *destination;
    char buffer[1024];  // 一時バッファ
    size_t bytes_read;

    // 元ファイルを開く（読み込みモード）
    source = fopen(source_file, "r");
    if (source == NULL) {
        fprintf(stderr, "Failed to open source file: %s\n", source_file);
        exit(1);
    }

    // 出力ファイルを開く（書き込みモード）
    destination = fopen(destination_file, "w");
    if (destination == NULL) {
        fprintf(stderr, "Failed to open destination file: %s\n", destination_file);
        fclose(source);
        exit(1);
    }

    // ファイルを一行ずつ読み込みながらコピー
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), source)) > 0) {
        fwrite(buffer, 1, bytes_read, destination);
    }

    // ファイルを閉じる
    fclose(source);
    fclose(destination);
}


//角度を度からラジアンに変える関数
double degree_to_radian(double theta){
    return (PI/180)*theta;
}

int radian_to_degree(double theta){
    return (180/PI)*theta;
}

// section間でのパラメータの初期化
void reset_parameters(double *current_x,double *current_y,double *velocity_x,double *velocity_y,double *accerelation_x,double *accerelation_y,double *jerk_x,double *jerk_y){
    *current_x = 0.0;
    *current_y = 0.0;
    *velocity_x = 0.0;
    *velocity_y = 0.0;
    *accerelation_x = 0.0;
    *accerelation_y = 0.0;
    *jerk_x = 0.0;
    *jerk_y = 0.0;
}

//パラメータを持ち越す関数
void take_over_parameters(double *pre_velocity_x,double velocity_x,double *pre_velocity_y,double velocity_y,double *pre_acceleration_x,double acceleration_x,double *pre_acceleration_y,double acceleration_y){
    *pre_velocity_x = velocity_x;
    *pre_velocity_y = velocity_y;
    *pre_acceleration_x = acceleration_x;
    *pre_acceleration_y = acceleration_y;
}

//初期地点から目標地点の必要な角度の計算
double target_value_decision(double initial_point,double target_point){
    double target_value;
    target_value =  fabs(initial_point - target_point);
    return target_value;
}

//仮想的な角速度にかける定数の計算
double integral_constant(double sum,double target_value){
    double constant;
    constant = target_value / sum;
    return constant;
}

//実際の角速度の計算
double reality_theta_vel(double param_m[],double integral_constant,int step){
    double real_vel;
    real_vel = integral_constant * param_m[step];
    if(real_vel > 4.0){
        return 4.0;
    }
    return real_vel;
}

// ガウス基底関数
double radial_basis_function(double theta, double center,double width) {
    return exp(-pow(theta - center, 2) / (2 * pow(width, 2)));
}


//状態を基底関数に入れる関数
void compute_features(double theta_s,double theta_e,double width, double features_s[],double features_e[],double centers[]) {
    for (int i = 0; i < STATE; i++) {
        features_s[i] = radial_basis_function(theta_s, centers[i],width);
        features_e[i] = radial_basis_function(theta_e, centers[i],width);
    }
}

//報酬のリセット
void reset_reward(double reward[]){
    for(int i = 0;i<MAX_STEP;i ++){
        reward[i] = 0;
    }
}

// 状態価値関数の初期化の関数
void reset_critic_value(double critic_weights_s[],double critic_weights_e[]){
    for(int i = 0;i<STATE;i ++){
        critic_weights_s[i] = 0.0;
        critic_weights_e[i] = 0.0;
    }
}

// 逆運動学関数
void inverse_kinematics(double x, double y,int *theta1, int *theta2)
{
    double theta_shoulder,theta_elbow;
    // θ2を計算
    double cos_theta2 = (x * x  + y * y  - L1 * L1  - L2 * L2) / (2 * L1 * L2);

    // 値の範囲チェック
    if (cos_theta2 <  -1 || cos_theta2 > 1)
    {
        printf("位置が到達可能範囲外です。\n");
    }

    theta_elbow = 0; // 肘の角度 θ2
    

    // θ1を計算
    double sin_theta2 = sin(theta_elbow);
    double k1 = L1  + L2 * cos(theta_elbow);
    double k2 = L2 * sin(theta_elbow);

    *theta1 = radian_to_degree(atan2(y, x)  - atan2(k2, k1)); // 肩の角度 θ1
    *theta2 = radian_to_degree(theta_elbow);
}


//確率密度関数にしたがって、softmax法でランダムに行動を出力
double random_movements(double features_1[],double actor_weights[][STATE],double width,double r,double center[],double temperature,int episode,int step){
    double q_weight[ACTION]={0.0},features_2[ACTION]={0.0},sum[MAX_ACTION_NUM+1]={0.0},normalize[MAX_ACTION_NUM+1]={0.0},count=0.0,max=0.0,da=0.0,num=0.0,probability,min;

    for(int temp_1 =0;temp_1<ACTION;temp_1++){
        for(int temp_2 =0;temp_2<STATE;temp_2++){
            q_weight[temp_1] += actor_weights[temp_1][temp_2]*features_1[temp_2];
        }
    }

    for(int temp_1 = 0;temp_1 <= MAX_ACTION_NUM;temp_1++){
        for(int temp_2 =0 ; temp_2<ACTION;temp_2++){
            da = temp_1*D_ACTION;
            features_2[temp_2] = radial_basis_function(da,center[temp_2],width);//actionを基底関数の引数に
            features_2[temp_2] = features_2[temp_2]* q_weight[temp_2];
            sum[temp_1] +=  features_2[temp_2];
        }
        if(temp_1 == 0 || max < sum[temp_1]){
            max = sum[temp_1];
            // max_selection[episode] = da;
        }
    }

    for(int temp = 0;temp <= MAX_ACTION_NUM;temp++){
        sum[temp] =  exp((sum[temp]-max)/temperature);
        count += sum[temp];
    }

    for(int temp = 0;temp <= MAX_ACTION_NUM;temp++){
        normalize[temp] = sum[temp]/count;
        probability += normalize[temp];
    }

    for(int temp = 0;temp <= MAX_ACTION_NUM;temp++){
        num += normalize[temp];
        if (r<num){
            da = temp*D_ACTION;
            return da;
        }
    }
    da = 4.0;
    return da;
}

//確率密度関数にしたがって、softmax法でランダムに行動を出力
double alt_random_movements(double features_1[],double actor_weights[][STATE],double width,double r,double center[],double temperature,int episode,int step){
    double features_3[ACTION][STATE]={0.0},features_2[ACTION]={0.0},sum[MAX_ACTION_NUM+1]={0.0},normalize[MAX_ACTION_NUM+1]={0.0},count=0.0,max=0.0,da=0.0,num=0.0;
    for(int temp_1 = 0;temp_1 <= MAX_ACTION_NUM;temp_1++){
        da = temp_1*D_ACTION;
        for(int temp_2 =0 ; temp_2<ACTION;temp_2++){
            features_2[temp_2] = radial_basis_function(da,center[temp_2],width);
            for(int temp_3 =0 ; temp_3 < STATE;temp_3++){
                features_3[temp_2][temp_3] = actor_weights[temp_2][temp_3]*features_1[temp_3]*features_2[temp_2]; //features_1は計算済み
                sum[temp_1] +=  features_3[temp_2][temp_3];
            }
        }
        if(temp_1 == 0 || max < sum[temp_1]){
            max = sum[temp_1];
        }
    }

    for(int temp = 0;temp <= MAX_ACTION_NUM;temp++){
        sum[temp] =  exp((sum[temp]-max)/temperature);
        count += sum[temp];
    }

    for(int temp = 0;temp <= MAX_ACTION_NUM;temp++){
        normalize[temp] = sum[temp]/count;
    }

    for(int temp = 0;temp <= MAX_ACTION_NUM;temp++){
        num += normalize[temp];
        if (r<=num){
            da = temp*D_ACTION;
            return da;
        }
    }
    da = 4.0;
    return da;
}



// double ipsiron_greedy(double features_1[],double actor_weights[][STATE],double width,double r,double r_select,double condition,double center[]){
//     double action,q_weight[MAX_ACTION_NUM],da,features_2[MAX_ACTION_NUM],sum[MAX_ACTION_NUM],max;
//     if (condition > r){
//         action = r_select*ACTION_SELECT;
//         return action;
//     }
//     else{
//         for(int temp_1 =0;temp_1<ACTION;temp_1++){
//             for(int temp_2 =0;temp_2<STATE;temp_2++){
//                 q_weight[temp_1] += actor_weights[temp_1][temp_2]*features_1[temp_2];
//             }
//         }
//         for(int temp_1 = 0;temp_1 <= MAX_ACTION_NUM;temp_1++){
//             for(int temp_2 =0 ; temp_2<ACTION;temp_2++){
//                 da = temp_1*D_ACTION;
//                 features_2[temp_2] = radial_basis_function(da,center[temp_2],width);
//                 features_2[temp_2] = features_2[temp_2]* q_weight[temp_2];
//                 sum[temp_1] +=  features_2[temp_2];
//             }
//             if(temp_1 == 0 || max < sum[temp_1]){
//                 max = da;
//             }
//         }
//         return da;
//     }
// }

//肩関節の角度
double shoulder_movements(double theta,double action[],int step){
    theta += action[step];
   
    if(theta>MAX_THETA_SHOULDER_ARRAY){//135度
        return MAX_THETA_SHOULDER_ARRAY;
    } 
    else if(theta<MIN_THETA_SHOULDER_ARRAY){// 0度
        return MIN_THETA_SHOULDER_ARRAY;  
    }
    return theta;
}

//肘関節の角度
double elbow_movements(double theta,double action[],int step){
    theta += action[step];
   
    if(theta>MAX_THETA_ELBOW_ARRAY){//145度
        return MAX_THETA_ELBOW_ARRAY;
    } 
    else if(theta<MIN_THETA_ELBOW_ARRAY){// 0度
        return MIN_THETA_ELBOW_ARRAY;  
    }
    return theta;
}

// 手先の座標を計算する関数（2リンクアーム）
void calculate_end_effector(double theta1, double theta2, double *x, double *y) {

    double r_theta1,r_theta2,d_theta1,d_theta2;
    r_theta1 = degree_to_radian(theta1);
    r_theta2 = degree_to_radian(theta2);
    *x = L1 * cos(r_theta1)  + L2 * cos(r_theta1  + r_theta2);
    *y = L1 * sin(r_theta1)  + L2 * sin(r_theta1  + r_theta2);
}

//差分計算
double cal_dif(double x,double pre_x){
    return (x -pre_x)/DT;
}

//速度、速さなどを計算する関数
double calculate_difference(double x,double y,double pre_x,double pre_y,double *differential_x,double  *differential_y,double *sum){
    *differential_x = cal_dif(x,pre_x);
    *differential_y = cal_dif(y,pre_y);
    *sum = sqrt((*differential_x)*(*differential_x) +  (*differential_y)*(*differential_y));
    return *sum;
}

//報酬を計算
double calculate_reward(double distance,double velocity_sum,double count_jerk_sum,double *reward_d_sum,double *reward_j_sum,double *reward_v_sum,double *reward_sum,double dt,double alpha){
    double normalize_distance;
    normalize_distance = distance*100;//距離の報酬に使う単位を m から cm に

    *reward_d_sum = - normalize_distance*REWARD_DP;
    if(SWITCH_J_R){
        *reward_j_sum = - count_jerk_sum*COUNT_JERK_SUM_P;
    }
    else{
        *reward_j_sum = 0;
    }
    if(SWITCH_V_R){
        *reward_v_sum = - alpha*velocity_sum*VELOCITY_SUM_P;
    }
    else{
        *reward_v_sum = 0;
    }
    *reward_sum = REWARD_P + *reward_d_sum + *reward_j_sum + *reward_v_sum;
    // 報酬関数の条件
    if (TIME_PUNISH > dt ) {
        return 0;
    } 

    // else if(TIME_PUNISH > dt) {
    //     return *reward_v_sum;
    // }
    
    else {
        return *reward_sum;
    }
}



// // 報酬関数を計算する
// double calculate_reward_distance(double distance,double velocity_sum,double count_jerk_sum,double dt,double *reward_t_sum,double *reward_j_sum,double *reward_v_sum,double *reward_sum) {
//     double normalize_distance;
//     normalize_distance = distance*100;
//     *reward_t_sum = - TIME_PUNISH_P*pow(dt-TIME_PUNISH,2);
//     *reward_j_sum = - count_jerk_sum*COUNT_JERK_SUM_P;
//     *reward_v_sum = - velocity_sum * VELOCITY_SUM_P;
//     *reward_sum = REWARD_P + *reward_t_sum + *reward_j_sum + *reward_v_sum;

//     // 報酬関数の条件
//     if (sqrt(distance) > THRESHOLD ) {
//         return 0;
//     } 
    
//     else {
//         return *reward_sum;
//     }
// }

//TD誤差を計算
double calculate_td(int step,double reward[],double critic_weights_s[],double critic_weights_e[],double features_s[],double features_e[], double next_features_s[],double next_features_e[]) {
    double value = 0.0,next_value = 0.0;

    for(int temp=0;temp < STATE;temp++){
        // value += critic_weights_s[temp] * features_s[temp] + critic_weights_e[temp] * features_e[temp];
        value +=  critic_weights_s[temp] * features_s[temp];
        // next_value += critic_weights_s[temp] * next_features_s[temp] + critic_weights_e[temp] * next_features_e[temp];
        next_value += critic_weights_s[temp] * next_features_s[temp];
    }
    return reward[step]  + GAMMA * next_value  - value; 
}

//重みをupdate
void update_weights(double td_error,double critic_weights_s[],double critic_weights_e[],double actor_weights_s[][STATE],double actor_weights_e[][STATE],double features_s[],double features_e[],double action_s[],double action_e,double width,double center[],int step) {
    double gradient_s[ACTION],gradient_e[ACTION],c_s,c_e;
    for(int i = 0;i< STATE;i++){
        critic_weights_s[i] += ALPHA_CRITIC*td_error*features_s[i];
        // critic_weights_e[i] += ALPHA_CRITIC*td_error*features_e[i];
    }

    for(int temp_1 =0 ; temp_1< ACTION;temp_1++){
        for(int temp_2 =0; temp_2 < STATE;temp_2++){
            gradient_s[temp_1] = radial_basis_function(action_s[step],center[temp_1],width);
            // gradient_e[temp_1] = alt_radial_basis_function(action_e,center[temp_1],width);
            actor_weights_s[temp_1][temp_2] += ALPHA_ACTOR* td_error*features_s[temp_2]*gradient_s[temp_1];
            // actor_weights_e[temp_1][temp_2] += ALPHA_ACTOR* td_error*features_e[temp_2]*gradient_e[temp_1];
        }
    }
}




