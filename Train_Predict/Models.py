from sklearn.ensemble import GradientBoostingClassifier
from sklearn.ensemble import RandomForestClassifier
from sklearn.tree import DecisionTreeClassifier
from sklearn.metrics import accuracy_score,precision_recall_curve,precision_score,recall_score
import datetime
import pandas as pd
from sklearn.tree import export_graphviz
import numpy as np
import sklearn.svm.libsvm as libsvm
class models():

    model = None

    def __init__(self,name):
        self.name = name

    def _build(self,x_train,y_train,path):
        fp = open(path + 'log.txt', 'a')
        fp.write('Start training '+self.name + '\n')
        start = datetime.datetime.now()
        self.model.fit(x_train, y_train)
        end = datetime.datetime.now()
        print self.name+' training time is '+str(end - start)
        fp.write(self.name+' training time is '+str(end - start) + '\n')
        fp.close()

    def modelEvaluate(self, x_test,y_test,path):
        start = datetime.datetime.now()
        pred_y = self.model.predict(x_test)
        end = datetime.datetime.now()
        fp = open(path + 'log.txt', 'a')
        fp.write('Start evaluate '+self.name + '\n')
        fp.write(self.name + ' predicte time is ' + str(end - start) + '\n')
        print self.name + ' predicte time is ' + str(end - start)

        print 'crosstab:{0}'.format(pd.crosstab(y_test, pred_y))
        print 'accuracy_score:{0}'.format(accuracy_score(y_test, pred_y))
        print '0precision_score:{0}'.format(precision_score(y_test, pred_y, pos_label=0))
        print '0recall_score:{0}'.format(recall_score(y_test, pred_y, pos_label=0))
        print '1precision_score:{0}'.format(precision_score(y_test, pred_y, pos_label=1))
        print '1recall_score:{0}'.format(recall_score(y_test, pred_y, pos_label=1))

        fp.write('crosstab:{0}'.format(pd.crosstab(y_test, pred_y))+ '\n')
        fp.write('accuracy_score:{0}'.format(accuracy_score(y_test, pred_y)) + '\n')
        fp.write('0precision_score:{0}'.format(precision_score(y_test, pred_y, pos_label=0)) + '\n')
        fp.write('0recall_score:{0}'.format(recall_score(y_test, pred_y, pos_label=0)) + '\n')
        fp.write('1precision_score:{0}'.format(precision_score(y_test, pred_y, pos_label=1)) + '\n')
        fp.write('1recall_score:{0}'.format(recall_score(y_test, pred_y, pos_label=1)) + '\n')

        fp.close()

    def predictP(self,input):
        input = np.array(input).reshape(1, -1)
        result = self.model.predict_proba(input)
        return result

    def predict(self,input):
        input = np.array(input)
        result = self.model.predict(input)
        return result

class DecisionTree(models):

    def build(self, x_train,y_train,path,**parameter):
        self.model = DecisionTreeClassifier(**parameter)
        self._build(x_train,y_train,path)

    def saveTree(self,path):
        with open(path+"dt.dot", 'w') as f:
            export_graphviz(self.model, out_file=f,feature_names=
            ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '10', '11', '12', '13', '14', '15', '16', '17', '18', '19', '20', '21', '22', '23', '24', '25', '26', '27', '28', '29', '30', '31', '32', '33', '34', '35', '36', '37', '38', '39', '40', '41', '42', '43', '44'])
            #, '46', '47', '48', '49', '50', '51', '52', '53', '54', '55', '56', '57', '58', '59', '60', '61', '62', '63', '64', '65', '66', '67', '68', '69', '70', '71', '72', '73', '74', '75', '76', '77', '78', '79', '80', '81', '82', '83', '84', '85', '86', '87', '88', '89', '90', '91', '92', '93', '94', '95', '96', '97', '98', '99', '100', '101', '102', '103', '104', '105', '106', '107', '108', '109', '110', '111', '112', '113', '114', '115', '116', '117', '118', '119', '120', '121', '122', '123', '124', '125', '126', '127', '128', '129', '130', '131', '132', '133', '134', '135', '136', '137', '138', '139', '140', '141', '142', '143', '144', '145', '146', '147', '148', '149', '150', '151', '152', '153', '154', '155', '156', '157', '158', '159', '160', '161', '162', '163', '164', '165', '166', '167', '168', '169', '170', '171', '172', '173', '174', '175', '176', '177', '178', '179', '180', '181', '182', '183', '184', '185', '186', '187', '188', '189', '190', '191', '192', '193', '194', '195', '196', '197', '198', '199', '200', '201', '202', '203', '204', '205', '206', '207', '208', '209', '210', '211', '212', '213', '214', '215', '216', '217', '218', '219', '220', '221', '222', '223', '224', '225', '226', '227', '228', '229', '230', '231', '232', '233', '234', '235', '236', '237', '238', '239', '240', '241', '242', '243', '244', '245', '246', '247', '248', '249', '250', '251', '252', '253', '254', '255', '256', '257', '258', '259', '260', '261', '262', '263', '264', '265', '266', '267', '268', '269', '270', '271', '272', '273', '274', '275', '276', '277', '278', '279', '280', '281', '282',
            # '283', '284', '285', '286', '287', '288', '289', '290', '291', '292', '293', '294', '295', '296', '297', '298'])



