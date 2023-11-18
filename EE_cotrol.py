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
                                   '0':Finger_Open（EEを開く状態） 
                    -send_data[4]: 入力終了判定の','

        receive:
                receive_data
                        send_data[0] = '1'を送ったときだけ，flagが返ってくる．
                        flag '0':トマト把持失敗，'1':トマト把持成功
        """

        def __init__(self):
            self.ser = serial.Serial('COM6', 115200, timeout=0.1)
            time.sleep(2.0)

        def send_Arduino(self, mode):

            # send_data = self.check_data(str(mode)+str(PWM))
            send_data = self.check_data(str(mode))
            
            self.ser.setDTR(False)
            self.ser.write(bytes(send_data, encoding='ascii'))
            self.ser.flush()
            self.ser.reset_input_buffer()  # 入力バッファをフラッシュ

            receive_data = self.serial_data()   
            return receive_data

        def check_data(self, send_data):
            # 語尾に','を追加する 
            if send_data[-1] != ',':
                send_data = send_data + ','
            return send_data

        def serial_data(self):
            line = self.ser.readline()
            try:
                line_disp = line.strip().decode('UTF-8')
            except:
                line_disp = line
            return line_disp
        
        def state_0(self):
            while True:
                flag = self.send_Arduino(0)
                if len(flag) > 1:
                    break
                
        def state_1(self):
            flag = self.send_Arduino(1)
            data = flag.split(',')
            if int(data[0]) > int(data[1]):
                flag = 1
            else:
                flag = 0
            return flag

        def state_2(self):
            while True:
                flag = self.send_Arduino(2)
                if flag == 'ok':
                    break
            return flag
 

'''debug'''
# EE = EE_Control()
# print(EE.state_0())
# print(EE.state_1())
# print(EE.state_2())

# while True:
#     flag = EE.state_2()
#     print(flag)

# print(EE.state_0())
# time.sleep(4)
# print(EE.state_1())
