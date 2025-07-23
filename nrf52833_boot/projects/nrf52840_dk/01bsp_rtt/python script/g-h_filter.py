'''
author Manjiang Cao 
e-mail <mcao999@connect.hkust-gz.edu.cn>
this script trying to using g-h filter on wireless experiment data
'''

import numpy as np
import matplotlib.pyplot as plt
import os
import ml_experiment2 as ml

class GHFilter:
    def __init__(self, gain, history):
        self.gain = gain
        self.history = history
        self.pred = None
        self.est = None
        self.dx = 0

    def update(self, measurement):
        if self.est is None:
            self.est = measurement
        else:
            self.pred = self.est + self.dx
            residual = measurement - self.pred
            self.dx = self.dx + self.history*residual
            self.est = self.pred + self.gain*residual
        return self.est
    
    def show_parameters(self):
        print('self.pred:',self.pred)
        print('self.est:',self.est)


def remove_outliers(data):
    mean = np.mean(data)
    std = np.std(data)
    threshold = 2 * std
    filtered_data = data[np.abs(data - mean) < threshold]
    return filtered_data

sample_times = 10
distance_list = [i for i in range(16)]
data_list = []
train_data = []
test_data = []
for i in distance_list:
    data = ml.read_data(i)
    data_list.append(data)

train_data,test_data = ml.construct_train_and_test_data(data_list)
train_data = np.array(train_data)       #(16,2,1000)
test_data = np.array(test_data)         #(16,2,1000)

plt.figure()
plt.plot([i for i in range(1000)],train_data[0:1,0:1,:].reshape((1000,)))
plt.show()


train_data_time_filter = []
train_data_rssi_filter = []
for i in range(len(train_data)):
    time_filter = GHFilter(0.2,0.1)
    rssi_filter = GHFilter(0.5,0.1)
    for j in range(len(train_data[i][0])):
        time_state = time_filter.update(train_data[i][0][j])
        rssi_state = rssi_filter.update(train_data[i][1][j])
    train_data_time_filter.append(time_state)
    train_data_rssi_filter.append(rssi_state)

train_data_mean = np.mean(train_data, axis=2, keepdims=True)
test_data_mean = np.mean(test_data, axis=2, keepdims=True)

plt.figure()
ax = plt.subplot(211)
ax.plot([i for i in range(16)],train_data_mean[:,0:1,:].reshape((16,1)),c = 'r',label = 'time mean')
ax.plot([i for i in range(16)],train_data_time_filter,c = 'b', label = 'time filter')
plt.legend()
ax = plt.subplot(212)
ax.plot([i for i in range(16)],train_data_mean[:,1:2,:].reshape((16,1)),c = 'r',label = 'rssi mean')
ax.plot([i for i in range(16)],train_data_rssi_filter,c = 'b', label = 'rssi filter')
plt.legend()
plt.show()





