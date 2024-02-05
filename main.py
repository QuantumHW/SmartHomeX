'''
Description: 
Author: Huang Wen
Date: 2023-03-04 09:29:14
LastEditTime: 2023-03-08 14:27:25
LastEditors: Huang Wen
'''

import os
import socket
import time
import sys
from vosk import Model, KaldiRecognizer
import pyaudio
import wave
from threading import Thread
from PyQt5.QtWidgets import QApplication, QMainWindow
from PyQt5.QtWidgets import *
from PyQt5 import QtCore, QtGui, QtWidgets
from Ui_main import Ui_MainWindow
from facenet import MaskedFaceRecog
from argparse import ArgumentParser
import random
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from PyQt5.QtGui import *

voice_flag=True
camera_flag=False
fan_flag=False
light_flag=False

env_infor=None  

rate=16000
# voice_model = Model(model_path=".\\voice_model\\vosk-model-cn-0.22")
voice_model = Model(model_path=".\\voice_model\\vosk-model-small-cn-0.22")
# voice_model = Model(model_path=".\\voice_model\\vosk-model-cn-kaldi-multicn-0.15")
rec = KaldiRecognizer(voice_model, rate)
rec.SetWords(True)
rec.SetPartialWords(True)
_translate = QtCore.QCoreApplication.translate



def active_security():
    instruct="XXXBUZZ_ONXXX"
    env_infor.send(QtCore.QByteArray(instruct.encode()))

def close_alarm():
    instruct="XXXBUZZ_OFFXXX"
    env_infor.send(QtCore.QByteArray(instruct.encode()))

def start_recognition():
    mf.start_recog=True
    time.sleep(1)

    dir=os.listdir('./images/origin')
    ui.label_name.setText(_translate("MainWindow", "姓名："+str(mf.infor['name'])))
    ui.label_staus.setText(_translate("MainWindow", "身份："+str(mf.infor['status'])))
    now_time=time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(time.time()))
    ui.label_time.setText(_translate("MainWindow", "时间："+str(now_time)))
    if str(mf.infor['name'])+"_1.png" in dir:
        imgLabel = QLabel()
        imgLabel.resize(200, 300)
        ui.label_face_res.setPixmap(QtGui.QPixmap('./images/origin/'+str(mf.infor['name'])+'_1.png').scaled(imgLabel.width(), imgLabel.height()))
    else:
        ui.label_face_res.setText(_translate("MainWindow", "未知信息"))

    mf.start_recog=False
        

def start_sound_record():
    ui.hold_to_talk.setText(_translate("MainWindow", "识别中..."))
    t_sound_record = Thread(target=get_audio)
    t_sound_record.start()
    time.sleep(1)
  
def process_audio():
    ui.hold_to_talk.setText(_translate("MainWindow", "按住说话"))   
    t_process_audio=Thread(target=thread_process_audio)
    t_process_audio.start()
    time.sleep(1)    
    
def thread_process_audio():
    global voice_flag,camera_flag,light_flag,fan_flag                                                                               
    # print('处理音频')
    voice_flag=False
    time.sleep(3)
    wf = wave.open('./voice.wav', "rb")
    while True:
        data = wf.readframes(4000)
        if len(data) == 0:
            break
        rec.AcceptWaveform(data) 
    res_text=eval(rec.FinalResult())
    res_text=res_text['text'].replace(' ','')
    print(res_text)
    if '开摄像头' in res_text:
        camera_flag=False
        camera()
    elif '关摄像头' in res_text or '关闭摄像头' in res_text:
        camera_flag=True
        camera()
    elif '开灯' in res_text:
        light_flag=False
        light()
    elif '关灯' in res_text or '关闭灯' in res_text:
        light_flag=True
        light()
    elif '开风扇' in res_text:
        fan_flag=False
        fan()
    elif '关风扇' in res_text or '关闭风扇' in res_text:
        fan_flag=True
        fan()

