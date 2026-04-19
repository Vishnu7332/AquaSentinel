from flask import Flask,jsonify
import serial

app = Flask(__name__)

ser = serial.Serial('COM3',9600)

@app.route('/lora-data')

def data():

    line = ser.readline().decode().strip()

    ph,turb,temp,tds,cond = line.split(",")

    return jsonify({
    "ph":float(ph),
    "turbidity":float(turb),
    "temperature":float(temp),
    "tds":float(tds),
    "conductivity":float(cond)
    })

app.run(port=5000)

