'''
author Manjiang Cao 
e-mail <mcao999@connect.hkust-gz.edu.cn>
this script using for draw the relationship between average of RTT time and true distance in the first wireless experiment.
'''


import numpy as np
import matplotlib.pyplot as plt
import os


def read_data(distance):
    f = open ('distance{}.txt'.format(distance), 'r')

    data = f.readlines()

    for i in range(len(data)):
        data[i] = int(data[i])

    data =np.array(data)
    return data

distance_list = [0,1,2,3,4,5,6,10,15,20]

data_list = []
data_mean_list = []
data_var_list = []

for i in distance_list:
    data = read_data(i)
    data_list.append(data)

for i in data_list:
    data_mean_list.append(i.mean())
    data_var_list.append(i.var())

fig = plt.figure()
ax1 = fig.add_subplot(121)
ax1.plot(distance_list,data_mean_list,c = 'r', label = 'mean value')
ax2 = fig.add_subplot(122)
ax2.plot(distance_list,data_var_list,c = 'b', label = 'varance')
plt.legend()
plt.show()