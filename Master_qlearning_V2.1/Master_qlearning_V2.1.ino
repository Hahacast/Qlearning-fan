
//version2.1
//1.將range由20改成一個define range變數 23
//2.將間隔改成0.15(interval全域變數
//3.若該reward_table某一個state的action皆為負值，則可以更新(避免死循環)
 
//version 2.0將table的struct改成陣列
#include <SoftwareSerial.h>   // 引用程式庫
#include <math.h>         //包含数学库
#include <EEPROM.h>
#include "DHT.h"
#define range 21
#define interval 0.15
DHT dht(10, DHT11);
SoftwareSerial BT(8, 9); // 接收腳, 傳送腳
const int button1Pin = 2;//(綠)
const int button2Pin = 3;//(黃)
const int button3Pin = 4;//(紅)
const int button4Pin = 5;//(紅)

int flag = 0;//  flag = 0 代表建立reword table , flag = 1 代表qlearning中 , flag = 2 代表智慧電扇
float goal_temp = 33.56; //使用者舒適溫度

float qtable[range][4] ;
//float reward_table[range][4]; // 獎賞table
//float STS_table[range][4]; //  the table that describe A State use B action to what state
float RH ;

//
float reward_table[range][4] = {
  {32.06, 0.00, 0.00, 0.00},
  {32.21, 0.00, 0.00, 0.00},
  {32.21, 0.00, 0.00, 0.00},
  {32.21, 0.00, 0.00, 0.00},
  {32.21, 0.00, 0.00, 0.00},
  {32.21, 0.00, 0.00, 0.00},
  {32.21, 0.00, 0.00, 0.00},
  {33.11, 80.00, 90.00, 80.00},
  {33.26, 60.00, 90.00, -70.00},
  {33.41, 80.00, -20.00, -80.00},
  {33.56, 70.00, 90.00, 80.00},
  {33.71, -30.00, 100.00, 80.00},
  {33.86, 80.00, 90.00, 80.00},
  {34.01, 70.00, 90.00, 80.00},
  {34.16, 80.00, 80.00, 90.00},
  {34.31, 60.00, 0.00, 0.00},
  {34.46, 0.00, 0.00, 0.00},
  {34.61, 0.00, 0.00, 0.00},
  {34.76, 0.00, 0.00, 0.00},
  {34.91, 0.00, 0.00, 0.00},
  {35.06, 0.00, 0.00, 0.00}
    };

float STS_table[range][4] = {
  {32.06, 0.00, 0.00, 0.00},
  {32.21, 0.00, 0.00, 0.00},
  {32.36, 0.00, 0.00, 0.00},
  {32.51, 0.00, 0.00, 0.00},
  {32.66, 0.00, 0.00, 0.00},
  {32.81, 0.00, 0.00, 0.00},
  {32.96, 0.00, 0.00, 0.00},
  {33.11, 33.86, 33.41, 33.26},
  {33.26, 34.16, 33.41, 33.11},
  {33.41, 33.86, 33.26, 33.26},
  {33.56, 34.01, 33.86, 33.41},//goal
  {33.71, 34.01, 33.56, 33.26},
  {33.86, 33.86, 33.71, 33.26},
  {34.01, 34.01, 33.41, 33.26},
  {34.16, 33.86, 33.86, 33.41},
  {34.31, 34.16, 0.00, 0.00},
  {34.46, 0.00, 0.00, 0.00},
  {34.61, 0.00, 0.00, 0.00},
  {34.76, 0.00, 0.00, 0.00},
  {34.91, 0.00, 0.00, 0.00},
  {35.06, 0.00, 0.00, 0.00} };


void setup() {
  // put your setup code here, to run once:
  flag = EEPROM.read(150);//開頭先將flag寫入
  Serial.begin(9600);
  BT.begin(9600); 
  dht.begin();//啟動DHT
  randomSeed(analogRead(A1));
  pinMode(button1Pin, INPUT);  
  pinMode(button2Pin, INPUT);  
  pinMode(button3Pin, INPUT);  
  pinMode(button4Pin, INPUT);  
  
  if( flag != 1 && flag != 255 ){
    for(int i=0;i<range;i++)
      for(int j = 1;j < 4;j++)
        qtable[i][j] = EEPROM.read(3*i+(j-1));
  }
  
  float set_state_temp = interval*((range-1)/2)*(-1) ;
  for( int i = 0 ; i < range ; i++){ 
    qtable[i][0] = goal_temp + set_state_temp ;
    reward_table[i][0] = goal_temp + set_state_temp ;
    STS_table[i][0] = goal_temp + set_state_temp ;
    set_state_temp += interval;   
  }
}