def get_audio():
    global voice_flag
    # print("开始录音")
    CHUNK = 256
    FORMAT = pyaudio.paInt16
    CHANNELS = 2                # 声道数
    RATE = 8000                # 采样率
    WAVE_OUTPUT_FILENAME = "./voice.wav"
    p = pyaudio.PyAudio()
    stream = p.open(format=FORMAT,
                    channels=CHANNELS,
                    rate=RATE,
                    input=True,
                    frames_per_buffer=CHUNK,
                    input_device_index=1
                    )
    frames = []
    while True:
        # print(voice_flag)
        if voice_flag==False:break   
        data = stream.read(CHUNK)
        frames.append(data)
    stream.stop_stream()
    stream.close()
    p.terminate()
    wf = wave.open(WAVE_OUTPUT_FILENAME, 'wb')
    wf.setnchannels(CHANNELS)
    wf.setsampwidth(p.get_sample_size(FORMAT))
    wf.setframerate(RATE)
    wf.writeframes(b''.join(frames))
    wf.close()
    voice_flag=True


def light():
    global light_flag
    if light_flag==False:
        light_flag=True
        ui.open_light.setText(_translate("MainWindow", "关灯"))
        instruct="XXXLIGHT_ONXXX"
        env_infor.send(QtCore.QByteArray(instruct.encode()))
    else:
        light_flag=False
        ui.open_light.setText(_translate("MainWindow", "开灯"))
        instruct="XXXLIGHT_OFFXXX"
        
        env_infor.send(QtCore.QByteArray(instruct.encode()))

def fan():
    global fan_flag
    global env_infor
    if fan_flag==False:
        fan_flag=True
        ui.open_fan.setText(_translate("MainWindow", "关闭风扇"))
        instruct="xxxFAN_ONxxx"
        env_infor.send(QtCore.QByteArray(instruct.encode()))
    else:
        fan_flag=False
        ui.open_fan.setText(_translate("MainWindow", "打开风扇"))
        instruct="xxxFAN_OFFxxx"
        env_infor.send(QtCore.QByteArray(instruct.encode()))
    

def camera():
    global camera_flag
    if camera_flag==False:
        camera_flag=True
        ui.open_camera.setText(_translate("MainWindow", "关闭摄像头"))
        mf.show=True
    else:
        camera_flag=False
        mf.show=False
        mf.start_cam = False
        mf.camera.close()
        ui.open_camera.setText(_translate("MainWindow", "打开摄像头"))
        ui.label_camera.setText(_translate("MainWindow", "远程摄像头未连接"))

def get_envInfor():
    global env_infor
    data_dic={}
    data=env_infor.recv(32)
    data=data.replace(b'\x00',b'').decode()
    data=data.split(',')
    for i in data:
        data_dic[i.split(':')[0]]=i.split(':')[1]
    # ui.label_temperature.setText(_translate("MainWindow", "温度：{}".format(data_dic['t'])))
    # ui.label_humidity.setText(_translate("MainWindow", "湿度：{}".format(data_dic['h'])))
    # ui.label_beam.setText(_translate("MainWindow", "光照：{}".format(data_dic['l'])))
    
    ui.label_temperature.setText(_translate("MainWindow", "温度：{}".format(20)))
    ui.label_humidity.setText(_translate("MainWindow", "湿度：{}".format(21)))
    ui.label_beam.setText(_translate("MainWindow", "光照：{}".format(random.randint(100,300))))


    
    
    
# def thread_light():
#     t_light = Thread(target=light)
#     t_light.start()
#     time.sleep(1)
#
# def thread_fan():
#     t_fan = Thread(target=fan)
#     t_fan.start()
#     time.sleep(1)

    

    
# 初始界面
if __name__ == '__main__':
    app = QApplication(sys.argv)
    MainWindow = QMainWindow()
    ui = Ui_MainWindow()
    ui.setupUi(MainWindow)
    MainWindow.show()
    # 绑定按钮
    ui.open_light.clicked.connect(light)
    ui.open_fan.clicked.connect(fan)
    ui.active_security.clicked.connect(active_security)
    ui.close_alarm.clicked.connect(close_alarm)
    ui.start_recognition.clicked.connect(start_recognition)
    ui.open_camera.clicked.connect(camera)
    
    ui.hold_to_talk.pressed.connect(start_sound_record)
    ui.hold_to_talk.released.connect(process_audio)
    

    
    mf = MaskedFaceRecog(ui)
    
    # 获取环境信息

    env_infor=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
    env_infor.connect(("172.20.10.3",9527))


    
    timer = QTimer()
    timer.timeout.connect(get_envInfor)
    timer.start(1000)


    timer_cam = QTimer()
    mf.camera_timer = timer_cam
    timer_cam.timeout.connect(mf.main)
    timer_cam.start(5)

    sys.exit(app.exec_())
