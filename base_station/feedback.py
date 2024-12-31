from datatools import SerialReaderWriter
from datatools import CSVWriter
from datetime import datetime as dt

FILE_HEADER = ["Sent", "Received"]
FILE_NAME = f"./feedback_logs/{dt.now().strftime('%m-%d-%Y_T%H-%M-%S')}.csv"

r = SerialReaderWriter("COM6", 9600)
w = CSVWriter(FILE_NAME)

w.write(FILE_HEADER)

values = range(0, 110, 10)
for v in values:
    r.write(v)
    response = int(r.read())

    print(v, response)
    w.write([v, response])
