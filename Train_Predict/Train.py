import pandas as pd
from sklearn import preprocessing
from sklearn.cross_validation import train_test_split
import Models
import pickle
import numpy as np
import matplotlib.pyplot as plt
from itertools import product

dir = "/home/situlingyun/Documents/test-case-missing-check/openssl-1.0.1a/"
#dir = "/home/situlingyun/Documents/test-case-missing-check/openssl-1.0.0a/"
#dir = "/home/situlingyun/Documents/test-case-missing-check/libtiff3.4.6/"

#path= dir + 'originalData_add_label.csv'
path= dir + "assumption2_train_data.csv"
#path= dir + 'assumption2.csv'

def dataPrepare(saveFilePath,logPath):

    csv = pd.read_csv(path)
    csv.columns = range(0, len(csv.columns), 1)

   # le = preprocessing.LabelEncoder()
   # le = preprocessing.LabelEncoder()
   # le.fit_transform(csv[299])  # add the column number (start from 0) of the label
   # csv[299].value_counts()     # add the column number (start from 0) of the label

    x_train, x_test, y_train, y_test = train_test_split(csv.iloc[:, :len(csv.columns.tolist()) - 1], csv.iloc[:, -1],
                                                        test_size=0.3)
    fp = open(logPath+'log.txt','a')
    fp.write('trainData shape is '+str(x_train.shape)+'\n')
    fp.write('trainData shape is '+str(x_test.shape)+'\n')
    fp.close()
    print x_train.shape
    print y_test.shape
    print x_test.shape
    print y_test.shape

    if(saveFilePath):
        csv_train = pd.concat([x_train,y_train],axis = 1)
        csv_test = pd.concat([x_test,y_test],axis = 1)
        csv_train.to_csv(saveFilePath + 'train.csv',index = False)
        csv_test.to_csv(saveFilePath + 'test.csv',index = False)
    return  x_train, x_test, y_train, y_test


def plotMargin(dir):
    csv = pd.read_csv(dir+'train.csv')
    csv.columns = range(0, len(csv.columns), 1)
    X = csv.iloc[:,[44,45]]
   # print x1
    Y = csv.iloc[:,-1]
    print X.shape
    '''
    model = None
    if(modelType =='rf'):
        model = trainRF(X,X,Y,Y,dir)
    elif(modelType == 'svm'):
        model = trainSVM(X,X,Y,Y,dir)
    elif(modelType =='gbdt'):
        model = trainGBDT(X,X,Y,Y,dir)
    elif(modelType=='dt'):
        model = trainDT(X,X,Y,Y,dir)
    else:
        model = trainDT(X,X,Y,Y,dir)
'''
    RandomForest = trainRF(X,X,Y,Y, dir)
    SVM = trainSVM(X,X,Y,Y, dir)
    GBDT = trainGBDT(X,X,Y,Y, dir)
    DecisionTree = trainDT(X,X,Y,Y, dir)

    x_min, x_max = X.iloc[:, 0].min() - 1, X.iloc[:, 0].max() + 1
    y_min, y_max = X.iloc[:, 1].min() - 1, X.iloc[:, 1].max() + 1
    step_x=(float(x_max)-float(x_min))/1000
    step_y = (float(y_max) - float(y_min)) / 1000
    print step_x
    print step_y
    xx, yy = np.meshgrid(np.arange(x_min, x_max, step_x),
                         np.arange(y_min, y_max, step_y))
    print xx.shape ,yy.shape
   # temp  = pd.concat([pd.DataFrame(xx).T,pd.DataFrame(yy).T],axis=1)
    f, axarr = plt.subplots(2, 2, sharex='col', sharey='row', figsize=(10, 8))
    for idx, clf, tt in zip(product([0, 1], [0, 1]),
                        [RandomForest,GBDT, GBDT, DecisionTree],
                        ['RandomForest', 'SVM(RBF)',
                         'GBDT', 'DecisionTree']):
        temp  = pd.DataFrame(np.c_[xx.ravel(), yy.ravel()])
        Z = clf.predict(temp)
        Z = Z.reshape(xx.shape)

        axarr[idx[0], idx[1]].contourf(xx, yy, Z, alpha=0.4)
        axarr[idx[0], idx[1]].scatter(X.iloc[:, 0], X.iloc[:, 1], c=Y, alpha=0.8)
        axarr[idx[0], idx[1]].set_title(tt)

    plt.show()




def modelDump(model,path):
    with open(path, 'wb') as f:
        pickle.dump(model, f)



def modelReload(path):
    with open(path, 'rb') as f:
        model2 = pickle.load(f)
    return model2



def trainDT(x_train, x_test, y_train, y_test,path):
    dt =  Models.DecisionTree('DT')
    dt.build(x_train,y_train,path,max_depth = 10)
    dt.modelEvaluate(x_test,y_test,path)
    #dt.saveTree(path)
    modelDump(dt, path+'dt.txt')
    return dt

def trainSVM(x_train, x_test, y_train, y_test,path):
    svm = Models.SVM('SVM')
    svm.build(x_train,y_train,path,C=10.0, kernel='rbf', gamma=1, shrinking=True, probability=True)
    svm.modelEvaluate(x_test, y_test,path)
    modelDump(svm, path+"svm.txt")
    return svm

def trainGBDT(x_train, x_test, y_train, y_test,path):
    GBDT = Models.GBDT('GBDT')
    GBDT.build(x_train,y_train,path,subsample=0.7)
    GBDT.modelEvaluate(x_test, y_test,path)
    modelDump(GBDT, path+"gbdt.txt")
    return GBDT

def trainRF(x_train, x_test, y_train, y_test, path):
    rf = Models.RF('RF')
    rf.build(x_train,y_train,path,n_estimators=100,max_depth = 6)
    rf.modelEvaluate(x_test, y_test,path)
    rf.featureImportance(x_train,path)
    modelDump(rf, path+ "rf.txt")
    return rf



x_train, x_test, y_train, y_test = dataPrepare(dir,dir)
trainRF(x_train, x_test, y_train, y_test,dir)
#model = modelReload(dir+'rf.txt')
#model.modelEvaluate(x_test,y_test,dir)
#input = [2,0,0,1,0,0,1,1,0,8,23,0,0,1,1]
#print model.predict(input)
trainDT(x_train, x_test, y_train, y_test,dir)
trainGBDT(x_train, x_test, y_train, y_test,dir)
trainSVM(x_train, x_test, y_train, y_test,dir)

#plotMargin(dir)