'''
Description: 
Author: Huang Wen
Date: 2023-03-04 15:31:51
LastEditTime: 2023-03-08 14:20:40
LastEditors: Huang Wen
'''

from model.SSD import FaceMaskDetection
from model.FACENET import InceptionResnetV1
import torch
import cv2
import numpy as np
import glob
import time
import tqdm
import socket
from PIL import Image,ImageDraw,ImageFont
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
import os

legal_host=['黄雯']

class MaskedFaceRecog:
    def __init__(self,ui):
        self.camera_timer=None
        self.frame_data=b''
        self.state=1
        self.piclen = 0
        self.ui=ui
        self.show=False
        self.infor={'status':'NULL','name':'NULL'}
        # 加载检测模型
        face_mask_model_path = r'weights/SSD/face_mask_detection.pb'
        self.ssd_detector = FaceMaskDetection(face_mask_model_path,margin=0,GPU_ratio=0.1)
        # 加载识别模型
        self.device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")
        self.facenet = InceptionResnetV1(is_train=False,embedding_length=128,num_classes=14575).to(self.device)
        self.facenet.load_state_dict(torch.load(r'./weights/Chinese_CASIA_ALL_AG_epoch/facenet_best.pt',map_location=self.device))
        self.facenet.eval()
        self.name_list,self.known_embedding = self.loadFaceFeats()
        self.name_list.append('未知')
        self.name_png_list = self.getNamePngs(self.name_list)
        self.mask_class_overlay = self.getMaskClassPngs()

        self.camera=None
        self.start_cam=False

        self.start_recog = False

    def getMaskClassPngs(self):
        '''
        加载佩戴和未佩戴标志
        '''
        labels = ['masked','without_mask']
        overlay_list = []
        for label in labels:
            fileName = './images/%s.png' % (label)
            overlay = cv2.imread(fileName,cv2.COLOR_RGB2BGR)
            overlay = cv2.resize(overlay,(0,0), fx=0.2, fy=0.2)
            overlay_list.append(overlay)
        return overlay_list

    def readPngFile(self,fileName):
        '''
        读取PNG图片
        '''
        # 解决中文路径问题
        png_img = cv2.imdecode(np.fromfile(fileName,dtype=np.uint8),-1)
        # 转为BGR，变成3通道
        png_img = cv2.cvtColor(png_img,cv2.COLOR_RGB2BGR)
        png_img = cv2.resize(png_img,(0,0), fx=0.4, fy=0.4)
        return png_img


    def getNamePngs(self,name_list):
        '''
        生成每个人的名称PNG图片（以解决中文显示问题）
        '''
        real_name_list = []
        for name in name_list:
            real_name = name.split('_')[0]
            if real_name not in real_name_list:
                real_name_list.append(real_name)
        pngs_list = {}
        for name in tqdm.tqdm(real_name_list,desc='生成人脸标签PNG...'):
            filename = './images/name_png/'+name+'.png'
            if os.path.exists(filename):
                png_img = self.readPngFile(filename)
                pngs_list[name] = png_img
                continue
            bg = Image.new("RGBA",(400,100),(0,0,0,0))
            # 添加文字
            d = ImageDraw.Draw(bg)
            font  = ImageFont.truetype('./fonts/MSYH.ttc',80,encoding="utf-8")
            if name == 'Unknown':
                color = (0,0,255,255)
            else:
                color = (0,255,0,255)

            d.text((0,0),name,font=font,fill=color)
            # 保存
            bg.save(filename)
            # 再次检查
            if os.path.exists(filename):
                png_img = self.readPngFile(filename)
                pngs_list[name] = png_img
            
        return pngs_list
            
    def loadFaceFeats(self):
        '''
        加载目标人的特征
        '''
        name_list = []
        # 输入网络的所有人脸图片
        known_faces_input = []
        # 遍历
        known_face_list = glob.glob('./images/origin/*')
        for face in tqdm.tqdm(known_face_list,desc='处理目标人脸...'):
            name = face.split('\\')[-1].split('.')[0]
            name_list.append(name)
            # 裁剪人脸
            croped_face = self.getCropedFaceFromFile(face)
            if croped_face is None:
                print('图片：{} 未检测到人脸，跳过'.format(face))
                continue
            # 预处理
            img_input = self.imgPreprocess(croped_face)
            known_faces_input.append(img_input)
        # 转为Nummpy
        faces_input = np.array(known_faces_input)
        # 转tensor并放到GPU
        tensor_input = torch.from_numpy(faces_input).to(self.device)
        # 得到所有的embedding,转numpy
        known_embedding = self.facenet(tensor_input).detach().cpu().numpy()
        return name_list,known_embedding

    def getCropedFaceFromFile(self,img_file, conf_thresh=0.5 ):
        # 读取图片
        # 解决中文路径问题
        img_ori = cv2.imdecode(np.fromfile(img_file,dtype=np.uint8),-1)
        
        if img_ori is None:
            return None
        # 转RGB
        img = cv2.cvtColor(img_ori,cv2.COLOR_BGR2RGB)
        # 缩放
        img = cv2.resize(img,self.ssd_detector.img_size)
        # 转float32
        img = img.astype(np.float32)
        # 归一
        img /= 255
        # 增加维度
        img_4d = np.expand_dims(img,axis=0)
        # 原始高度和宽度
        ori_h,ori_w = img_ori.shape[:2]
        bboxes, re_confidence, re_classes, re_mask_id = self.ssd_detector.inference(img_4d,ori_h,ori_w)
        for index,bbox in enumerate(bboxes):
            class_id = re_mask_id[index] 
            l,t,r,b = bbox[0], bbox[1], bbox[0] + bbox[2], bbox[1] + bbox[3]

            croped_face = img_ori[t:b,l:r]
            return croped_face
        # 都不满足
        return None

    def imgPreprocess(self,img):
        # 转为float32
        img = img.astype(np.float32)
        # 缩放
        img = cv2.resize(img,(112,112))
        # BGR 2 RGB
        img = cv2.cvtColor(img,cv2.COLOR_BGR2RGB)
        # h,w,c 2 c,h,w
        img = img.transpose((2,0,1))
        # 归一化[0,255] 转 [-1,1]
        img = (img - 127.5) / 127.5
        # 增加维度
        # img = np.expand_dims(img,0)
        return img


    def main(self):
        if self.show==False:
            return
        if self.start_cam==False:
            print("连接摄像头")
            self.camera = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.camera.connect(("172.20.10.3", 8080))
            # 修改接收缓冲区
            self.camera.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 1000000)
            self.start_cam=True

        threshold = 1  # 阈值
        frame_h = 480
        frame_w = 640
        camera = self.camera
        if self.state == 1:
            bytes_available = camera.recv(10, socket.MSG_PEEK)
            if len(bytes_available) < 10:
                # continue
                return
            else:
                self.frame_data = camera.recv(10)
                self.frame_data = self.frame_data.replace(b'\x00', b'')
                self.piclen = int(self.frame_data)
                self.state = 2
                self.frame_data = b''
                # continue
                # print("piclen:"+str(self.piclen))
                return
        elif self.state == 2:
            bytes_available = camera.recv(self.piclen, socket.MSG_PEEK)
            if len(bytes_available) < self.piclen:
                # continue
                # print("图片长度："+str(len(bytes_available)))
                return                                       
            else:
                self.frame_data = camera.recv(self.piclen)
                self.state = 1
                img = np.frombuffer(self.frame_data,dtype=np.uint8)
                img = cv2.imdecode(img, cv2.IMREAD_COLOR)

        frame = img
   
        self.camera_timer.stop()
        frame = cv2.flip(frame,1)
        img = cv2.cvtColor(frame,cv2.COLOR_BGR2RGB)
        img = cv2.resize(img,self.ssd_detector.img_size)
        img = img.astype(np.float32)
        img /= 255
        img_4d = np.expand_dims(img,axis=0)
        bboxes, re_confidence, re_classes, re_mask_id = self.ssd_detector.inference(img_4d,frame_h,frame_w)
        for index,bbox in enumerate(bboxes):
            class_id = re_mask_id[index]
            # conf  = re_confidence[index]
            if class_id == 0:
                color = (0, 255, 0)  # 戴口罩
            elif class_id == 1:
                color = (0, 0, 255)  # 没带口罩
            l,t,r,b = bbox[0], bbox[1], bbox[0] + bbox[2], bbox[1] + bbox[3]
            # 裁剪人脸
            crop_face = frame[t:b,l:r]
            # 人脸识别
            img = crop_face.astype(np.float32)
            # 缩放
            img = cv2.resize(img,(112,112))
            img = cv2.cvtColor(img,cv2.COLOR_BGR2RGB)
            img = img.transpose((2,0,1))
            img = (img - 127.5) / 127.5
            img_input = np.expand_dims(img,0)
            tensor_input = torch.from_numpy(img_input).to(self.device)
            embedding = self.facenet(tensor_input)
            embedding = embedding.detach().cpu().numpy()
            # 计算距离
            dist_list = np.linalg.norm((embedding-self.known_embedding),axis=1)
            min_index = np.argmin(dist_list)
            pred_name = self.name_list[min_index]
            min_dist = dist_list[min_index]
            if min_dist < threshold:
                real_name = pred_name.split('_')[0]
                name_overlay = self.name_png_list[real_name]
                self.infor['name']= real_name
                global legal_host
                if real_name in legal_host:
                    self.infor['status']='主人'
                else:
                    self.infor['status']='来访者'
            else:
                # 未识别到，加载未知
                name_overlay = self.name_png_list['未知']
                self.infor['name']='未知'
                self.infor['status']='陌生人'
            # √和×标志
            class_overlay = self.mask_class_overlay[class_id]
            # 拼接两个PNG
            overlay = np.zeros((40, 210,3), np.uint8)
            overlay[:40, :40] = class_overlay
            overlay[:40, 50:210] = name_overlay
            # 覆盖显示
            overlay_h,overlay_w = overlay.shape[:2]
            # 覆盖范围
            overlay_l,overlay_t = l,(t - overlay_h-20)
            overlay_r,overlay_b = (l + overlay_w),(overlay_t+overlay_h)
            # 判断边界
            if overlay_t > 0 and overlay_r < frame_w:
                overlay_copy=cv2.addWeighted(frame[overlay_t:overlay_b, overlay_l:overlay_r ],1,overlay,20,0)
                frame[overlay_t:overlay_b, overlay_l:overlay_r ] =  overlay_copy
            cv2.rectangle(frame,(l,t),(r,b),color,2)
        try:
            frame = cv2.resize(frame, (900, 700), interpolation=cv2.INTER_AREA)
            frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        except Exception as e:
            return 
        # 转为QImage对象
        showImage = QImage(
        frame.data, frame.shape[1], frame.shape[0], frame.shape[1]*3, QImage.Format_RGB888)
        self.ui.label_camera.setPixmap(QPixmap.fromImage(showImage))
        self.camera_timer.start()