void loop() {
  
//  RH = dht.readHumidity();
//  set_comfortable_temp();
  qlearning();
  Serial.println("qtable = ");
  for(int i = 0 ; i < 21 ;i++ ){
    Serial.print("{ ");
    Serial.print(qtable[i][0]);
    Serial.print(", ");
    for(int j = 1 ; j <= 3 ; j++ ){
      if ( j == 3 ){
        Serial.print(qtable[i][j]);
        Serial.print(" },");        
      }
      else{
        Serial.print(qtable[i][j]);
        Serial.print(", ");        
      }

    }
    Serial.println("");
  }



//  set_comfortable_temp();
//  setup_rewardtable();
//  Serial.println("reward_table做完了");
//  Serial.println("reward_table = ");
//  for(int i = 0 ; i < range ;i++ ){
//  Serial.print(reward_table[i][0]);
//  Serial.print(", ");
//    for(int j = 1 ; j <= 3 ; j++ ){
//      Serial.print(reward_table[i][j]);
//      Serial.print(", ");
//    }
//    Serial.println("");
//  }
//  Serial.println("STS = ");
//  for(int i = 0 ; i < range ;i++ ){
//    Serial.print(STS_table[i][0]);
//    Serial.print(", ");
//    for(int j = 1 ; j <= 3 ; j++ ){
//      Serial.print(STS_table[i][j]);
//      Serial.print(", ");
//    }
//    Serial.println("");
//  }

  
//  if( flag == 0 || flag == 255 ){
//    RH = dht.readHumidity();
//    set_comfortable_temp();
//    setup_rewardtable();
//    //做完後寫入eeprom
//    qlearning();
//    for(int i=0;i<range;i++)
//      for(int j=0;j<3;j++)
//        EEPROM.write(3*i+j,qtable[i][j+1]); //EEPROM.write(address,value) 其中 address範圍0-255, 每個address寫入極限100000次     
//  }
//  else if ( flag == 1 ){
//    AI_FAN();    
//  }
//  else{
//    Serial.println("flag狀態不明");    
//  }
  delay(10000000);
   
}

void setup_rewardtable(){ //建立rewardtable之主程式
  int timecount = 0; // 依次timecount++代表吹10秒
  int state_index; // 某溫度(state)下在table的哪個一index，如reward_table[10] = 28.4，則若此時某state溫度為28.4則該state對應之state_index為10
  BT.print(1);
  delay(15000);
  float old_state = Temperature(1);
  while( timecount++ < 90 ) { //即做15 分鐘
    Serial.print("old_state =  " );
    Serial.println( old_state );
    int action = chooseAction_reward( old_state ); //選動作
    Serial.print("action = " );
    Serial.println( action );
    BT.print(action); //傳送給從端要求他吹action , 注意傳送的可能為int型態轉成char型態，還未測試
    delay(15000);//吹15秒
    float new_state = Temperature(action);
    Serial.print("new_state = ");
    Serial.println( new_state );
    done_reward( old_state , new_state, action ); // 知道old_state吹action10秒後得到new_state，並將資訊填入rewardtable和 STStable
    old_state = new_state ;
    Serial.print("timecount =  " );
    Serial.println( timecount );
    Serial.println(" ");
  }
}

float Temperature(int action ){
  float total = 0;
  float a_temp = 0;
  float V = 0;
  //平均20次
  for (int i= 0 ; i < 20 ; i++ ){
    float temp = dht.readTemperature();
    total += temp;
    delay(50); 
  }
  a_temp = total/20;
  switch(action){ //-1關 0弱 1中 2強
    case 0:
      V = 0;
      break;
    case 1:
      V = 4.0;
      break;
    case 2:
      V = 4.4;
      break;
    case 3:
      V = 5;
      break;
  }
  float e = RH/100*6.105*exp(17.27*a_temp/(237.7+a_temp));
  float AT = 1.07*a_temp+0.2*e-0.65*V-2.7 ;

  for( int index = 0 ; index < range ; index++){
    if ( AT >= qtable[index][0]-interval/2 && AT < qtable[index][0]+interval/2 ){
      AT = qtable[index][0];
    }
  }
  return AT;
}

