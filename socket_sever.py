import socket
import pyautogui as pag
import time
import threading

s = socket.socket()         

s.bind(('0.0.0.0', 8090 ))
s.listen(0)                 
currentMouseX, currentMouseY = pag.position()

posi = ["uu","ur" ,"dr" , "dl","dd","ul","lu","ld"]
zone = ["u","l","d"]
press_value = [0]*8
press_zone = [0]*3
press_anv = 0
direction = 'n'
INTERVAL = 10

def socket_sever():
    while True:
        client, addr = s.accept()
        while True:
            content = client.recv(32)
            content = str(content,encoding="utf-8")
            if len(content) ==0:
                break
            else:
                #print(content)
                sub = content.split(" ")
                try:
                    dir_idx = sub.index("DIR")
                except:
                    pass
                else:
                    direction = sub[dir_idx+1][0]

                try:
                    pre_idx = sub.index("PRE")
                except:
                    pass
                else:
                    idx = pre_idx+1
                    while idx<len(sub)-2 :
                        try:
                            press_value[int(sub[idx])] = float(sub[idx+1])
                            idx = idx+2
                        except:
                            print("error in str to float/int")  
                    press_zone[0] = (press_value[0]+press_value[1]+press_value[5])/3
                    press_zone[1] = (press_value[6]+press_value[7])/2
                    press_zone[2] = (press_value[2]+press_value[3]+press_value[4])/3 
                    global press_anv
                    press_anv = sum(press_value)/8

        client.close()

thread_ss =threading.Thread(target=socket_sever)
thread_ss.setDaemon(True)
thread_ss.start()  

isDown = False
anv_old = 0;

while True:
    # ----- pressure
    #press = zip(posi,press_value)
    #print(press_value)  
    print(press_zone,press_anv)
    if(anv_old-press_anv>10):
        if(not isDown):
            print('h')
            pag.press(' ')
            isDown = True
            anv_old = press_anv
    else:
        isDown = False
    
    anv_old = press_anv
    # --------- direction
    #print("DIR", direction)
    # if(direction == 'l'):
    #     pag.moveRel(-INTERVAL,None)
    # elif(direction == 'r'):
    #     pag.moveRel(INTERVAL,None)
    # elif(direction == 'u'):
    #     pag.moveRel(None,INTERVAL)
    # elif(direction == 'd'):
    #     pag.moveRel(None,-INTERVAL)
    # time.sleep(0.05)
    # ---------
    time.sleep(0.01)
