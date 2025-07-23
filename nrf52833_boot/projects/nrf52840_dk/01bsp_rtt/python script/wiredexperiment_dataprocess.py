'''
author Manjiang Cao 
e-mail <mcao999@connect.hkust-gz.edu.cn>
this script using for draw the relationship between RTT estimate distance and true distance in the wired experiment.
'''

import numpy as np
import matplotlib.pyplot as plt
import os


def read_data(distance):
    f = open ('C:/Users/11422/Desktop/work_on_nrf52840/openwsn-fw/projects/nrf52840_dk/01bsp_rtt/wiredexperiment/distance{}.txt'.format(distance), 'r')
    time_list = []
    rssi_list = []
    data = f.readlines()
    #print(data)
    for i in range(len(data)):
        time_list.append(int(data[i].split(' ')[0]))
        rssi_list.append(int(data[i].split(' ')[1]))

    time = np.array(time_list)
    rssi = np.array(rssi_list)
    return time,rssi

distance_list = [1,2,3,4,5,6,7,8,9,10,11]
true_list = [1.2,2.2,3.2,4.2,5.2,6.2,7.2,8.2,9.2,10.2,11.2]

data_time_list = []
data_rssi_list = []
data_time_mean_list = []
data_rssi_mean_list = []
data_time_var_list = []
data_rssi_var_list = []

for i in distance_list:
    time,rssi = read_data(i)
    data_time_list.append(time)
    data_rssi_list.append(rssi)

for i in data_time_list:
    #print(i[:500].shape)
    data_time_mean_list.append(i[:200].mean())
    data_time_var_list.append(i[:200].var())

for i in data_rssi_list:
    data_rssi_mean_list.append(i.mean())
    data_rssi_var_list.append(i.var())

based_mean_time = data_time_mean_list[0]
for i in range(len(data_time_mean_list)):
    #222222222 is the signal speed in the antenna extention cable. i.e. 4.5us/meter
    #16000000 means the frequency of clock is 16MHz
    #20074.659 is the average RTT time when the distance between two node is zero
    data_time_mean_list[i] = ((data_time_mean_list[i] - 20074.659)/2)/16000000*222222222




fig = plt.figure()
ax1 = fig.add_subplot(211)
ax1.plot(true_list,data_time_mean_list,c = 'r', label = 'mean',marker='o')
ax1.set_xlabel('distance')
ax1.set_ylabel('RTT distance')
ax1.grid()
plt.legend()
#ax2 = fig.add_subplot(222)
#ax2.plot(distance_list,data_time_var_list,c = 'b', label = 'varance of time')
#ax2.set_xlabel('distance')
#plt.legend()
ax3 = fig.add_subplot(212)
ax3.plot(true_list,data_rssi_mean_list,c = 'r', label = 'mean rssi')
ax3.set_xlabel('distance')
ax3.set_ylabel('rssi')
plt.legend()
#ax4 = fig.add_subplot(224)
#ax4.plot(distance_list,data_rssi_var_list,c = 'b', label = 'varance of rssi')
#ax4.set_xlabel('distance')
plt.legend()
plt.show()

data_time_list_float = [arr.astype(float) for arr in data_time_list]

for arr in data_time_list_float:
    arr -= 20074.659
    print(arr)

print(data_time_list_float)

fig = plt.figure()
plt.boxplot(data_time_list_float, showfliers=False, showmeans=True, meanline=True)
x_labels = [ '1.2 meter', '2.2 meter', '3.2 meter', '4.2 meter',
             '5.2 meter', '6.2 meter', '7.2 meter', '8.2 meter', 
             '9.2 meter', '10.2 meter', '11.2 meter']
plt.xticks(range(1,len(distance_list)+1), x_labels)


plt.show()

