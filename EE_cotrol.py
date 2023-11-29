import serial
import time


class Suction_Unit():
    def __init__(self):
        self.ser = serial.Serial('/dev/ttyUSB-arduino-end-effector', 115200)
        time.sleep(2)
    
    def harvest(self):
        self.state_0()
        time.sleep(5)
        self.state_1()
        time.sleep(5)
        self.state_2()
        time.sleep(5)
        self.state_0()
        time.sleep(5)
        
  
   
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
            if flag :
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

su = Suction_Unit()
su.harvest()
