#include <MsTimer2.h>
// https://github.com/NicoHood/PinChangeInterrupt
#include "PinChangeInterrupt.h"

#define PWM 5                       // cutter動作用モータのPWM
#define A 3                         // Aピン                         
#define B 4                         // Bピン
#define TSW 6                       // Close_cutter_SW
#define BSW 7                       // Open_cutter_SW
#define OPTICAL 1                   // 光センサ
#define EDF 9

int CloseTime = 5000; //fingerがしまる上限時間

int optical_sensor = 0;             // 光センサの値:Analog
int optical_sensor_cali[10];        // 光センサのしきい値のキャリブレーション用
int sensor_threshold = 900;           // 光センサのしきい値の初期化
int top_sw = 1;                     // TSW(Top_SW)
int bottom_sw = 1;                  // BSW(Bottom_SW)

int InitEDF_PWM = 100;              // EDF起動時のPWM
int EDF_PWM = 220;                        // EDF吸引時のPWM(serialで受信)

uint8_t receive_data;               // Pythonから受信したデータ
uint8_t mode;                       // EEのモード番号

uint8_t send_data;                  // Simulinkに送信するデータ
uint8_t flag = 0;                   // EEの状態
uint8_t sensor = 0;                 // 光センサの値

int count = 0;
char data[32];


void setup() {
  // put your setup code here, to run once:
  pinMode(A, OUTPUT);               // モータドライバ Aピン
  pinMode(B, OUTPUT);               // モータドライバ Bピン
  pinMode(PWM, OUTPUT);             // cutter動作用モータのPWM出力
  pinMode(TSW, INPUT_PULLUP);       // Close_cutter
  pinMode(BSW, INPUT_PULLUP);       // Open_cutter
  pinMode(OPTICAL, INPUT);          // 光センサ
  pinMode(EDF, OUTPUT);             // EDF用PWM出力

  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  // モータ停止
  digitalWrite(A, HIGH);
  digitalWrite(B, HIGH);
  analogWrite(PWM, 0);
  analogWrite(EDF, InitEDF_PWM);
  Serial.begin(115200);

  // 光センサから10個のデータ取得
  int cnt = 0;  // カウント変数
  for(cnt = 0 ; cnt < 10 ; cnt++){
    optical_sensor_cali[cnt] = analogRead(OPTICAL); //光センサの値取得
  }
  // 光センサ10のデータの平均値
  int optical_sensor_sum = 0; //10個の光センサデータの合計変数の初期化
  double optical_sensor_mean = 0; //光センサの平均値変数の初期化
  cnt = 0;
  for(cnt = 0 ; cnt < 10 ; cnt++){
    optical_sensor_sum += optical_sensor_cali[cnt];  //10個の光センサデータの合計
  }
  optical_sensor_mean = (double)optical_sensor_sum / (double)(cnt + 1); // 光センサの平均値
  double t = 0; //偏差変数の初期化
  double optical_sensor_std = 0; //標準偏差変数の初期化
  cnt = 0;
  for(cnt = 0 ; cnt < 10 ; cnt++){
    t += (double)optical_sensor_cali[cnt] - (double)optical_sensor_mean; //偏差
  }
  optical_sensor_std = sqrt(t / (double)(cnt + 1)); //標準偏差
  if(optical_sensor_std > 100){ //分散が大きすぎる場合
    sensor_threshold = 900;
  }
  else{ //分散が許容範囲である場合
    sensor_threshold = (int)optical_sensor_mean + 120; //平均値+120をしきい値とする
  }
}

void finger_stop(){
  digitalWrite(A, HIGH);
  digitalWrite(B, HIGH);
}

void finger_open(){
  digitalWrite(A, HIGH);  
  digitalWrite(B, LOW);
}

void finger_close(){
  digitalWrite(A, LOW);
  digitalWrite(B, HIGH);
}

void loop() {
  if(Serial.available() > 0){
    String input = Serial.readStringUntil(',');
    mode = input.toInt();
    switch(mode){
      case 0:
        flag = 0;
        analogWrite(EDF, InitEDF_PWM);
        finger_open();
        while(digitalRead(BSW)){
          analogWrite(PWM, 210);
        }
        finger_stop();
        analogWrite(PWM, 0);

        optical_sensor = analogRead(OPTICAL);
        if(optical_sensor > sensor_threshold){
          flag = 1;
        }
        Serial.print(optical_sensor);Serial.print(",");Serial.println(sensor_threshold);
        // Serial.println(flag);
      
        break;
      
      case 1:
        flag = 0;
        analogWrite(EDF, EDF_PWM);
        finger_stop();
        analogWrite(PWM, 0);

        optical_sensor = analogRead(OPTICAL);
        if(optical_sensor > sensor_threshold){
          flag = 1;
        }
        Serial.print(optical_sensor);Serial.print(",");Serial.println(sensor_threshold);
        // Serial.println(flag);
        break;
      
      case 2:
        flag = 0;
        analogWrite(EDF, EDF_PWM);
        finger_close();
        unsigned long startTime = millis();
        Serial.println("ok"); //ここでいったんPC側に文字を送信しないと、millis()- startTime>=6000が無視される。okをPCが受け取ったらは、PC側からは指令が来なくなる
        while(digitalRead(TSW)!=0){  //digitalRead(TSW)
          if(millis()- startTime>=6000){
            break;
          }
          analogWrite(PWM, 220);
        }
        analogWrite(PWM, 0);
        finger_stop();
        optical_sensor = analogRead(OPTICAL);
        Serial.print(optical_sensor);Serial.print(",");Serial.println(sensor_threshold);
        break;

      default:
        break;
    }
  }
}
