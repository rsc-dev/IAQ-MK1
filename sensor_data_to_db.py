#!/usr/local/bin/python
import time, sys
from influxdb import InfluxDBClient
import requests

json_message = [{
      "measurement": "humidity",
      "fields": {
           "value": 12.34
       }
    }]   

influxClient = InfluxDBClient('localhost', 8086, 'iaq', 'IaQ82!!', 'iaqdata')

def dataToInflux():
  while True:
      while True:  
          try:   
            message = requests.get('http://sensor.local')
            print(message)
          except:
            print("Error")  

    time.sleep(10);

if __name__ == "__main__":
    dataToInflux()
