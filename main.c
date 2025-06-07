#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "param.h"
#include "function.c"


int main(void) {
    //シミュレーション条件
    double dt;//時間の刻み幅
    int episode=0,step=0,best_episode;
    double goal_x=GOAL_X,goal_y=GOAL_Y;//目標地点
    double temperature=TEMP;//softmaxの温度パラメータ

    //手先の位置、速度
    double current_x,current_y,pre_current_x,pre_current_y;//手先の位置
    double velocity_x,velocity_y,pre_velocity_x = 0.0,pre_velocity_y = 0.0,velocity_sum = 0.0; //手先の速度とその総和
    double acceleration_x,acceleration_y,pre_acceleration_x = 0.0,pre_acceleration_y = 0.0,acceleration_sum = 0.0;//手先の加速度とその総和
    double jerk_x,jerk_y,pre_jerk_x,pre_jerk_y,jerk_sum,pre_jerk_sum,count_jerk_sum;//手先の躍度とその総和
    double temp_velocity_x,temp_velocity_y,temp_acceleration_x,temp_acceleration_y,temp_jerk_x,temp_jerk_y;
    double temp_velocity_sum,temp_acceleration_sum,temp_jerk_sum;//次の手先の速度を0と仮定したときに躍度の計算に使う

    //角度
    double theta_shoulder,theta_elbow,pre_theta_shoulder,pre_theta_elbow,best_theta[MAX_EPISODE];//関節角度
    double theta_elbow_vel = 0.0,theta_shoulder_vel= 0.0,pre_theta_elbow_vel= 0.0,pre_theta_shoulder_vel= 0.0,theta_vel_sum;//関節角速度
    double theta_elbow_ac= 0.0,theta_shoulder_ac= 0.0,pre_theta_elbow_ac = 0.0,pre_theta_shoulder_ac =0.0,theta_ac_sum;//関節角加速度
    double theta_elbow_je = 0.0, theta_shoulder_je =0.0,pre_theta_elbow_je = 0.0,pre_theta_shoulder_je =0.0,theta_je_sum;//関節角躍度
    double temp_theta_shoulder_vel,temp_theta_elbow_vel,temp_theta_elbow_ac,temp_theta_shoulder_ac,temp_theta_elbow_je,temp_theta_shoulder_je;
    double temp_theta_vel_sum,temp_theta_ac_sum,temp_theta_je_sum;//次の手先の角速度を0と仮定したときに角躍度の計算に使う
    
    //運動学
    double r_shoulder,r_elbow;//乱数の代入先
    double shoulder_m_param[MAX_STEP],elbow_m_param[MAX_STEP];//関節をどれだけ動かすか
    double sum_shoulder;//いままで動いた関節角度の総和
    double initial_shoulder = INITIAL_SHOULDER,initial_elbow = INITIAL_ELBOW;//最初の角度
    double target_shoulder = TARGET_SHOULDER,target_elbow = TARGET_ELBOW;//ゴールの角度
    double target_shoulder_value,target_elbow_value;//初期地点から目標地点の必要な角度
    double shoulder_constant,elbow_constant;//仮想的な角速度にかける定数
    double distance,prev_distance;//手先の距離
    
    //基底関数
    double centers_state[STATE] = {54,60,66,72,78,84,90,96}; //状態の基底関数の中心
    double centers_action[ACTION] = {-0.1,0,0.1,0.2,0.3,0.4,0.5};//行動の基底関数の中心
    double critic_weights_s[STATE]={0.0},critic_weights_e[STATE]={0.0};//状態の基底関数の重み
    double actor_weights_s[ACTION][STATE]={0.0},actor_weights_e[ACTION][STATE]={0.0};//状態と行動の基底関数の重み
    double width_state = WIDTH_STATE, width_action=WIDTH_ACTION;//状態、状態と行動のそれぞれの基底関数の幅
    double c_w_s,c_w_e,a_w_s[ACTION],a_w_e[ACTION];//状態、状態と行動の基底関数の重みをファイル出力する際に用いる
    double features_s[STATE]={0.0},features_e[STATE]={0.0},next_features_s[STATE]={0.0},next_features_e[STATE]={0.0};//基底関数に今の状態を入力する際に用いる

    //報酬
    double reward[MAX_STEP],best_reward;//報酬
    double reward_sum,reward_d_sum=0,reward_j_sum=0,reward_v_sum=0,reward_t_sum;//それぞれ総報酬、手先の位置の報酬、角躍度の報酬、手先の速さによる報酬、時間による報酬
    double td_error;//td誤差
    double alpha=1.0;

    //ファイル
    int episode_bookmark=0;
    char filename[1000];
    struct tm tm;
    time_t t=time(NULL);
    localtime_r(&t, &tm);
    const char *source_file_1 = "param.h",*source_file_2 = "main.c", *source_file_3 = "function.c";
    FILE *data,*data_1,*parameter,*data_episode,*data_value;

    srand(time(NULL)); 
    //ヘッダーファイルのコピー
    sprintf(filename,"%04d_%02d_%02d_%02d_%02d_%02d_parameter.h", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    copy_file(source_file_1,filename);

    //ヘッダーファイルのコピー
    sprintf(filename,"%04d_%02d_%02d_%02d_%02d_%02d_main.c", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    copy_file(source_file_2,filename);

    //ヘッダーファイルのコピー
    sprintf(filename,"%04d_%02d_%02d_%02d_%02d_%02d_function.c", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    copy_file(source_file_3,filename);

    //episodeごとの値を保存するためのファイル作成
    sprintf(filename,"%04d_%02d_%02d_%02d_%02d_%02d_data_episode.csv",tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    data_episode = fopen(filename, "w");
    fprintf(data_episode,"ep g_s c_j r_d r_j r_t r_s cr_v \n");

    //価値関数の値を保存するためのファイル作成
    sprintf(filename,"%04d_%02d_%02d_%02d_%02d_%02d_data_value.csv",tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    data_value = fopen(filename, "w");



    //初期地点から目標地点の必要な角度の計算
    target_shoulder_value = target_value_decision(initial_shoulder,target_shoulder);
    target_elbow_value = target_value_decision(initial_elbow,target_elbow);

    // // 学習のループ
    for(int episode = 0;episode<MAX_EPISODE; episode++){
        //最後のエピソードだけ温度パラメータを1.0に
        if(episode == MAX_EPISODE-1){
            temperature = 1.0;
        }

        //パラメータの初期化
        printf("episode:%d \n",episode);
        reset_parameters(&current_x,&current_y,&velocity_x,&velocity_y,&acceleration_x,&acceleration_y,&jerk_x,&jerk_y);
        reset_parameters(&pre_current_x,&pre_current_y,&pre_velocity_x,&pre_velocity_y,&pre_acceleration_x,&pre_acceleration_y,&pre_jerk_x,&pre_jerk_y);
        reset_parameters(&theta_shoulder,&theta_elbow,&theta_elbow_vel,&theta_shoulder_vel,&theta_elbow_ac,&theta_shoulder_ac,&theta_elbow_je,&theta_shoulder_je);
        reset_parameters(&pre_theta_shoulder,&pre_theta_elbow,&pre_theta_elbow_vel,&pre_theta_shoulder_vel,&pre_theta_elbow_ac,&pre_theta_shoulder_ac,&pre_theta_elbow_je,&pre_theta_shoulder_je);
        reset_reward(reward);
        count_jerk_sum = 0;
        sum_shoulder = 0;
        dt = 0.0;

        // 初期位置の目標地点までの距離の計算
        theta_shoulder = 60.0;//60度
        theta_elbow = 0.0;
        
        //手先の位置と距離、状態の計算
        calculate_end_effector(theta_shoulder, theta_elbow,&current_x,&current_y);
        prev_distance = pow(goal_x - current_x, 2) + pow(goal_y - current_y, 2);
        compute_features(theta_shoulder,theta_elbow,width_state, features_s, features_e, centers_state);

        if(episode != 0 && episode == 49999){
            episode_bookmark = 49999;
        }
        
        if(episode == episode_bookmark){
            //ファイル名を作成
            sprintf(filename,"%04d_%02d_%02d_%02d_%02d_%02d_data_%d.csv", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,episode_bookmark+1);
            //ファイルを開く
            data = fopen(filename, "w");
            fprintf(data,"dt c_x c_y t_s t_e ve ac je rd rj rt rs td\n");
            fprintf(data,"%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f \n",dt,current_x,current_y,theta_shoulder,theta_elbow,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0);
        }

        if(0 < episode && episode <= 200000  && episode % 20000 == 0){
            alpha -= 0.1; 
        }

        for (step = 0; step < MAX_STEP; step++) {
            
            dt += DT;
            //current_x,yの初期値をpre_currnt_x,yに代入
            pre_current_x = current_x;
            pre_current_y = current_y;
            pre_theta_shoulder = theta_shoulder;
            pre_theta_elbow = theta_elbow;

            // ランダムな行動選択
            r_shoulder = (double)rand()/(RAND_MAX);
            r_elbow = 0;
            // shoulder_m_param = ipsiron_greedy(features_s,actor_weights_s,width_action,r_shoulder,r_shoulder_select,condition,centers_action);
            shoulder_m_param[step] = alt_random_movements(features_s,actor_weights_s,width_action,r_shoulder,centers_action,temperature,episode,step);
            elbow_m_param[step] = 0;
            sum_shoulder += shoulder_m_param[step];
            // sum_elbow = elbow_m_param[step];

            // 現在の角度にΔθをかけて次の角度を更新
            theta_shoulder = shoulder_movements(theta_shoulder,shoulder_m_param,step);
            theta_elbow = elbow_movements(theta_elbow,elbow_m_param,step);
            compute_features(theta_shoulder,theta_elbow, width_state,next_features_s, next_features_e, centers_state);

            //角速度、角加速度、角躍度計算
            calculate_difference(theta_shoulder,theta_elbow,pre_theta_shoulder,pre_theta_elbow,&theta_shoulder_vel,&theta_elbow_vel,&theta_vel_sum);
            calculate_difference(theta_shoulder_vel,theta_elbow_vel,pre_theta_shoulder_vel,pre_theta_elbow_vel,&theta_shoulder_ac,&theta_elbow_ac,&theta_ac_sum);
            calculate_difference(theta_shoulder_ac,theta_elbow_ac,pre_theta_shoulder_ac,pre_theta_elbow_ac,&theta_shoulder_je,&theta_elbow_je,&theta_je_sum);
            count_jerk_sum +=  theta_je_sum*theta_je_sum*DT;//躍度の2乗に刻み幅をかける、後で報酬計算に用いる

            // 手先の座標を計算
            calculate_end_effector(theta_shoulder, theta_elbow,&current_x,&current_y);

            // 距離を計算
            distance = pow(goal_x - current_x, 2) + pow(goal_y - current_y, 2);

            //速度、加速度、躍度計算
            calculate_difference(current_x,current_y,pre_current_x,pre_current_y,&velocity_x,&velocity_y,&velocity_sum);
            calculate_difference(velocity_x,velocity_y,pre_velocity_x,pre_velocity_y,&acceleration_x,&acceleration_y,&acceleration_sum);
            calculate_difference(acceleration_x,acceleration_y,pre_acceleration_x,pre_acceleration_y,&jerk_x,&jerk_y,&jerk_sum);

            if(dt >= TIME_PUNISH){//次の手先の速度を0と仮定し、計算
                //手先の速度、加速度、躍度計算
                calculate_difference(current_x,current_y,current_x,current_y,&temp_velocity_x,&temp_velocity_y,&temp_velocity_sum);
                calculate_difference(temp_velocity_x,temp_velocity_y,velocity_x,velocity_y,&acceleration_x,&acceleration_y,&temp_acceleration_sum);
                calculate_difference(temp_acceleration_x,temp_acceleration_y,acceleration_x,acceleration_y,&temp_jerk_x,&temp_jerk_y,&temp_jerk_sum);

                //手先の角速度、角加速度、角躍度
                calculate_difference(theta_shoulder,theta_elbow,theta_shoulder,theta_elbow,&temp_theta_shoulder_vel,&temp_theta_elbow_vel,&temp_theta_vel_sum);
                calculate_difference(temp_theta_shoulder_vel,temp_theta_elbow_vel,theta_shoulder_vel,theta_elbow_vel,&temp_theta_shoulder_ac,&temp_theta_elbow_ac,&temp_theta_ac_sum);
                calculate_difference(temp_theta_shoulder_ac,temp_theta_elbow_ac,theta_shoulder_ac,theta_elbow_ac,&temp_theta_shoulder_je,&temp_theta_elbow_je,&temp_theta_je_sum);
                count_jerk_sum +=  temp_theta_je_sum*temp_theta_je_sum*DT;//躍度の2乗に刻み幅をかける、後で報酬計算に用いる
            }

            // 前回のパラメータを更新
            prev_distance = distance;
            take_over_parameters(&pre_velocity_x,velocity_x,&pre_velocity_y,velocity_y,&pre_acceleration_x,acceleration_x,&pre_acceleration_y,acceleration_y);
            take_over_parameters(&pre_theta_elbow_vel,theta_elbow_vel,&pre_theta_shoulder_vel,theta_shoulder_vel,&pre_theta_elbow_ac,theta_elbow_ac,&pre_theta_shoulder_ac,theta_shoulder_ac);

            //次の状態を基底関数に
            for (int i = 0; i < STATE; i++) {
                features_s[i] = next_features_s[i];
                features_e[i] = next_features_e[i];
            }

            if(dt >= TIME_PUNISH){
                break;
            }
        }

        //ここより下は積分値一定の機構
        dt = 0.0;
        count_jerk_sum = 0.0;
        shoulder_constant = integral_constant(sum_shoulder,target_shoulder_value);
        // elbow_constant = integral_constant(sum_elbow,target_elbow_value);
        // shoulder_constant = 1.0;

        //パラメータの初期化
        reset_parameters(&current_x,&current_y,&velocity_x,&velocity_y,&acceleration_x,&acceleration_y,&jerk_x,&jerk_y);
        reset_parameters(&pre_current_x,&pre_current_y,&pre_velocity_x,&pre_velocity_y,&pre_acceleration_x,&pre_acceleration_y,&pre_jerk_x,&pre_jerk_y);
        reset_parameters(&theta_shoulder,&theta_elbow,&theta_elbow_vel,&theta_shoulder_vel,&theta_elbow_ac,&theta_shoulder_ac,&theta_elbow_je,&theta_shoulder_je);
        reset_parameters(&pre_theta_shoulder,&pre_theta_elbow,&pre_theta_elbow_vel,&pre_theta_shoulder_vel,&pre_theta_elbow_ac,&pre_theta_shoulder_ac,&pre_theta_elbow_je,&pre_theta_shoulder_je);
        reset_reward(reward);

        // 初期位置の目標地点までの距離の計算
        theta_shoulder = 60.0;//60度
        theta_elbow = 0.0;
        
        calculate_end_effector(theta_shoulder, theta_elbow,&current_x,&current_y);
        prev_distance = pow(goal_x - current_x, 2) + pow(goal_y - current_y, 2);
        compute_features(theta_shoulder,theta_elbow,width_state, features_s, features_e, centers_state);

        for (step = 0; step < MAX_STEP; step++) {
            dt += DT;
            //current_x,yの初期値をpre_currnt_x,yに代入
            pre_current_x = current_x;
            pre_current_y = current_y;
            pre_theta_shoulder = theta_shoulder;
            pre_theta_elbow = theta_elbow;
            // ランダムな行動選択（-1から1の範囲でランダムに選択）
            shoulder_m_param[step] = reality_theta_vel(shoulder_m_param,shoulder_constant,step);
            elbow_m_param[step] = 0;
            // 現在の角度にΔθをかけて次の角度を更新
            theta_shoulder = shoulder_movements(theta_shoulder,shoulder_m_param,step);
            theta_elbow = elbow_movements(theta_elbow,elbow_m_param,step);
            compute_features(theta_shoulder,theta_elbow, width_state,next_features_s, next_features_e, centers_state);
           
            //角速度、角加速度、角躍度
            calculate_difference(theta_shoulder,theta_elbow,pre_theta_shoulder,pre_theta_elbow,&theta_shoulder_vel,&theta_elbow_vel,&theta_vel_sum);
            calculate_difference(theta_shoulder_vel,theta_elbow_vel,pre_theta_shoulder_vel,pre_theta_elbow_vel,&theta_shoulder_ac,&theta_elbow_ac,&theta_ac_sum);
            calculate_difference(theta_shoulder_ac,theta_elbow_ac,pre_theta_shoulder_ac,pre_theta_elbow_ac,&theta_shoulder_je,&theta_elbow_je,&theta_je_sum);
            count_jerk_sum +=  theta_je_sum*theta_je_sum*DT;

            // 手先の座標を計算
            calculate_end_effector(theta_shoulder,theta_elbow,&current_x,&current_y);

            // 距離を計算
            distance = pow(goal_x - current_x, 2) + pow(goal_y - current_y, 2);

            //手先の速度、加速度、躍度計算
            calculate_difference(current_x,current_y,pre_current_x,pre_current_y,&velocity_x,&velocity_y,&velocity_sum);
            calculate_difference(velocity_x,velocity_y,pre_velocity_x,pre_velocity_y,&acceleration_x,&acceleration_y,&acceleration_sum);
            calculate_difference(acceleration_x,acceleration_y,pre_acceleration_x,pre_acceleration_y,&jerk_x,&jerk_y,&jerk_sum);

            if(dt >= TIME_PUNISH){
                //手先の速度、加速度、躍度計算
                calculate_difference(current_x,current_y,current_x,current_y,&temp_velocity_x,&temp_velocity_y,&temp_velocity_sum);
                calculate_difference(temp_velocity_x,temp_velocity_y,velocity_x,velocity_y,&acceleration_x,&acceleration_y,&temp_acceleration_sum);
                calculate_difference(temp_acceleration_x,temp_acceleration_y,acceleration_x,acceleration_y,&temp_jerk_x,&temp_jerk_y,&temp_jerk_sum);

                //手先の角速度、角加速度、角躍度
                calculate_difference(theta_shoulder,theta_elbow,theta_shoulder,theta_elbow,&temp_theta_shoulder_vel,&temp_theta_elbow_vel,&temp_theta_vel_sum);
                calculate_difference(temp_theta_shoulder_vel,temp_theta_elbow_vel,theta_shoulder_vel,theta_elbow_vel,&temp_theta_shoulder_ac,&temp_theta_elbow_ac,&temp_theta_ac_sum);
                calculate_difference(temp_theta_shoulder_ac,temp_theta_elbow_ac,theta_shoulder_ac,theta_elbow_ac,&temp_theta_shoulder_je,&temp_theta_elbow_je,&temp_theta_je_sum);
                count_jerk_sum +=  temp_theta_je_sum*temp_theta_je_sum*DT;
            }

            // 報酬を計算
            reward[step] = calculate_reward(sqrt(distance),velocity_sum*velocity_sum,count_jerk_sum,&reward_d_sum,&reward_j_sum,&reward_v_sum,&reward_sum,dt,alpha);
           
            // TD誤差を計算
            td_error = calculate_td(step,reward,critic_weights_s,critic_weights_e,features_s,features_e,next_features_s,next_features_e);

            //重みの更新 
            update_weights(td_error,critic_weights_s,critic_weights_e,actor_weights_s,actor_weights_e,features_s,features_e,shoulder_m_param,0.0,width_action,centers_action,step);

            if(episode == episode_bookmark){
                fprintf(data,"%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f \n",dt,current_x,current_y,theta_shoulder,theta_elbow,velocity_sum,acceleration_sum,jerk_sum,reward_d_sum,reward_j_sum,reward_t_sum,reward_sum,td_error,0.0,0.0,theta_elbow_vel,theta_shoulder_vel,theta_elbow_ac,theta_shoulder_ac,velocity_x,velocity_y,acceleration_x,acceleration_y,jerk_x,jerk_y,theta_elbow_je,theta_shoulder_je);
            }

            if(dt >= TIME_PUNISH){
                if(episode == episode_bookmark){
                    fprintf(data,"%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f \n",dt+DT,current_x,current_y,theta_shoulder,theta_elbow,temp_velocity_sum,temp_acceleration_sum,temp_jerk_sum,reward_d_sum,reward_j_sum,reward_t_sum,reward_sum,td_error,0.0,0.0,temp_theta_elbow_vel,temp_theta_shoulder_vel,temp_theta_elbow_ac,temp_theta_shoulder_ac,temp_velocity_x,temp_velocity_y,temp_acceleration_x,temp_acceleration_y,temp_jerk_x,temp_jerk_y,temp_theta_elbow_je,temp_theta_shoulder_je);
                }
                break;
            }

            // 前回のパラメータを更新
            prev_distance = distance;
            take_over_parameters(&pre_velocity_x,velocity_x,&pre_velocity_y,velocity_y,&pre_acceleration_x,acceleration_x,&pre_acceleration_y,acceleration_y);
            take_over_parameters(&pre_theta_elbow_vel,theta_elbow_vel,&pre_theta_shoulder_vel,theta_shoulder_vel,&pre_theta_elbow_ac,theta_elbow_ac,&pre_theta_shoulder_ac,theta_shoulder_ac);

            for (int i = 0; i < STATE; i++) {
                features_s[i] = next_features_s[i];
                features_e[i] = next_features_e[i];
            }
        }

        if (episode == episode_bookmark && episode != 0){
            episode_bookmark += 50000; 
        }
        if(episode % 100 ==0){
            fprintf(data_episode,"%d %d %f %f %f %f %f %f ",episode,step,count_jerk_sum,reward_d_sum,reward_j_sum,reward_v_sum,reward_sum,reward_t_sum);
            fprintf(data_episode,"\n");
        }

        //重みをfprintf
        if(episode % 10000 == 0 || episode == MAX_EPISODE -1 ){
            fprintf(data_value,"%d ",episode);
            for(int temp_1=40;temp_1<=110;temp_1+=5){
                compute_features(temp_1,0, width_state,features_s, features_e, centers_state);//角度シータは固定
                for(int temp_2=0;temp_2<STATE;temp_2++){
                    c_w_s += critic_weights_s[temp_2] * features_s[temp_2];
                    // c_w_e += critic_weights_e[temp_2] * features_e[temp_2];
                }
                fprintf(data_value,"%f ",c_w_s);
                c_w_s = 0.0;
                c_w_e = 0.0;
            }

            for(int temp_3=0;temp_3<ACTION;temp_3++){
                for(int temp_2=0;temp_2<STATE;temp_2++){
                    fprintf(data_value,"%f ",actor_weights_s[temp_3][temp_2]);
                }
            }                
            fprintf(data_value,"\n");
        }
    }
    fclose(data);
    fclose(data_episode);
    fclose(data_value);
    return 0;
}
   
   