import csv
import pandas as pd
def findstrinfile(filename, lookup):
    return lookup in open(filename,'rt').read()

original=list()
missing_result=list()
result2=list()
predict_result=list()

#dir = "/home/situlingyun/Documents/test-case-missing-check/openssl-1.0.1a/"
#dir = "/home/situlingyun/Documents/test-case-missing-check/openssl-1.0.1a/"
dir ="/home/situlingyun/Documents/test-case-missing-check/libtiff3.4.6/"

joernMetricData=dir + "metrics-openssl-1.0.1a.csv"
#joernMetricData=dir + "metrics-openssl-1.0.1a.csv"
joernMetricData=dir + "libtiff3.4.6.csv"

missing_check_fun=dir+"Train_Function.csv"
vul_fun=dir+"Predicate_Function.csv"
predict_fun=dir + "Predicate_Function.csv"

assumption1_data_label=dir+"assumption1_train_data.csv"
cluster_data_withTrainFun=dir +" Train_Fueature_Joern.csv"
assumption2_data_label=dir +"assumption2_train_data.csv"
predict_data=dir+"predict_data.csv"








#obtain the train data based on casual assumption1
with open(joernMetricData) as feature:
    feature_csv = csv.reader(feature)
    for line in feature_csv:
        if findstrinfile(missing_check_fun, line[0]):
            missing_result.append(line)
            line2 =line+ ["0"]
        else:
            line2 =line+ ["1"]
        original.append(line2)


file=open(assumption1_data_label, 'w')
for v in original:
    for ele in v:
        file.write(str(ele)+",")
    file.write('\n')

print ("obtain assumption1_data")



#obtain the train data based on strict assumption2
for line in missing_result:
    if findstrinfile(vul_fun, line[0]):
        line2=line+ ['1']
    else:
        line2 = line + ['0']
    result2.append(line2)


file=open(assumption2_data_label, 'w')
for v in result2:
    for ele in v:
        file.write(str(ele)+",")
    file.write('\n')

print ("obtain assumption2_data")













