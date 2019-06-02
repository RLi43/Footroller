import socket
import pyautogui as pag
import time
import threading

addr = "0.0.0.0"
port = 13579
class PressureAn():
    def __init__(self,_left):
        self.isleft = _left
        if not _left:
            self.posi_name = {"uu":0,"ur":1,"dr":2, "dl":3,"dd":4,"ul":5,"ru":6,"rd":7}#['dr','dl','ul','dd','uu','rd','ur','ru']#
        else:
            self.posi_name = {"ur":0,"dr":1,"dl":2, "uu":3,"dd":4,"ul":5,"ld":6,"lu":7}
        self.values = [4095]*8
        self.zone_name = ["u","l","d"]
        self.zone_value = [0]*3
        self.anv_value = 0;
    def data_update(self):
        self.zone_value[0] = (self.values[self.posi_name["uu"]]+self.values[self.posi_name["ur"]]+self.values[self.posi_name["ul"]])/3
        if self.isleft:
            self.zone_value[1] = (self.values[self.posi_name["lu"]]+self.values[self.posi_name["ld"]])/2
        else:
            self.zone_value[1] = (self.values[self.posi_name["ru"]]+self.values[self.posi_name["rd"]])/2            
        self.zone_value[2] = (self.values[self.posi_name["dl"]]+self.values[self.posi_name["dr"]]+self.values[self.posi_name["dd"]])/3 
        self.anv_value = sum(self.values)/8

class DirectionAn():
    def __init__(self):
        self.acc = [0]*3
        self.gyc = [0]*3
        self.angle = [0]*3

        self.direction = 'n'
        self._interval = 10
    def mouseMove(self):
        print("DIR", self.direction)
        if(self.direction == 'l'):
            pag.moveRel(-self._interval,None)
        elif(self.direction == 'r'):
            pag.moveRel(self._interval,None)
        elif(self.direction == 'u'):
            pag.moveRel(None,self._interval)
        elif(self.direction == 'd'):
            pag.moveRel(None,-self._interval)


class FootAn():
    def __init__(self, _left = True):
        self.pre = PressureAn(_left)
        self.dir = DirectionAn()
        self.left = _left
    
    def data_solve(self,str):
        sub = str.split(" ")
        idx = 0
        while idx<len(sub):
            if(sub[idx]=="ANG"):
                idx = idx+1
                i = 0            
                while True:
                    try: 
                        self.dir.angle[i] = float(sub[idx])
                        idx = idx + 1
                        i = i+1
                    except: break
            elif(sub[idx]=="PRE"): 
                idx = idx + 1
                while True:
                    try: 
                        self.pre.values[int(sub[idx])] = float(sub[idx+1])
                        idx = idx+2
                    except: break
                self.pre.data_update()
            else: 
                #忽略什么都不是
                idx = idx + 1
                

class FeetAn():
    def __init__(self):
        self.footL = FootAn(_left=True)
        self.footR = FootAn(_left=False)

        self.socket = socket.socket()
        self.socket.bind((addr,port))
        self.socket.listen(4)
        self.bg = threading.Thread(target = self.socket_sever,daemon=True)
        self.bg.start()

    def socket_sever(self):
        while True:
            client, addr = self.socket.accept()  
            t=threading.Thread(target=self.new_socket_solver,args=(client,addr))  
            t.start()
        print("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
        self.socket.close()
        self.bg.start()
    
    def new_socket_solver(self,sk,addr):
        while True:
            content = sk.recv(200)
            content = str(content,encoding="utf-8")
            if len(content) ==0:
                break
            else:
                #print("Get!:",content)
                self.getData(content)
        sk.close()

    def getData(self,str):
        idx1 = str.find('L')
        if idx1==-1:
            idx2 = str.find('R')
            if idx2==-1:
                return
            else:
                str = str[idx2:]
                left = False
        else:
            str = str[idx1:]
            left = True
        if(left):
            self.footL.data_solve(str)
        else:
            self.footR.data_solve(str)        
  
    def run(self):
        # pres_oldR = self.footR.pre.values.copy()
        # pres_oldL = self.footL.pre.values.copy()
        i = 0
        jump=False
        down = False
        while True:
            i = i+1
            # change_valuesL = 0
            # change_valuesR = 0
            # for j in range(len(self.footR.pre.values)):
            #     change_valuesR = change_valuesR + (self.footR.pre.values[j]-pres_oldR[j])**2
            #     change_valuesL = change_valuesL + (self.footL.pre.values[j]-pres_oldL[j])**2
            # #print(change_values)
            # if change_valuesL>1e5 and change_valuesR>1e5:
            if self.footR.pre.zone_value[0] - self.footR.pre.anv_value > 100:
                if not jump:
                    print(i,"go!   Down")
            #         print(i,change_valuesL,change_valuesR,"                                         Go!")
                    jump =True
                    pag.keyDown(" ")
            else: 
                if jump:
                    pag.keyUp(" ")
                    print(i,"go!            UP")
                jump = False
            
            #print(self.footL.pre.zone_value[0],self.footL.pre.anv_value)
            if self.footL.pre.zone_value[0] - self.footL.pre.anv_value < -100:
                if not down:
                    print(i,"                                                                           key down down!")
                    down =True
                    pag.keyDown("down")
            else: 
                if down:
                    print(i,"                                                                           key UP down!")
                    pag.keyUp("down")
                down = False
            
            # d_old = d_new    
           # print(i,': L ',self.footL.pre.anv_value,self.footL.pre.zone_value,self.footL.dir.angle,
            #            ' R ',self.footR.pre.anv_value,self.footR.pre.zone_value,self.footR.dir.angle)  
            pres_oldR = self.footR.pre.values.copy()      
            pres_oldL = self.footL.pre.values.copy()     
            time.sleep(0.01)
 
feet = FeetAn()
feet.run()