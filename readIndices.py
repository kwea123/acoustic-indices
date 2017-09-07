
# coding: utf-8

import datetime
from os import listdir
from os.path import getsize
import pandas as pd
import sys
import matplotlib.pyplot as plt
import struct
import numpy as np
NAMES = ["ACI","H_T","CVR","FFT"]

def indices_pdf(matrices, names, startTime, fileName, sampleRate=48000):
    """
    plot each normalized index as a false color image in one pdf file
    """
    fig = plt.figure(figsize=(16,24))
    plt.subplots_adjust(bottom=0.05, top=0.95, left=0.05, right=0.95, hspace=0.30)
    n = matrices.shape[0]
    length = matrices.shape[1]
    start = datetime.datetime.fromtimestamp(startTime)
    end = datetime.datetime.fromtimestamp(startTime+length*60)
    dates = pd.date_range(start, end, freq='30min')
    for i in range(n-1):
        ax = fig.add_subplot(n,1,i+1)
        ax.set_title(names[i], fontsize = 20)
        ax.set_xlabel("Time", fontsize=10)
        ax.set_ylabel("Frequency (kHz)", fontsize=10)
        ax.set_xticks(range(0,length+1,30))
        ax.set_xticklabels(dates.map(lambda x: x.strftime("%H:%M")))
        ax.set_xlim(0,length)
        ax.set_yticks(range(0,sampleRate//2000+1,2))
        ax.set_ylim(0,sampleRate/2000)
        nor = matrices[i]
        if names[i]=="ACI":
            r = nor
        elif names[i]=="H_T":
            g = nor
        elif names[i]=="CVR":
            b = nor
        plt.imshow(nor.swapaxes(0,1),extent=[0,length,0,sampleRate/2000],origin="lower",aspect="auto")

    img_data = np.stack((r,g,b),axis=2)
    ax = fig.add_subplot(n,1,n)
    ax.set_xlabel("Time", fontsize=10)
    ax.set_ylabel("Frequency (kHz)", fontsize=10)
    ax.set_xticks(range(0,length+1,30))
    ax.set_xticklabels(dates.map(lambda x: x.strftime("%H:%M")))
    ax.set_xlim(0,length)
    ax.set_yticks(range(0,sampleRate//2000+1,2))
    ax.set_ylim(0,sampleRate/2000)
    ax.set_title("False Color Image using ACI,H_T,CVR", fontsize = 20)
#     fig.autofmt_xdate()
    plt.imshow(img_data.swapaxes(0,1),extent=[0,length,0,sampleRate/2000],origin="lower",aspect="auto")
    plt.savefig(fileName)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        sys.exit("Please specify the folder where the .bin files are.")
    folder = sys.argv[1]
    files = listdir('./'+folder)
    for fileName in files:
        filePath = './'+folder+'/'+fileName
        duration = getsize(filePath)//4096 #divide by 1024 (kB/B) and 4 (floating point) to get the duration in seconds
        f = open('./'+folder+'/'+fileName,'rb')
        data = np.array([struct.unpack('f', f.read(4))[0] for i in range(256*4*duration)])
        data = np.transpose(data.reshape((duration,256,4)),(2,0,1)) #rearrange the data in the order of ACI,H[t],CVR,FFT
        f.close()
        fileNameWithoutExtension = fileName.split(".")[0]
        startTime = int(fileNameWithoutExtension,16)
        indices_pdf(data,NAMES,startTime,'./'+folder+'/'+fileNameWithoutExtension+".PDF")