class GBDT(models):
    def build(self, x_train, y_train, path,**parameter):
        self.model =GradientBoostingClassifier(**parameter)
        self._build(x_train, y_train,path)


class RF(models):
    def build(self, x_train, y_train, path,**parameter):
        self.model = RandomForestClassifier(**parameter)
        self._build(x_train, y_train,path)

    def featureImportance(self,x_train,path):
        importance = self.model.feature_importances_
        indices = np.argsort(importance)[::-1]
        #print indices
        col_list = ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '10', '11', '12', '13', '14', '15', '16', '17',
                    '18', '19', '20', '21', '22', '23', '24', '25', '26', '27', '28', '29', '30', '31', '32', '33', '34',
                    '35', '36', '37', '38', '39', '40', '41', '42', '43', '44', '45']
            #, '46', '47', '48', '49', '50', '51', '52',
             #       '53', '54', '55', '56', '57', '58', '59', '60', '61', '62', '63', '64', '65', '66', '67', '68', '69', '70', '71', '72', '73', '74', '75', '76', '77', '78', '79', '80', '81',
              #      '82', '83', '84', '85', '86', '87', '88', '89', '90', '91', '92', '93', '94', '95', '96', '97', '98', '99', '100', '101', '102', '103', '104', '105', '106', '107', '108',
               #     '109', '110', '111', '112', '113', '114', '115', '116', '117', '118', '119', '120', '121', '122', '123', '124', '125', '126', '127', '128', '129', '130', '131', '132', '133', '134', '135', '136', '137', '138', '139', '140', '141', '142', '143', '144', '145', '146', '147', '148', '149', '150', '151', '152', '153', '154', '155', '156', '157', '158', '159', '160', '161', '162', '163', '164', '165', '166', '167', '168', '169', '170', '171', '172', '173', '174', '175', '176', '177', '178', '179', '180', '181', '182', '183', '184', '185', '186', '187', '188', '189', '190', '191', '192', '193', '194', '195', '196', '197', '198', '199', '200', '201', '202', '203', '204', '205', '206', '207', '208', '209', '210', '211', '212', '213', '214', '215', '216', '217', '218', '219', '220', '221', '222', '223', '224', '225', '226', '227', '228', '229', '230', '231', '232', '233', '234', '235', '236', '237', '238', '239', '240', '241', '242', '243', '244', '245', '246', '247', '248', '249', '250', '251', '252', '253', '254', '255', '256', '257', '258', '259', '260', '261', '262', '263', '264', '265', '266', '267', '268', '269', '270', '271', '272', '273', '274', '275', '276', '277', '278', '279', '280', '281', '282', '283', '284', '285', '286', '287', '288', '289', '290', '291', '292', '293', '294', '295', '296', '297', '298','299']



        fp = open(path + 'log.txt', 'a')
        for f, name in zip(range(x_train.shape[1]), col_list):
            print "%d. feature %d (%f)(name = %s)" % (f + 1, indices[f], importance[indices[f]], col_list[indices[f]])
            fp.write("%d. feature %d (%f)(name = %s)" % (f + 1, indices[f], importance[indices[f]], col_list[indices[f]]))
        fp.close()



class SVM(models):

    def build(self, x_train, y_train,path, **parameter):
        x = x_train.as_matrix()
        x = x.copy(order='C').astype(np.float64)
        y = y_train.as_matrix().astype(np.float64)

        self.model = libsvm.fit(x, y, **parameter)

    def predictP(self, input):
        input = np.array(input).reshape(1, -1)
        result = libsvm.predict_proba(input,*(self.model))
        return result

    def predict(self, input):
        input = np.array(input)
        result = libsvm.predict(input, *(self.model))
        return result

    def modelEvaluate(self, x_test, y_test,path):
        start = datetime.datetime.now()
        pred_y = libsvm.predict(x_test.as_matrix().copy(order='C').astype(np.float64), *(self.model))
        end = datetime.datetime.now()
        print self.name + ' predicte time is ' + str(end - start)
        print 'crosstab:{0}'.format(pd.crosstab(y_test, pred_y))
        print 'accuracy_score:{0}'.format(accuracy_score(y_test, pred_y))
        print '0precision_score:{0}'.format(precision_score(y_test, pred_y, pos_label=0))
        print '0recall_score:{0}'.format(recall_score(y_test, pred_y, pos_label=0))
        print '1precision_score:{0}'.format(precision_score(y_test, pred_y, pos_label=1))
        print '1recall_score:{0}'.format(recall_score(y_test, pred_y, pos_label=1))
        fp = open(path + 'log.txt', 'a')
        fp.write('crosstab:{0}'.format(pd.crosstab(y_test, pred_y)) + '\n')
        fp.write('accuracy_score:{0}'.format(accuracy_score(y_test, pred_y)) + '\n')
        fp.write('0precision_score:{0}'.format(precision_score(y_test, pred_y, pos_label=0)) + '\n')
        fp.write('0recall_score:{0}'.format(recall_score(y_test, pred_y, pos_label=0)) + '\n')
        fp.write('1precision_score:{0}'.format(precision_score(y_test, pred_y, pos_label=1)) + '\n')
        fp.write('1recall_score:{0}'.format(recall_score(y_test, pred_y, pos_label=1)) + '\n')
        fp.close()