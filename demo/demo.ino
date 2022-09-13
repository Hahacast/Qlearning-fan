
#include <SoftwareSerial.h>   // 引用程式庫
#include <math.h>         //包含数学库
#include <Timer.h>
#include "Wire.h"
#include "LiquidCrystal_I2C.h"
#include "DHT.h"
#define range 21
#define interval 0.15

LiquidCrystal_I2C lcd(0x27,2,1,0,4,5,6,7,3,POSITIVE);
DHT dht(10, DHT11);
SoftwareSerial BT(8, 9); // 接收腳, 傳送腳


const int button1Pin = 2;//(綠)
const int button2Pin = 3;//(黃)
const int button3Pin = 4;//(黑)
const int LEDRpin = A3;
const int LEDGpin = A2;
const int LEDBpin = A1;
Timer t ; 

//int flag = 0;//  flag = 0 代表建立reword table , flag = 1 代表qlearning中 , flag = 2 代表智慧電扇
//float goal_temp = 33.56; //使用者舒適溫度
float temp = 0;
int action = 0;
int state_index = 0;
float start_temp = 34.01;
float qtable[range][4] = {
{ 32.06, 7.00, 0.00, 0.00 },
{ 32.21, 7.00, 0.00, 0.00 },
{ 32.36, 7.00, 0.00, 0.00 },
{ 32.51, 7.00, 0.00, 0.00 },
{ 32.66, 7.00, 0.00, 0.00 },
{ 32.81, 0.00, 7.00, 0.00 },
{ 32.96, 0.00, 7.00, 0.00 },
{ 33.11, 917.97, 916.15, 904.52 },
{ 33.26, 886.17, 916.17, 756.16 },
{ 33.41, 917.97, 804.54, 744.54 },
{ 33.56, 894.28, 927.35, 905.97 },
{ 33.71, 794.48, 934.57, 904.54 },
{ 33.86, 917.98, 931.09, 904.54 },
{ 34.01, 894.45, 916.10, 904.52 },
{ 34.16, 917.98, 917.97, 916.13 },
{ 34.31, 0.00, 0.00, 7.00 },
{ 34.46, 0.00, 0.00, 7.00 },
{ 34.61, 0.00, 0.00, 7.00 },
{ 34.76, 0.00, 0.00, 7.00 },
{ 34.91, 0.00, 0.00, 7.00 },
{ 35.06, 0.00, 0.00, 7.00 }
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
  Serial.begin(9600);
  BT.begin(9600); 
  lcd.begin(16,2);
  dht.begin();//啟動DHT
  randomSeed(analogRead(A1));
  pinMode(button1Pin, INPUT);  
  pinMode(button2Pin, INPUT);  
  pinMode(button3Pin, INPUT);
  pinMode(LEDRpin, OUTPUT);
  pinMode(LEDGpin, OUTPUT);
  pinMode(LEDBpin, OUTPUT);
  digitalWrite(LEDRpin, LOW);
  digitalWrite(LEDGpin, LOW);
  digitalWrite(LEDBpin, LOW);
  t.every(6000, update_state); 
  BT.print(0);
}

void loop() {
//  set_comfortable_temp();
  hand_fan();
  AI_FAN(); 
}


