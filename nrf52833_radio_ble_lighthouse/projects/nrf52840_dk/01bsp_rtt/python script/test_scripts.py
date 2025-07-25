'''
author Manjiang Cao 
e-mail <mcao999@connect.hkust-gz.edu.cn>
this script trying to using ML to predict the distance based on the second RTT wireless experiment
'''

import numpy as np
import matplotlib.pyplot as plt
import os
from scipy.linalg import svd

from sklearn import linear_model
from sklearn.linear_model import LinearRegression
from sklearn.linear_model import Ridge, Lasso
from sklearn.metrics import mean_squared_error

import torch
import torch.nn as nn
import torch.optim as optim

from sklearn.mixture import GaussianMixture

import csv

def save_data_to_csv(data, filename):
    with open(filename, mode='w', newline='') as file:
        writer = csv.writer(file)
        writer.writerows(data)

def read_csv(filename):
    data = []
    with open('C:/Users/11422/Desktop/work_on_nrf52840/openwsn-fw/projects/nrf52840_dk/01bsp_rtt/python script/dataset/' + filename, mode='r') as file:
        reader = csv.reader(file)
        for row in reader:
            float_row = [float(element) for element in row]
            data.append(float_row)
    return data

X = np.array(read_csv('train_x.csv'))[:,:]
Y = np.array(read_csv('train_y.csv'))
test_x = np.array(read_csv('test_x.csv'))[:,:]
test_y = np.array(read_csv('test_y.csv'))
'''
columns_to_remove = []
X = np.delete(X, columns_to_remove, axis=1)
test_x = np.delete(test_x, columns_to_remove, axis=1)

X = torch.from_numpy(X).float()

Y = torch.from_numpy(Y.reshape(16*50,1)).float()
test_x = torch.from_numpy(test_x).float()
test_y = torch.from_numpy(test_y.reshape((16,1))).float()
'''

class RegressionNet(nn.Module):
        def __init__(self, input_size, hidden_size, output_size):
            super(RegressionNet, self).__init__()
            self.fc1 = nn.Linear(input_size, hidden_size)
            self.relu = nn.ReLU()
            self.fc2 = nn.Linear(hidden_size, hidden_size)
            self.relu = nn.ReLU()
            self.fc3 = nn.Linear(hidden_size, output_size)



        def forward(self, x): 
            out = self.fc1(x)
            out = self.relu(out)
            out = self.fc2(out)
            out = self.relu(out)
            out = self.fc3(out)
            return out
      
def model_train(columns_to_remove,X,Y,test_x,test_y):

    columns_to_remove = columns_to_remove
    X = np.delete(X, columns_to_remove, axis=1)
    test_x = np.delete(test_x, columns_to_remove, axis=1)

    X = torch.from_numpy(X).float()
    Y = torch.from_numpy(Y.reshape(16*50,1)).float()
    test_x = torch.from_numpy(test_x).float()
    test_y = torch.from_numpy(test_y.reshape((16,1))).float()

    # 定义模型的超参数
    input_size = 10 - len(columns_to_remove)
    hidden_size = 512
    output_size = 1

    # 创建模型实例
    model = RegressionNet(input_size, hidden_size, output_size)

    # 定义损失函数和优化器
    criterion = nn.MSELoss()
    optimizer = optim.Adam(model.parameters(), lr=0.001)

    # 进行模型训练
    num_epochs = 2000
    train_loss = []
    test_loss = []
    for epoch in range(num_epochs):
        # 前向传播
        outputs = model(X)
        loss = criterion(outputs, Y) #+ 0.0001*data_loss(X,Y,outputs)

        # 反向传播和优化
        optimizer.zero_grad()
        loss.backward()
        optimizer.step()

        # 打印训练过程中的损失
        print(f'Epoch [{epoch+1}/{num_epochs}], Loss: {loss.item():.4f}')
        train_loss.append(loss.item())

        predictions = model(test_x)
        tt_loss = criterion(predictions,test_y)
        test_loss.append(tt_loss.item())

    # 在测试集上进行预测
    with torch.no_grad():
        predictions = model(test_x)

    tt_loss = criterion(predictions,test_y)
    print(tt_loss.item())
    # 将预测结果转换为numpy数组
    predictions = predictions.numpy()
    return tt_loss.item()

without_GMM_list = []
GMM_list = [] 
for i in range(10):
    without_GMM_list.append(model_train([0,1,2,3,4,5],X,Y,test_x,test_y))
    GMM_list.append(model_train([],X,Y,test_x,test_y))

plt.figure()
plt.boxplot([without_GMM_list,GMM_list])
plt.title('loss of two different model')
plt.xticks([1, 2], ['without GMM', 'GMM'])
plt.xlabel('model')
plt.ylabel('loss')
plt.show()
'''

# 定义模型的超参数
input_size = 10 - len(columns_to_remove)
hidden_size = 512
output_size = 1

# 创建模型实例
model = RegressionNet(input_size, hidden_size, output_size)

# 定义损失函数和优化器
criterion = nn.MSELoss()
optimizer = optim.Adam(model.parameters(), lr=0.001)

# 进行模型训练
num_epochs = 2000
train_loss = []
test_loss = []
for epoch in range(num_epochs):
    # 前向传播
    outputs = model(X)
    loss = criterion(outputs, Y) #+ 0.0001*data_loss(X,Y,outputs)

    # 反向传播和优化
    optimizer.zero_grad()
    loss.backward()
    optimizer.step()

    # 打印训练过程中的损失
    print(f'Epoch [{epoch+1}/{num_epochs}], Loss: {loss.item():.4f}')
    train_loss.append(loss.item())

    predictions = model(test_x)
    tt_loss = criterion(predictions,test_y)
    test_loss.append(tt_loss.item())

# 在测试集上进行预测
with torch.no_grad():
    predictions = model(test_x)

tt_loss = criterion(predictions,test_y)
print(tt_loss.item())
# 将预测结果转换为numpy数组
predictions = predictions.numpy()


plt.figure()
ax = plt.subplot(121)
ax.scatter([i for i in range(len(test_x))],predictions, c = 'b',label = 'ML_predictions')
ax.plot([i for i in range(len(test_x))],[i for i in range(len(test_x))],c = 'r',label = 'true value')
ax.plot([i for i in range(len(test_x))],[i+1 for i in range(len(test_x))],c = 'r',label = '+1 error boundary',linestyle = '--')
ax.plot([i for i in range(len(test_x))],[i-1 for i in range(len(test_x))],c = 'r',label = '-1 error boundary',linestyle = '--')
ax.legend()
ax = plt.subplot(122)
ax.plot([i for i in range(num_epochs)],train_loss,c='r',label='training loss')
ax.plot([i for i in range(num_epochs)],test_loss,c='b',label='test loss')
ax.legend()
plt.show()
'''