import serial
import time

class EE_Control:
        """
        send:
                send_data 
                    -send_data[0]: 動作モード
                                   '0':Wait （待機状態）
                                   '1':Suction_On(吸引） 
                                   '2':Finger_Close(EE閉じる)
                                   '3':Finger_Open(EEを開く)

                    -send_data[1]: EE_PWM 100の位
                    -send_data[2]: EE_PWM 10の位
                    -send_data[3]: EE_PWM 1の位
                    -send_data[4]: 入力終了判定の','
        """

        def __init__(self):
            self.ser = serial.Serial('COM6', 115200)
            time.sleep(2)
        
        def harvest_tomato(self):
            self.send2Arduino(0, 50) #待機
            result = self.send2Arduino(1, 50) #吸引
            if result : 
                self.send2Arduino(2,50) #もぎ取り
                result = self.send2Arduino(3,50) #収穫確認
                if result : 
                    return True
                else :
                    return False
            else : #トマトが吸引できなかった場合
                return False

        def send2Arduino(self, mode, PWM):
            PWM = format(PWM, '03')
            send_data = str(mode)+str(PWM)
            if send_data[-1] != ',':
                send_data = send_data + ','
            self.ser.write(send_data.encode(encoding='utf-8'))
            try:
                self.ser.timeout = 3 #(s)
                line = self.ser.readline()
                receive_data = line.strip().decode('UTF-8')    
                if receive_data !="": #データが帰ってこなかった場合
                    return False
                elif receive_data == "0": 
                    return False
                else :
                    return True
            except serial.serialutil.SerialTimeoutException:
                print("time out erro")
                return False


def debug():
    EE = EE_Control()
    EE.harvest_tomato()

def main():
    '''デバック'''
    #debug()
if __name__ == '__main__':
    main()
