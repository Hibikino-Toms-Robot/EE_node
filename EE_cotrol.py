import serial
import time

# 自分がアーム座標系からみて特定の位置に移動するためには、1つ前の位置（アーム座標系）と、これから動くアームの位置（アーム座標系）が必要
class EE_Control:

        """
        send:
                send_data 
                    -send_data[0]: '0':Wait （待機状態）
                                　 '1':Suction_On（吸い上げる状態） 
                                   '2':Finger_Close（EEを閉じる状態）
                                   '3':Finger_Open（EEを開く状態） 
                                   '4':Suction_Off（吸い上げ状態）
                                   '5':Hold（For Test用）
                    -send_data[1]: EE_PWM 100の位
                    -send_data[2]: EE_PWM 10の位
                    -send_data[3]: EE_PWM 1の位
                    -send_data[4]: 入力終了判定の','

        receive:
                receive_data

                            Waitのとき
                                0:初期化完了

                            Suction_Onのとき
                                1:光センサが未反応
                                5:光センサが反応（中にトマトが入っている）

                            Finger_Closeのとき
                                2:上SW反応（EEが閉じた状態）

                            Finger_Openのとき
                                3:下SW反応（EEが開いた状態）
                                6:光センサ反応（EE内にトマトが存在）
                                7:光センサ（吸引終了、トマト落下確認）

                            Suction_Offのとき
                                4:吸引動作終了
        """

        def __init__(self):
            self.ser = serial.Serial('COM6', 115200)
        
        def send_Arduino(self, mode, PWM):
            PWM = format(PWM, '03')
            send_data = self.check_data(str(mode)+str(PWM))
            # print(send_data)
            time.sleep(2)
            self.ser.write(send_data.encode(encoding='utf-8'))
            receive_data = self.serial_data()
            return (receive_data)

        def check_data(self, send_data):
            # 語尾に','を追加する 
            if send_data[-1] != ',':
                send_data = send_data + ','
            return send_data


        def serial_data(self):
            line = self.ser.readline()
            line_disp = line.strip().decode('UTF-8')
            return line_disp
 

'''debug'''
EE = EE_Control()
flag = EE.send_Arduino(0, 110)
print(flag)