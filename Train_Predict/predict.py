import pandas as pd
from sklearn import preprocessing
from sklearn.cross_validation import train_test_split
import Models
import pickle
import numpy as np
import matplotlib.pyplot as plt
from itertools import product
import Train
import Models
import csv

Predict_model_dir = '/home/situlingyun/PycharmProjects/Train_Predict/'
#path= dir + 'originalData_add_label.csv'
predict_data_dir='/home/situlingyun/PycharmProjects/csv_handle/data_prepair/openssl1.0.0a/'


open_path= predict_data_dir + 'predict_data.csv'
output_path=predict_data_dir + "predict_result.csv"

def modelReload(path):
    with open(path, 'rb') as f:
        model2 = pickle.load(f)
    return model2

modelType = "RF"

if(modelType=="RF"):
    model = modelReload(Predict_model_dir+'rf.txt')
elif(modelType=="DT"):
    model = modelReload(Predict_model_dir + 'dt.txt')
elif (modelType == "GBDT"):
    model = modelReload(Predict_model_dir + 'gbdt.txt')
elif(modelType=="svm"):
    model = modelReload(Predict_model_dir + 'svm.txt')
else:
    model=modelReload(Predict_model_dir+'rf.txt')


predict_result_list=list()
with open(open_path) as feature:
    feature_csv = csv.reader(feature)
    for line in feature_csv:
        #line2=line[1:]
        predict_data = np.array(line).reshape((1, -1))
        #print line
        #print line2
        #print predict_data
        predict_result=model.predict(predict_data)
       # print predict_result,predict_result[0],str(predict_result[0])
        if(str(predict_result[0])=='1'):
            line+=['1']
            #print line
        elif(str(predict_result[0])=='0'):
            line+=['0']
            #print line
        predict_result_list.append(line)


file=open(output_path, 'w')
for v in predict_result_list:
    for ele in v:
        file.write(str(ele)+",")
    file.write('\n')

