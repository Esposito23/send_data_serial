import serial
from time import sleep
import os

byteHeader = '#'

def createFirstPack():
    lista = []
    lista.extend([ord(byteHeader)]*2)
    lista.extend([ord(' ')]*18)
    return(lista)


def extractName(encoded, dir_file):
    file_name = os.path.basename(dir_file)
    startName = 3  # posizione da cui partire con il nome
    indice = 0
    for byte_name in file_name:
        encoded[startName+indice] = (ord(byte_name))
        indice += 1
    return(encoded)


def extractSize(encoded, dir_file):
    file_size = os.path.getsize(dir_file)
    encoded[-5] = ((file_size >> 24) & 0xff)
    encoded[-4] = ((file_size >> 16) & 0xff)
    encoded[-3] = ((file_size >> 8) & 0xff)
    encoded[-2] = (file_size & 0xff)
    return(encoded)


def addFileInfo(encoded, dir_file):
    file = open(dir_file, 'rb')
    for byte in file.read():
        encoded.append(byte)
    return(encoded)


ser = serial.Serial('/dev/ttyUSB1', 9600)  # open serial port

dir_file = '/home/esposito/Desktop/send_Serial/testo.txt'

sleep(5)  # tempo per aprire il terminale di arduino

encoded = createFirstPack()

encoded[2] = ord('t')  # da definire meglio lo scopo

encoded = extractName(encoded, dir_file)

encoded = extractSize(encoded, dir_file)

encoded[-1] = ord('c')

encoded = addFileInfo(encoded, dir_file)

for i in range(0,len(encoded),2):
    ser.write(bytearray(encoded[i:i+2]))
    sleep(0.03)

#ser.write(bytearray(encoded))

ser.close()             # chiudo la porta
