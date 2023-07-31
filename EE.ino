/*
input:
        data
                -data[0]: mode
                          0:Wait(Finger_Open)
                          1:Suction_On（吸い上げる）
                          2:Finger_Close（EEを閉じる）
                          3:Finger_Open（EEを開く）
                          4:Suction_Off（吸い上げ止め）
                -data[1]: EE_PWM 100の位
                -data[2]: EE_PWM 10の位
                -data[3]: EE_PWM 1の位
                -data[4]: 入力終了判定の','
        
output:

*/



// Parameter 
#define CutMtrPWM 5                 // cutter動作用モータのPWM
#define mtrDrvPinA 3                // mtrDrvPinA_pin                        
#define mtrDrvPinB 4                // mtrDrvPinB_pin
#define TSW 6                       // Close_cutter_Top_Switch
#define BSW 7                       // Open_cutter_Bottom_Switch
#define OPTICAL 1                   // optical_sensor
#define EDF 9                       // Electric_Ducted_Fan_PWM_pin

int sensor_threshold = 0;           // 光センサのしきい値の初期化
int InitEDF_PWM = 100;              // EDF起動時のPWM //不感帯領域pwm
int Suc_EDF_PWM;                    // EDF吸引時のPWM(serialで受信)

//receive_data
int count = 0;               
char data[32];

// Function ///////////////////////////////////////////////////////////

int Conversion(int num1, int num2, int num3){
  // 百の位
  int Num1 = num1 - '0';      // char型をint型に変換
  if(Num1 > 0 && Num1 <= 2){
    Num1 = Num1*100;          
  }else if(Num1 > 2){
    Num1 = 200;
  }else{
    Num1 = 0;
  }

  // 十の位
  int Num2 = num2 - '0';
  if(Num1 >= 200){
    if(Num2 > 5 && Num2 < 10){
      Num2 = 50;
    }else{
      Num2 = Num2*10;
    }
  }else if(Num2 > 0 && Num2 < 10){
    Num2 = Num2*10;
  }else{
    Num2 = 0;
  }
  
  // 一の位
  int Num3 = num3 - '0';
  if(Num1 >= 200 && Num2 >= 50){
    if(Num3 > 5 && Num3 < 10){
      Num3 = 5;
    }else{
      Num3 = Num3*1;
    }
  }else if(Num3 > 0 && Num3 < 10){
    Num3 = Num3*1;
  }else{
    Num3 = 0;
  }
  
  int Num = Num1 + Num2 + Num3; // 合計
  return Num;
}

void opt_sensor_calib(){
  int n = 10;
  /*
    1.平均値求める
    　光センサからn個のデータ取得⇨n個の光センサデータの合計出す⇨光センサの平均値求める
    2.閾値の決定
      偏差⇨分散⇨標準偏差の順で求める
      標準偏差の値がおかしいときがあるのでその時は定数でしきい値決めてる
  */

  //1step
  double optical_sensor_sum = 0; 
  int optical_sensor_cali[10];        // 光センサのしきい値のキャリブレーション用
  for(int cnt = 0 ; cnt < n ; cnt++){
    optical_sensor_cali[cnt] = analogRead(OPTICAL); 
    optical_sensor_sum +=  analogRead(OPTICAL);
  }
  double optical_sensor_mean = optical_sensor_sum / (double)(n); 
  
  //2step
  double deviation = 0 ; 
  for(int cnt = 0 ; cnt < n ; cnt++){
    deviation += (double)optical_sensor_cali[cnt] - (double)optical_sensor_mean; 
  }
  double bias = deviation / (double)(n);
  double std_deviation = sqrt(bias); 
  if(std_deviation > 100){ 
    sensor_threshold = 900;
  }
  else{ 
    sensor_threshold = (int)optical_sensor_mean + 90; //threshold = mean + 90
  }  
}

//Motor 
void stopMotor(){
    analogWrite(CutMtrPWM, 0);
    digitalWrite(mtrDrvPinA, HIGH);
    digitalWrite(mtrDrvPinB, HIGH); 
}
void upMotor(){
    analogWrite(EDF, Suc_EDF_PWM);
    digitalWrite(mtrDrvPinA, LOW);            
    digitalWrite(mtrDrvPinB, HIGH);
}
void downMotor(){
    analogWrite(EDF, InitEDF_PWM);
    digitalWrite(mtrDrvPinA, HIGH);            
    digitalWrite(mtrDrvPinB, LOW);
}


void setup() {                      
  pinMode(mtrDrvPinA, OUTPUT);      
  pinMode(mtrDrvPinB, OUTPUT);      
  pinMode(CutMtrPWM, OUTPUT);       
  pinMode(TSW, INPUT_PULLUP);       
  pinMode(BSW, INPUT_PULLUP);       
  pinMode(OPTICAL, INPUT);          
  pinMode(EDF, OUTPUT);             
  //for debug
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  //初期設定 
  stopMotor();
  analogWrite(EDF, InitEDF_PWM);
  Serial.begin(115200);
}


void loop(){
  if(Serial.available() > 0){
    data[count] = Serial.read();
    if(data[count] == ','){
      uint8_t mode = data[0] - '0'; //先頭の値がmode番号
      Suc_EDF_PWM = Conversion(data[1], data[2], data[3]); //残りのデータがPWM値を示す
      switch(mode){
        //Wait
        case 0:  
          downMotor();
          while(digitalRead(BSW)){
            analogWrite(CutMtrPWM, 150);
          }
          stopMotor();
          Serial.println('ok');
          break;
        
        //Suction_On
        case 1:  
          stopMotor();
          analogWrite(EDF, Suc_EDF_PWM);
          int cnt = 0;
          uint8_t flag = 1;
          int optical_sensor = 0; // 光センサの値:Analog
          while(optical_sensor > sensor_threshold){
            delay(1000);
            optical_sensor = analogRead(OPTICAL);
            if (cnt>3){
                flag = 0;
                break;
            }
            cnt ++;
          }
          Serial.println(flag);
          break;

        case 2:  //Finger_Close
          upMotor();
          //continue close until the switch is pressed
          while(digitalRead(TSW)){         
            analogWrite(CutMtrPWM, 250);
          }
          stopMotor();
          //Guide the tomatoes to the drop-off point
          Serial.println('ok');
          break;

        case 3:  //Finger_Open
            downMotor();  
            //continue open until the switch is pressed
            while(digitalRead(BSW)){         
                analogWrite(CutMtrPWM, 120);
            }
            stopMotor();   
            //Noise reduction of sensor values
            int n = 10;
            double optical_sensor_sum = 0; 
            for(int cnt = 0 ; cnt < n ; cnt++){
                optical_sensor_sum +=  analogRead(OPTICAL);
            }
            double optical_sensor_mean = optical_sensor_sum / (double)(n); 
            if(optical_sensor_mean > sensor_threshold){
                uint8_t flag = 1;  
            }else{
                uint8_t flag = 0;
            }
            Serial.println(flag);
            break;
      }
    }else{
      count ++;
    }
  }
}