void hand_fan(){
  temp = start_temp;
  action = 1;
  digitalWrite(LEDRpin, LOW);
  digitalWrite(LEDGpin, LOW);
  digitalWrite(LEDBpin, LOW);
  Serial.println("進入手動控制");
  lcd.setCursor(0,0);
  lcd.setCursor(0,1);
  lcd.clear();
  lcd.print("button control");
  delay(2000);
  BT.print(action);
  lcd.clear();
  lcd.print("power=");
  lcd.print(action);
  lcd.setCursor(0,1);
  lcd.print("temp=");
  lcd.print(temp); 
  three_led();

  int state_index = FindIndex(temp);
  int down = 0;
  int up = 0;
  while ( digitalRead(button3Pin) == LOW  ){
    t.update();
    if( digitalRead(button1Pin) == HIGH ){
      if ( action - 1 > 0 )
        action -= 1;
      Serial.print("目前風速 = ");
      Serial.println(action);
      lcd.setCursor(0,0);
      lcd.print("power=");
      lcd.print(action);
      BT.print(action);
    }
    if( digitalRead(button2Pin) == HIGH ){
      if ( action + 1 <= 3 )
        action += 1;
      Serial.print("目前風速 = ");
      Serial.println(action);
      lcd.setCursor(0,0);
      lcd.print("power=");
      lcd.print(action);
      BT.print(action);      
    }
    delay(1000);
  }
  delay(2000);
}

void AI_FAN(){
  temp = start_temp;
  state_index = FindIndex(temp);
  Serial.println("AI_fan");
  digitalWrite(LEDRpin, LOW);
  digitalWrite(LEDGpin, LOW);
  digitalWrite(LEDBpin, LOW);
  action = AI_Max(state_index );
  lcd.setCursor(0,0);
  lcd.clear();
  lcd.print("AI_Fan");
  delay(3000);
  lcd.setCursor(0,0);
  lcd.print("power=");
  lcd.print(action);
  BT.print(action);
  lcd.setCursor(0,1);
  lcd.print("temp=");
  lcd.print(temp);
  three_led();
  delay(4000);
  
  while( digitalRead(button3Pin) == LOW ) {
    delay(1000);
    Serial.print("temp = ");
    Serial.println(temp); 
    state_index = FindIndex(temp);
    if(state_index == -1)
      BT.print(1);
    else if(state_index == -2)
      BT.print(3);
    else{
      action = AI_Max(state_index );
      Serial.print("action = ");
      Serial.println( action );
      lcd.setCursor(0,0);
      lcd.print("power=");
      lcd.print(action);
      BT.print(action);
    }
    delay(5000);
    temp = STS_table[state_index][action];
    Serial.print("new_temp = ");
    Serial.println(temp);
    Serial.println("");
    lcd.setCursor(0,1);
    lcd.print("temp=");
    lcd.print(temp);
    three_led();

  }
  Serial.println("AI_Fan結束");
  lcd.setCursor(0,1);
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
  Serial.print("s = ");
  Serial.println(s);
  for( int index = 0 ; index < range ; index++){
    if ( s >= qtable[index][0]-interval/2 && s < qtable[index][0]+interval/2 ){
//      Serial.print("index = ");
//      Serial.println(index);
      return index;
    }
  }
}

float AI_Max( int newstate_index ){//找尋該某state下的最大q值
  int max_index = 1;
  for( int action_index = 1 ; action_index <= 3 ; action_index++ ){
    if ( qtable[newstate_index][action_index] > qtable[newstate_index][max_index] )
      max_index = action_index;
  }
  return max_index ;
}

void update_state(){
  float old_temp = temp;
  state_index = FindIndex(old_temp);
  Serial.print("action = ");
  Serial.println(action);
  temp = STS_table[state_index][action];
  Serial.print("temp = ");
  Serial.println(temp);
  lcd.setCursor(0,1);
  lcd.print("temp=");
  lcd.print(temp);
  three_led();


  
}
void three_led(){
  if( temp > 33.56 ){
    digitalWrite(LEDRpin, HIGH);
    digitalWrite(LEDGpin, LOW);
    digitalWrite(LEDBpin, LOW);    
  }
  else if (  temp == 33.56 ){
    digitalWrite(LEDRpin, LOW );
    digitalWrite(LEDGpin, HIGH);
    digitalWrite(LEDBpin, LOW);
  }
  else{
    digitalWrite(LEDRpin, LOW );
    digitalWrite(LEDGpin, LOW);
    digitalWrite(LEDBpin, HIGH);    
  }
}
