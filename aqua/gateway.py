import serial
import firebase_admin
from firebase_admin import credentials, db

cred = credentials.Certificate("serviceAccountKey.json")

firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://watermonitoring-36dcf-default-rtdb.asia-southeast1.firebasedatabase.app'
})

ser = serial.Serial('COM7', 115200, timeout=1)

print("Gateway started... Waiting for receiver data")

while True:
    try:
        line = ser.readline().decode('utf-8', errors='ignore').strip()

        if not line:
            continue

        print("Received:", line)

        if not line.startswith("DATA:"):
            continue

        clean = line.replace("DATA:", "")
        parts = clean.split(",")

        if len(parts) != 6:
            print("Invalid data format")
            continue

        location = parts[0]
        ph = float(parts[1])
        turbidity = float(parts[2])
        temperature = float(parts[3])
        tds = float(parts[4])
        conductivity = float(parts[5])

        ref = db.reference(location)
        ref.set({
            "ph": ph,
            "turbidity": turbidity,
            "temperature": temperature,
            "tds": tds,
            "conductivity": conductivity
        })

        print("Uploaded to Firebase under", location)

    except Exception as e:
        print("Error:", e)