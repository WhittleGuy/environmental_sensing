#!/usr/bin/python
##########################################
# Author: Brandon Whittle
# Date: 2022-1121
##########################################

import pandas as pd
import requests
from datetime import date, datetime

if __name__ == '__main__':
    store = pd.HDFStore(f'~/data/{date.today()}.h5')
    clients = [7, 13, 12]
    
    for id in clients:
        try:
            res = requests.get(f'http://192.168.0.{id}').json()
            uid = res['id']
            co2 = res['co2']
            if uid == 1:
                temp_0 = res['temp_0']
                temp_1 = res['temp_1']
                humidity = res['hum']
                pm1_0 = res['pm1.0']
                pm2_5 = res['pm2.5']
                pm10_0 = res['pm10.0']

            else:
                temp_0 = -1.
                temp_1 = -1.
                humidity = -1.
                pm1_0 = -1
                pm2_5 = -1
                pm10_0 = -1

            df = pd.DataFrame(data={
                'timestamp': datetime.now(),
                'id': uid,
                'temp_0': temp_0,
                'temp_1': temp_1,
                'humidity': humidity,
                'co2': co2,
                'pm1_0': pm1_0,
                'pm2_5': pm2_5,
                'pm10_0': pm10_0,
            }, index=[0])
            store.put('envSense', df, format='t', append=True, data_columns=True)

        except:
            print(f'No connection: 192.168.0.{id}')