int chooseAction_reward( float old_state ){ //選擇哪一個action
  // algorithm 若先選action reward為0的，若皆不為0隨機選強中弱
  int action = -1; 
  int state_index = FindIndex( old_state );
  if(state_index == -1)//新增的 低於table，太冷 
    return -1;
  else if(state_index == -2)
    return 2;
  for( int i = 1 ; i <= 3 ; i++ ){
    if( reward_table[ state_index ][i] == 0){
      return i;
    }   
  }
  return random(1,4);
}

void done_reward( float old_state , float new_state , int action_index  ) {

  int reward = 0;
  int state_index = FindIndex(old_state);
  int new_state_index = FindIndex( new_state) ;
  if( state_index == -1 ) //old_state超出適當範圍，不宜寫入reward_table
  {
    STS_table[state_index][action_index]=qtable[0][0];
    reward_table[state_index][action_index] = -1;
    return ;
   }
   else if ( state_index == -2 ){
    STS_table[state_index][action_index]=qtable[range-1][0];
    reward_table[state_index][action_index] = -1;
    return;
   }
   
  if( reward_table[state_index][action_index] == 0 || (reward_table[state_index][1]< 0 && reward_table[state_index][2] <0
                                                      &&reward_table[state_index][3] < 0) )
  {
    STS_table[state_index][action_index] = new_state; //存入delta_table
    //想吹冷但越吹越熱
    if( state_index > (range-1)/2 && new_state_index > state_index )
      reward_table[state_index][action_index] = ( 100-abs_2((range-1)/2-new_state_index)*10 )-100;
    //想吹熱但越吹越冷
    else if ( state_index < (range-1)/2 && new_state_index < state_index )
      reward_table[state_index][action_index] = ( 100-abs_2((range-1)/2-new_state_index)*10 )-100;
    else
      reward_table[state_index][action_index] = ( 100-abs_2((range-1)/2-new_state_index)*10 );
  }
  else
    return ;
  Serial.println("reward_table = ");
  for(int i = 0 ; i < range ;i++ ){
    if( i == (range-1)/2 ){
      Serial.print("goal     ");
    }
    else{
      Serial.print(reward_table[i][0]);
      Serial.print("    ");
    }
    for(int j = 1 ; j <= 3 ; j++ ){
      Serial.print(reward_table[i][j]);
      Serial.print("  ");
    }
    Serial.println("");
  }
  Serial.println("STS = ");
  for(int i = 0 ; i < range ;i++ ){
    Serial.print(STS_table[i][0]);
    Serial.print(", ");
    for(int j = 1 ; j <= 3 ; j++ ){
      Serial.print(STS_table[i][j]);
      Serial.print(", ");
    }
    Serial.println("");
  }
}
void qlearning(){ //訓練1000次
  //開頭隨機從range個state中選其中一個
  float alpha = 0.3 , Gamma = 0.9;
  float old_state  = qtable[ (range-1)/2][0] ;
  int oldstate_index = FindIndex(old_state);
  
  float new_state = 0 ;
  int newstate_index = 0;
  
  int q_count = 0 ; //進行幾次qlearing更新
  while( q_count++ < 10000 ){
    int action_index =  chooseAction_q(old_state ); 
    new_state = STS_table[ oldstate_index ][ action_index ];
    newstate_index = FindIndex( new_state );  
//    Serial.print("old_state = ");
//    Serial.println(old_state);
//    Serial.print("action = ");
//    Serial.println(action_index);
//    Serial.print("new_state = ");
//    Serial.println(new_state);
    
    float extrome=random(1000)/1000;//新增
    if(extrome>=0.9)//新增greedy
      new_state=Max( oldstate_index);
    qtable[ oldstate_index ][ action_index ] = ( 1 - alpha )*qtable[ oldstate_index ][action_index] 
                                                      + alpha*( reward_table[oldstate_index][action_index]
                                                      + Gamma*Max( newstate_index ) ); 
//    for(int i = 0 ; i < range;i++ ){
//      if( i == (range-1)/2 ){
//        Serial.print("goal     ");
//     }
//     else{
//       Serial.print(qtable[i][0]);
//       Serial.print("    ");
//      }
//      for(int j = 1 ; j <= 3 ; j++ ){
//       Serial.print(qtable[i][j]);
//       Serial.print("  ");
//     }
//     Serial.println("");
//    }
//    Serial.println("");  
    old_state = new_state;
    oldstate_index = newstate_index; 
  }
  //將沒三個動作都沒值得state，依照離goal的距離給予 強 or 中 or弱 7
  for( int i = 0 ; i < range ; i++ ){
    if ( i < (range-1)/4 && qtable[i][1]+qtable[i][2]+qtable[i][3] == 0 ){
      qtable[i][1] = 7;
    }
    if ( i >= (range-1)/4 &&  i< (range-1)*3/4 && qtable[i][1]+qtable[i][2]+qtable[i][3] == 0 ){
      qtable[i][2] = 7;
    }
    if (  i >= (range-1)*3/4 && qtable[i][1]+qtable[i][2]+qtable[i][3] == 0 ){
      qtable[i][3] = 7;
    } 
  }
  flag = 1;//新增
  EEPROM.write(150,1) ;//version1.3 第五點
}

int chooseAction_q ( float old_state ){
  int positive_count = 0;
  int positive_value[3] = {-1};
  int state_index = FindIndex (old_state) ;
  if( state_index == -1)//低於table，太冷
    return 1;
  if( state_index == -2)
    return 3;
  int action = 0;
  do{
    action = random(1,4);
//    Serial.println(action);
  }  while( STS_table[state_index][action] == 0 );
    
  return action;
}

void AI_FAN(){
  int ai_action;
  while( digitalRead(button4Pin) == LOW ) {
    RH = dht.readHumidity();
    float old_temp = Temperature(ai_action);
    int state_index=FindIndex(old_temp);
    if(state_index == -1)
      BT.print(1);
    else if(state_index == -2)
      BT.print(3);
    else{
      ai_action = AI_Max(state_index );
      BT.print(ai_action+1);
    }
    delay(10000);
  }
  flag = 0;
  EEPROM.write(150,0) ;
}

int FindIndex ( float s){ //此輸入一個state(溫度)給此function，則此function會回傳該溫度是在table的哪一個index
  if( s < qtable[0][0]-interval/2 ){
    return -1;
    Serial.println("溫度小於qlearning範圍");
  }
  if( s > qtable[range-1][0]+interval/2 ){ //version1.3 第六點
    return -2;
    Serial.println("溫度大於qlearning範圍");
  }
  //s為適當溫度
  for( int index = 0 ; index < range ; index++){
    if ( s >= qtable[index][0]-interval/2 && s < qtable[index][0]+interval/2 ){
//      Serial.print("index = ");
//      Serial.println(index);
      return index;
    }
  }
}

float Max( int newstate_index ){//找尋該某state下的最大q值
  int max_index = 1;
  for( int action_index = 1 ; action_index <= 3 ; action_index++ ){
    if ( qtable[newstate_index][action_index] > qtable[newstate_index][max_index] )
      max_index = action_index;
  }
  return qtable[newstate_index][max_index];
}
float AI_Max( int newstate_index ){//找尋該某state下的最大q值
  int max_index = 1;
  for( int action_index = 1 ; action_index <= 3 ; action_index++ ){
    if ( qtable[newstate_index][action_index] > qtable[newstate_index][max_index] )
      max_index = action_index;
  }
  
  return max_index ;
}


float  abs_2 ( float value ) {
  if ( value >= 0 ){
    return value;
  }
  else{
    return value*-1;
  }
}
void set_comfortable_temp(){ //button1是減弱 (綠) //button2是增強(黃) //button3(紅)是確認
  int fan_speed = 1;
  Serial.print("目前風速: ");
  Serial.println(fan_speed);
  int down = 0;
  int up = 0;
  while ( digitalRead(button3Pin) != HIGH ){
    down = digitalRead(button1Pin);
    up = digitalRead(button2Pin);
    if( down == 1 && fan_speed-1 >= 0 ){
      fan_speed -= 1;
      Serial.print("目前風速: ");
      Serial.println(fan_speed);
      BT.print(fan_speed);
    }
    else if ( up == 1 && fan_speed+1 <= 3 ){
      fan_speed += 1;
      Serial.print("目前風速: ");
      Serial.println(fan_speed);
      BT.print(fan_speed);      
    }
    delay(1000);    
  }
  goal_temp = Temperature(fan_speed );
  Serial.println("設定好了，goal temp = ");
  Serial.println(goal_temp);   
  //設定sts 和 reward_table的state值
  float set_state_temp = interval*((range-1)/2)*(-1) ;
  for( int i = 0 ; i < range ; i++){ 
    qtable[i][0] = goal_temp + set_state_temp ;
    reward_table[i][0] = goal_temp + set_state_temp ;
    STS_table[i][0] = goal_temp + set_state_temp ;
    set_state_temp += interval;   
  }
}
