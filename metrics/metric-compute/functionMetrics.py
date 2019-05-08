import csv
import re
import time
from joern.all import JoernSteps
from symbol import Symbol
from link import Links
from priodict import priorityDictionary

def Dijkstra(G, start, end=None):
    D = {}  # dictionary of final distances
    P = {}  # dictionary of predecessors
    Q = priorityDictionary()  # est.dist. of non-final vert.
    Q[start] = 0
    for v in Q:
        D[v] = Q[v]
        if v == end: break

        for w in G[v]:
            vwLength = D[v] + G[v][w]
            if w in D:
                if vwLength < D[w]:
                    raise ValueError, \
                        "Dijkstra: found better path to already-final vertex"
            elif w not in Q or vwLength < Q[w]:
                Q[w] = vwLength
                P[w] = v
    return (D, P)

def shortestPath(G, start, end):
    D, P = Dijkstra(G, start, end)
    Path = []
    while 1:
        Path.append(end)
        if end == start: break
        if end in P:
            end = P[end]
        else:
            break
    Path.reverse()
    return Path


class FunctionMetrics(object):
    functions = []  # function node
    fileGlobals = {}
    conditionStms = {}  # condition nodes' AST parent node
    conditionStmsChildren = {}  # condition nodes' astNodes
    conditionnodeset=set()
    loopStms = {}  # loop statement
    fanIns={}
    fanOuts={}
    cfgNode = {}  # CFG node
    cfgEdge = {}  # CFG edge
    cdgedge={}
    symbol = {}  # symbol node
    BasicArithmetic = {} # + - * / % statement
    assignmentStms = {}  #
    shiftStms = {}  # shift statement
    IncDecOp = {}  # ++ --
    identifier = {}
    loopconditionNodesChildren = {}
    bitoperationNodes = {}  # & ^ |
    IncDecChildren = {}
    shiftstmsChildren = {}
    BitOperationChildren = {}
    BasicArithmeticChildren = {}
    ConditionIncreChildren = {}
    ConditionShiftChildren = {}
    ConditionBitopChildren = {}
    ConditionBasicArithmeticChildren = {}
    conditionuselist={}
    declStmsuse={}
    declStmsdefine={}

    identifierDecl = {}
    parameters = {}
    symbolReuse = {}
    symbolsInFunctionCall = {}
    argumentsInFunctionCall = {}
    symbolsInLibraryCall = {}
    argumentsInLibraryCall = {}
    returnDes = {}
    ptrMemberAccess = {}
    ptrMemberAccessSys = {}
    addressOps = {}
    addOps = {}
    dereferenceOps = {}
    dereferenceOpsSys = {}
    relationOps = {}

    taintSourceFunctionId = []
    taintSinkFunctionId = []
    taintSymbolToSource = {}
    taintSymbolToSink = {}
    parameterLists={}

    constants = {}
    castExpressions={}
    castApis={}
    j = None

    def __init__(self, functionId, fileNode):
        self.functionId = functionId
        self.fileNode = fileNode
        self.functionIdStr=str(self.functionId)

    def getFunctionId(self):
        return self.functionId

    def setFunctionName(self, functionName):
        self.functionName = functionName

    def setFilePath(self,filePath):       
        self.filePath=filePath

    def getFunctionName(self):
        return self.functionName

    def getFilePath(self):
        return self.filePath

    def getParameterList(self):
        if FunctionMetrics.parameterLists.has_key(self.functionIdStr):
            paralist=FunctionMetrics.parameterLists[self.functionIdStr][0]
            paralist=paralist.replace(',', ';') 
            return paralist
        else:            
            return ""

    def getFanIn(self):
        fanIn=0
        if FunctionMetrics.fanIns.has_key(self.functionName):
            fanIn=FunctionMetrics.fanIns[self.functionName]
        return fanIn

    def getFanOut(self):
        self.fanOut=[]
        if FunctionMetrics.fanOuts.has_key(self.functionIdStr):
            self.fanOut=FunctionMetrics.fanOuts[self.functionIdStr]
        return len(self.fanOut)

    def getVG(self):
        n = 0
        edgeres = []
        if FunctionMetrics.cfgNode.has_key(self.functionIdStr):
            n=FunctionMetrics.cfgNode[self.functionIdStr]
        if self.functionId in FunctionMetrics.cfgEdge:
            edgeres = FunctionMetrics.cfgEdge[self.functionId]
        e = len(edgeres)
        return (e - n + 2)

    def getStrictVG(self):
        return 1 + self.getNumPredicates()

    # predicates related metrics
    def setPredicates(self):
        self.predicates=[]
        if FunctionMetrics.conditionStms.has_key(self.functionIdStr):
            self.predicates = FunctionMetrics.conditionStms[self.functionIdStr]

    def getNumPredicates(self):
        return len(self.predicates)

    def getNestedPredicates(self):  # Attention! Should be executed before getNumNestedLoops()
        self.nestsEachP = {}  # Nest other
        self.nestedLevelEachP = {}  # Be nested
        self.isSurfaceDictP = {}
        self.isinnestDictP = {}
        self.constms_nests = {}  # can be reused for computing nestedLoops

        num = 0
        stms = self.predicates

        for i in stms:  # initialization
            self.nestsEachP[i] = 0
            self.isSurfaceDictP[i] = True
            self.nestedLevelEachP[i] = 1
            self.isinnestDictP[i] = 1
            self.constms_nests[i] = []
            if FunctionMetrics.conditionStmsChildren.has_key(i):
                self.constms_nests[i] = FunctionMetrics.conditionStmsChildren[i]

        for i in stms:
            nestOfI = self.constms_nests[i]
            for other in stms:
                if other != i and other in nestOfI:  # Find number of other statements as children of a specific statement
                    self.isSurfaceDictP[other] = False
                    self.nestsEachP[i] += 1
                    self.nestedLevelEachP[other] += 1
                    self.isinnestDictP[i] = 0
        for i in self.isSurfaceDictP:  # Sum up number of children of the outside statements
            if self.isSurfaceDictP[i]:
                num += self.nestsEachP[i]

        return num

    def getMaxNestedPre(self):
        numMaxNestedPre = 0
        if len(self.nestedLevelEachP) > 0:
            maxItem = max(self.nestedLevelEachP.items(),
                          key=lambda x: x[1])  # If emit the latter parameter, returns the max key
            numMaxNestedPre = maxItem[1]
        else:
            numMaxNestedPre = 0
        return numMaxNestedPre

    def getControlDependencyPrePath(self):
        cdgnodedict = {} #to determine whether the node is the end of the path
        graphdict = {} #to store the extracted cdg
        self.pathDictP = {}
        maxcontroldependencypre = 0
        edgestartset = set()
        edgeendset = set()
        searchstartnode = set()

        if int(self.functionId) not in FunctionMetrics.cdgedge:
            return self.pathDictP, maxcontroldependencypre

        cdgedgeperfunc = FunctionMetrics.cdgedge[int(self.functionId)]
        cfgentrynodeid=""
        for i in cdgedgeperfunc:
            if i[0] == u'CFGEntryNode':
                cfgentrynodeid = str(i[1])
                if  i[2] in FunctionMetrics.conditionnodeset:
                    if i[1] not in graphdict:
                        graphdict[i[1]] = {i[2]: 1}
                    else:
                        graphdict[i[1]].update({i[2]: 1})
                    edgestartset.add(i[1])
                    edgeendset.add(i[2])
                if i[2] not in FunctionMetrics.conditionnodeset:
                    if i[1] not in graphdict:
                        graphdict[i[1]] = {}

            if i[1] in FunctionMetrics.conditionnodeset and i[2] in FunctionMetrics.conditionnodeset:
                cdgnodedict[i[1]] = 1
                if i[1] not in graphdict:
                    graphdict[i[1]] = {i[2]: 1}
                else:
                    graphdict[i[1]].update({i[2]: 1})
                edgestartset.add(i[1])
                edgeendset.add(i[2])

            if i[1] in FunctionMetrics.conditionnodeset and i[2] not in FunctionMetrics.conditionnodeset:
                if i[1] not in cdgnodedict:
                    cdgnodedict[i[1]] = 0
                if i[1] not in graphdict:
                    graphdict[i[1]] = {}


        for i in edgestartset:
            if i not in edgeendset:
                searchstartnode.add(i)
            if i not in graphdict:
                graphdict[i]={}
        for i in edgeendset:
            if i not in graphdict:
                graphdict[i]={}
        # print searchstartnode

        for i in graphdict:
            if i == cfgentrynodeid:
                for key in graphdict[i]:
                    if key in FunctionMetrics.conditionnodeset and (key in graphdict and len(graphdict[key]>0)):
                        cdgnodedict[graphdict[i]] = 1

        # print cdgnodedict

        # print graphdict

        for i in cdgnodedict:
            if cdgnodedict[i] == 0:
                for start in searchstartnode:
                    path =shortestPath(graphdict, int(start), int(i))
                    if path:
                        if start==cfgentrynodeid:
                            temp = len(path) - 1
                        else:
                            temp=len(path)
                        if temp > maxcontroldependencypre:
                            maxcontroldependencypre = temp
                        if (temp in self.pathDictP):
                            self.pathDictP[temp] += 1
                        else:
                            self.pathDictP[temp] = 1

        return self.pathDictP,maxcontroldependencypre

    def getDefinedSymbol(self):
        self.definedsymbols = set()  # element in definedsymbols set has been defined
        self.finddefineDict = {}  # key:symbol name #value: datatype:pointer/array/user defined/basic type
        for i in self.symsInstances:
            self.definedsymbols.add(i.getSymbolName())
            self.definedsymbols=self.definedsymbols.union(set(i.getSymbolReuse()))
            self.finddefineDict[i.getSymbolName()] = i.getDataType()
            for ii in i.getSymbolReuse():
                self.finddefineDict[ii] = i.getDataType()

    def getDataDependencyPre(self):
        self.DataDepenDict = {}  # key:symbol value:condition node id
        self.symbolDict = {}  # key:symbol value:alias symbol name
        self.Dataresult = {}  # key:length of data dependency predicate value:occurrence
        conditionuse = []
        if FunctionMetrics.conditionuselist.has_key(self.functionId):
            conditionuse = FunctionMetrics.conditionuselist[self.functionId]
        declaliasdic = {}

        for i in self.definedsymbols:
            self.symbolDict[i] = [set()]  # symbolDict element 0: alias name; symbolDict is also used in getSymbolAttribute()
            self.DataDepenDict[i] = set()  # key: symbol name value:conditon node
        # print self.symbolDict
        # print self.definedsymbols

        for i in conditionuse:
            if i[1] in self.definedsymbols:
                self.DataDepenDict[i[1]].add(i[0])
        for i in conditionuse:
            if i[1] in self.definedsymbols:
                self.DataDepenDict[i[1]].add(i[0])

        self.symbolprepair = self.DataDepenDict  # self.symbolprepair is to compute the metrcis related to symbol and predicates

        declStmsuse=[]
        if FunctionMetrics.declStmsuse.has_key(self.functionId):
            declStmsuse=FunctionMetrics.declStmsuse[self.functionId]

        for i in declStmsuse:
            if i[1] in self.definedsymbols:
                if i[0] in declaliasdic:
                    declaliasdic[i[0]][0].append(i[1])
                else:
                    declaliasdic[i[0]] = [[i[1]], []]

        declStmsdefine=[]
        if FunctionMetrics.declStmsdefine.has_key(self.functionId):
            declStmsdefine=FunctionMetrics.declStmsdefine[self.functionId]

        for i in declStmsdefine:
            if i[1] in self.definedsymbols:
                if i[0] in declaliasdic:
                    declaliasdic[i[0]][1].append(i[1])
                else:
                    declaliasdic[i[0]] = [[], [i[1]]]        

        for i in declaliasdic:
            if len(declaliasdic[i][0])>0 and len(declaliasdic[i][1])>0:
                for use in declaliasdic[i][0]:
                    for ii in declaliasdic[i][1]:
                        self.symbolDict[ii][0].add(use)
                        self.DataDepenDict[ii] = self.DataDepenDict[ii].union(self.DataDepenDict[use])
                        self.symbolDict[use][0].add(ii)
                        self.DataDepenDict[use] = self.DataDepenDict[use].union(self.DataDepenDict[ii])

        for i in self.symsInstances:
            for ii in i.getSymbolReuse():
                if ii in self.DataDepenDict and i.getSymbolName() in self.DataDepenDict:
                    self.DataDepenDict[i.getSymbolName()] = self.DataDepenDict[i.getSymbolName()].union(
                        self.DataDepenDict[ii])
                    del self.DataDepenDict[ii]

        for i in self.symbolDict:
            if i in self.DataDepenDict:
                if len(self.symbolDict[i][0]) > 0:
                    for ii in self.symbolDict[i][0]:
                        if ii in self.DataDepenDict:
                            del self.DataDepenDict[ii]

        maxdatadependencypre=0

        for i in self.DataDepenDict:
            tempnum = len(self.DataDepenDict[i])
            if tempnum>maxdatadependencypre:
                maxdatadependencypre=tempnum
            if tempnum in self.Dataresult:
                self.Dataresult[tempnum] += 1
            else:
                self.Dataresult[tempnum] = 1
        return self.Dataresult,maxdatadependencypre

    def gettimesofaliasing(self):
        declStmsuse=[]
        if FunctionMetrics.declStmsuse.has_key(self.functionId):
            declStmsuse=FunctionMetrics.declStmsuse[self.functionId]
        return len(declStmsuse)

    def getconstant(self):
        strconstantsperfun=''
        constantsperfun=set()
        constants=[]
        if FunctionMetrics.constants.has_key(self.functionIdStr):
            constants=FunctionMetrics.constants[self.functionIdStr]
            # print constants
        for i in constants:
            if i.isalnum():
                constantsperfun.add(i)
        if len(constantsperfun)>0:
            for element in constantsperfun:
                strconstantsperfun += element + ';'
        else:
            strconstantsperfun = 'none'
        return strconstantsperfun


    def getIndependentPre(self):
        if 1 in self.pathDictP:
            independentpairnum = self.pathDictP[1]
            return independentpairnum
        else:
            return 0

    # loops related metrics
    def setLoops(self):
        self.loops = []
        if FunctionMetrics.loopStms.has_key(self.functionIdStr):
            self.loops = FunctionMetrics.loopStms[self.functionIdStr]

    def getNumLoops(self):
        return len(self.loops)

    def getNumNestedLoops(self):
        numNestedLoops = 0
        loopsnodeid = self.loops
        self.nestsEachL = {}  # Nest other
        self.nestedLevelEachL = {}  # Be nested
        self.isSurfaceDictL = {}

        for i in loopsnodeid:
            self.nestsEachL[i] = 0
            self.isSurfaceDictL[i] = True
            self.nestedLevelEachL[i] = 1

        for i in loopsnodeid:
            nestOfI=[]
            if self.constms_nests.has_key(i): #condition.parents can exclude some few cases..
                nestOfI = self.constms_nests[i]
            for other in loopsnodeid:
                if other != i and other in nestOfI:
                    self.isSurfaceDictL[other] = False
                    self.nestsEachL[i] += 1
                    self.nestedLevelEachL[other] += 1

        for i in self.isSurfaceDictL:
            if self.isSurfaceDictL[i]:
                numNestedLoops += self.nestsEachL[i]

        return numNestedLoops

    def getMaxNestedLoops(self):
        maxNestedLoops = 0
        if len(self.nestedLevelEachL) > 0:
            maxItem = max(self.nestedLevelEachL.items(),
                          key=lambda x: x[1])  # If emit the latter parameter, returns the max key
            maxNestedLoops = maxItem[1]
        else:
            maxNestedLoops = 0
        return maxNestedLoops

    # Vulnerable specific metrics and program analysis metrics
    def getSymbolAttribute(self):
        identifier = []
        if FunctionMetrics.identifier.has_key(self.functionIdStr):
            identifier = FunctionMetrics.identifier[self.functionIdStr]
        self.loopconditonchildrennode=[]
        if FunctionMetrics.loopconditionNodesChildren.has_key(self.functionIdStr):
            self.loopconditonchildrennode = FunctionMetrics.loopconditionNodesChildren[self.functionIdStr]

        for i in self.symbolset:
            iCode=i[u'code']
            if iCode in self.symbolDict:
                self.symbolDict[iCode].append(i[u'id'])  # symbolDict element 1: symbolnode id
                self.symbolDict[iCode].append(set())  # symbolDict element 2: identifier node id
                self.symbolDict[iCode][2] = [x for x in identifier if x[u'code'] == iCode]
                self.symbolDict[iCode].append(0)  # symbolDict element 3: loop num involved
                self.symbolDict[iCode].append(0)  # symbolDict element 4: arithmetic num involved
                self.symbolDict[iCode].append(0)  # symbolDict element 5: ++ -- involved
                self.symbolDict[iCode].append(0)  # symbolDict element 6: shift op involved
                self.symbolDict[iCode].append(0)  # symbolDict element 7: if loop var
                self.symbolDict[iCode].append(0)  # symbolDict element 8: bitop involved
            
        IncDecPerFunc=[]
        if FunctionMetrics.IncDecChildren.has_key(self.functionIdStr):
            IncDecPerFunc=FunctionMetrics.IncDecChildren[self.functionIdStr]                
        shiftstmsPerFunc=[]
        if FunctionMetrics.shiftstmsChildren.has_key(self.functionIdStr):
            shiftstmsPerFunc=FunctionMetrics.shiftstmsChildren[self.functionIdStr]        
        BitOperationPerFunc=[]
        if FunctionMetrics.BitOperationChildren.has_key(self.functionIdStr):
            BitOperationPerFunc=FunctionMetrics.BitOperationChildren[self.functionIdStr]        
        BasicArithmeticPerFunc=[]
        if FunctionMetrics.BasicArithmeticChildren.has_key(self.functionIdStr):
            BasicArithmeticPerFunc=FunctionMetrics.BasicArithmeticChildren[self.functionIdStr]
        
        for s in self.symbolDict:
            if len(self.symbolDict[s])<8:
                continue
            for ii in self.symbolDict[s][2]:
                identifiernodeid = ii[u'id']
                if identifiernodeid in BasicArithmeticPerFunc:
                    self.symbolDict[s][4] += 1
                if identifiernodeid in IncDecPerFunc:
                    self.symbolDict[s][5] += 1
                if identifiernodeid in shiftstmsPerFunc:
                    self.symbolDict[s][6] += 1
                if identifiernodeid in BitOperationPerFunc:
                    self.symbolDict[s][8] += 1
                if identifiernodeid in self.loopconditonchildrennode:
                    self.symbolDict[s][7] = 1

        loopvarnum = 0  # the number var involved in loop
        arithmenticvarnum = 0  # the number var involved in arithmetic expression
        increvarnum = 0  # the number var involved in ++/-- expression
        shiftvarnum = 0  # the number var involved in shift expression
        bitopvarnum = 0  # the number var involved in bit op expression

        for s in self.symbolDict:
            if len(self.symbolDict[s])<8:
                continue
            if self.symbolDict[s][7] == 1:
                loopvarnum += 1
            if self.symbolDict[s][4] > 0:
                arithmenticvarnum += 1
            if self.symbolDict[s][5] > 0:
                increvarnum += 1
            if self.symbolDict[s][6] > 0:
                shiftvarnum += 1
            if self.symbolDict[s][8] > 0:
                bitopvarnum += 1
        return self.symbolDict, self.symbolprepair, loopvarnum, arithmenticvarnum, increvarnum, shiftvarnum, bitopvarnum

    def getFuntionAttribute(self):
        shiftsPerFuncNum=0
        if FunctionMetrics.shiftStms.has_key(self.functionIdStr):
            shiftsPerFuncNum=len(FunctionMetrics.shiftStms[self.functionIdStr])
        lenElseArithmetic=0
        if FunctionMetrics.assignmentStms.has_key(self.functionIdStr):
            lenElseArithmetic=FunctionMetrics.assignmentStms[self.functionIdStr]        
        lenBasicArithmeticPerFunc=0
        if FunctionMetrics.BasicArithmetic.has_key(self.functionIdStr):
            lenBasicArithmeticPerFunc=len(FunctionMetrics.BasicArithmetic[self.functionIdStr])
        TotalArithmeticNum = lenElseArithmetic + lenBasicArithmeticPerFunc
        IncDecPerFuncNum=0
        if FunctionMetrics.IncDecOp.has_key(self.functionIdStr):
            IncDecPerFuncNum=len(FunctionMetrics.IncDecOp[self.functionIdStr])        
        BitOperationPerFuncNum=0        
        if FunctionMetrics.bitoperationNodes.has_key(self.functionIdStr):
            BitOperationPerFuncNum=len(FunctionMetrics.bitoperationNodes[self.functionIdStr])
        return TotalArithmeticNum, IncDecPerFuncNum, shiftsPerFuncNum, BitOperationPerFuncNum

    def getPredicateInvolvedVar(self):
        self.PreSymDict = {}  # key:Pre Id value:symbol name
        vartype = {}
        vartype['Pointer'] = 0
        vartype['Array'] = 0
        vartype['User defined'] = 0
        vartype['BasicType'] = 0
        temp = set() #to store the symbol id involved in the function
        typeset=[]
       
        for s in self.symbolprepair:
            if len(self.symbolprepair[s]) > 0:
                temp.add(s)
                type = self.finddefineDict[s]
                if type in vartype:
                    vartype[type] += 1
                else:
                    vartype['BasicType'] += 1
                for i in self.symbolprepair[s]:
                    if i in self.PreSymDict:
                        self.PreSymDict[i].add(s)
                    else:
                        self.PreSymDict[i] = set()
                        self.PreSymDict[i].add(s)
        # print self.PreSymDict
        for p in self.PreSymDict:
            tempsymbol=self.PreSymDict[p]
            tempset=set()
            for i in tempsymbol:
                stype=self.finddefineDict[i]
                tempset.add(stype)
            typeset.append(tempset)
        tempstrtypeset=str(typeset)
        print "old"+tempstrtypeset
        strtypeset=tempstrtypeset.replace(',',';')
        print "new"+strtypeset

        # print typeset
  
        return self.PreSymDict, len(temp), vartype['Pointer'], vartype['Array'], vartype['User defined'], vartype[
            'BasicType'],strtypeset

    def getPredicateInvolvedOp(self):
        FConditionIncre=0
        FConditionShift=0
        FConditionBitop=0
        FConditionBasicArithmetic=0
        if FunctionMetrics.ConditionIncreChildren.has_key(self.functionIdStr):
            FConditionIncre=FunctionMetrics.ConditionIncreChildren[self.functionIdStr]
        if FunctionMetrics.ConditionShiftChildren.has_key(self.functionIdStr):
            FConditionShift=FunctionMetrics.ConditionShiftChildren[self.functionIdStr]
        if FunctionMetrics.ConditionBitopChildren.has_key(self.functionIdStr):
            FConditionBitop=FunctionMetrics.ConditionBitopChildren[self.functionIdStr]     
        if FunctionMetrics.ConditionBasicArithmeticChildren.has_key(self.functionIdStr):
            FConditionBasicArithmetic=FunctionMetrics.ConditionBasicArithmeticChildren[self.functionIdStr]        
        return FConditionBasicArithmetic, FConditionIncre, FConditionShift, FConditionBitop

    def getLoopInvolvedVar(self):
        self.LoopSymDict = {}  # key:Loop condition node Id value:symbol name
        temp = set()
        vartype = {}
        vartype['Pointer'] = 0
        vartype['Array'] = 0
        vartype['User defined'] = 0
        vartype['BasicType'] = 0
        for i in self.symbolDict:
            if len(self.symbolDict[i])<8:
                continue
            if self.symbolDict[i][7] == 1:
                temp.add(i)
                type = self.finddefineDict[i]
                if type in vartype:
                    vartype[type] += 1
                else:
                    vartype['BasicType'] += 1
        return self.LoopSymDict, len(temp), vartype['Pointer'], vartype['Array'], vartype['User defined'], vartype[
            'BasicType']

    def setSymbols(self):
        self.symbolset = []
        if FunctionMetrics.symbol.has_key(self.functionIdStr):
            self.symbolset = FunctionMetrics.symbol[self.functionIdStr]
        self.symsInstances = []

    def setParameters(self):
        paras = []
        if FunctionMetrics.parameters.has_key(self.functionId):
            paras = FunctionMetrics.parameters[self.functionId]
        ptype = ""
        pname = ""
        for p in paras:  # Ordered already
            if p[1].properties[u'type'] == "ParameterType":
                ptypeCode = p[1].properties[u'code']
                ptype = Symbol.getSymbolDataType(ptypeCode)
            elif p[1].properties[u'type'] == "Identifier":
                pname = p[1].properties[u'code']
                aSymbol = Symbol(self.functionId, pname)
                aSymbol.setDataType(ptype)
                aSymbol.addOrigin("Parameter")
                self.symsInstances.append(aSymbol)

    def setIdentifierDecl(
            self):  # Preprocessors starts with ifndef, endif are regarded as IdentifierDeclType due to Joern's bugs. Set them aside now
        # Local variable should be declared before any operation. So just find the declaration involved.
        idfDecl=[]
        if FunctionMetrics.identifierDecl.has_key(self.functionId):
            idfDecl = FunctionMetrics.identifierDecl[self.functionId]        
        itype = ""
        iname = ""
        for i in idfDecl:
            if i[1].properties[u'type'] == "IdentifierDeclType":
                itypeCode = i[1].properties[u'code']
                itype = Symbol.getSymbolDataType(itypeCode)
            elif i[1].properties[u'type'] == "Identifier":
                iname = i[1].properties[u'code']
                aSymbol = Symbol(self.functionId, iname)
                aSymbol.setDataType(itype)
                aSymbol.addOrigin("Local")
                self.symsInstances.append(aSymbol)

    def mergeSymbols(self):  # Settle symbol reuse issue, as there are some symbols derived from one symbol
        symsDefined = [x.getSymbolName() for x in self.symsInstances]
        symsCallee = self.fanOut
        symsNotDefined = [x for x in self.symbolset if x[u'code'] not in symsDefined and x[u'code'] not in symsCallee]
        self.symsNotRelative = []
        for symbol in symsNotDefined:
            symbolName = symbol[u'code']
            symbolId = symbol[u'id']
            ids=[]
            if FunctionMetrics.symbolReuse.has_key(symbolId):
                ids=FunctionMetrics.symbolReuse[symbolId]
            idsRelative = []
            symbol_new = Symbol.removeArrowDot(symbolName)
            for i in ids:
                if i in symbol_new and symbolName != i:
                    idsRelative.append(i)
            if len(idsRelative) <= 0:
                self.symsNotRelative.append(symbolName)
                continue
            isRelative = False
            for ins in self.symsInstances:
                if ins.getSymbolName() in idsRelative:
                    ins.addSymbolReuse(symbolName)
                    isRelative = True
            if not isRelative:
                self.symsNotRelative.append(symbolName)

    def setGlobal(self):
        if FunctionMetrics.fileGlobals.has_key(self.fileNode):
            for globalNode in FunctionMetrics.fileGlobals[self.fileNode]:
                if globalNode[u'identifier'] in self.symsNotRelative:
                    aSymbol = Symbol(self.functionId, globalNode[u'identifier'])
                    gtype = Symbol.getSymbolDataType(globalNode[u'completeType'])
                    aSymbol.setDataType(gtype)
                    aSymbol.addOrigin("Global")
                    self.symsInstances.append(aSymbol)

    def setCallOrigin(self):
        # Symbols in the left side of the assignment operator
        sysInFunctionCall=[]
        if FunctionMetrics.symbolsInFunctionCall.has_key(self.functionIdStr):
            sysInFunctionCall=FunctionMetrics.symbolsInFunctionCall[self.functionIdStr]
        sysInLibraryCall=[]
        if FunctionMetrics.symbolsInLibraryCall.has_key(self.functionIdStr):
            sysInLibraryCall=FunctionMetrics.symbolsInLibraryCall[self.functionIdStr]
        for ins in self.symsInstances:
            if ins.getSymbolName() in sysInFunctionCall:
                ins.addOrigin("Function Call Return")
            elif ins.getSymbolName() in sysInLibraryCall:
                ins.addOrigin("Library Call Return")

    def setTaintSource(self):  # Not included now
        if self.functionId in FunctionMetrics.taintSourceFunctionId:
            self.isTaintSource = 1
        else:
            self.isTaintSource = 0

        intersec = [x for x in self.symbolset if FunctionMetrics.taintSymbolToSource.has_key(x[u'id'])]
        if len(intersec) > 0:
            for ins in self.symsInstances:
                for inter in intersec:
                    if ins.getSymbolName() == inter[u'code'] or inter[u'code'] in ins.getSymbolReuse():
                        ins.addOrigin("Taint Source")
                        #ins.setTaintSource(FunctionMetrics.taintSymbolToSource[inter[u'id']]) #Not used yet
        return self.isTaintSource

    def setTaintSink(self):
        if self.functionId in FunctionMetrics.taintSinkFunctionId:
            self.isTaintSink = 1
        else:
            self.isTaintSink = 0

        intersec = [x for x in self.symbolset if FunctionMetrics.taintSymbolToSink.has_key(x[u'id'])]
        if len(intersec) > 0:
            for ins in self.symsInstances:
                for inter in intersec:
                    if ins.getSymbolName() == inter[u'code'] or inter[u'code'] in ins.getSymbolReuse():
                        ins.addDestination("Taint Sink")
                        #ins.setTaintSink(FunctionMetrics.taintSymbolToSink[inter[u'id']])    #Not used yet
        return self.isTaintSink

    def setReturnDes(self):
        myReturnDes=set()
        if FunctionMetrics.returnDes.has_key(self.functionIdStr):
            myReturnDes=set(FunctionMetrics.returnDes[self.functionIdStr])
        for ins in self.symsInstances:
            if ins.getSymbolName() in myReturnDes:
                ins.addDestination("Return Value")

    def setArgumentDes(self):
        myArgumentsInFunctionCall=[]
        if FunctionMetrics.argumentsInFunctionCall.has_key(self.functionIdStr):
            myArgumentsInFunctionCall=FunctionMetrics.argumentsInFunctionCall[self.functionIdStr]
        myArgumentsInLibraryCall=[]
        if FunctionMetrics.argumentsInLibraryCall.has_key(self.functionIdStr):
            myArgumentsInLibraryCall=FunctionMetrics.argumentsInLibraryCall[self.functionIdStr]
        for ins in self.symsInstances:
            name = ins.getSymbolName()
            if name in myArgumentsInFunctionCall:
                ins.addDestination("Argument of Function")
            elif name in myArgumentsInLibraryCall:
                ins.addDestination("Argument of Library")
            else:
                reuse = ins.getSymbolReuse()
                intersection = [x for x in reuse if x in myArgumentsInFunctionCall]
                if len(intersection) > 0:
                    ins.addDestination("Argument of Function")
                intersection = [x for x in reuse if x in myArgumentsInLibraryCall]
                if len(intersection) > 0:
                    ins.addDestination("Argument of Library")

    def setPointerInvolved(self):        
        myPtrMemberAccess=0
        if FunctionMetrics.ptrMemberAccess.has_key(self.functionIdStr):
            myPtrMemberAccess=FunctionMetrics.ptrMemberAccess[self.functionIdStr]        
        myPtrMemberAccessSys=[]
        if FunctionMetrics.ptrMemberAccessSys.has_key(self.functionIdStr):
            myPtrMemberAccessSys=FunctionMetrics.ptrMemberAccessSys[self.functionIdStr]        
        myAddressOps=[]
        if FunctionMetrics.addressOps.has_key(self.functionId):
            myAddressOps=FunctionMetrics.addressOps[self.functionId]        
        myDereferenceOps=0
        if FunctionMetrics.dereferenceOps.has_key(self.functionIdStr):
            myDereferenceOps=FunctionMetrics.dereferenceOps[self.functionIdStr]        
        myDereferenceOpsSys=[]
        if FunctionMetrics.dereferenceOpsSys.has_key(self.functionIdStr):
            myDereferenceOpsSys=FunctionMetrics.dereferenceOpsSys[self.functionIdStr]        
        # maybe can reuse the other's
        addOps=[]
        if FunctionMetrics.addOps.has_key(self.functionId):
            addOps=FunctionMetrics.addOps[self.functionId]        
        incDecOps=[]
        if FunctionMetrics.IncDecOp.has_key(self.functionIdStr):
            incDecOps=FunctionMetrics.IncDecOp[self.functionIdStr]        
        relationOps = []
        if FunctionMetrics.relationOps.has_key(self.functionId):
            relationOps=FunctionMetrics.relationOps[self.functionId]
        
        self.numPointerOps = myPtrMemberAccess
        self.numPointerOps += myDereferenceOps
        
        if len(myPtrMemberAccessSys) > 0:
            sysPtrDict = {}
            for i in myPtrMemberAccessSys:
                tmp = i.split(' ')
                if '->' in tmp or '[' in tmp:  # There are corresponding individual symbols for symbols in these formats. So just emit the duplicates
                    continue
                name = tmp[len(tmp) - 1]
                if sysPtrDict.has_key(name):
                    sysPtrDict[name] += 1
                else:
                    sysPtrDict[name] = 1
            if len(sysPtrDict) > 0:
                for sys in self.symsInstances:
                    if sysPtrDict.has_key(sys.getSymbolName()):
                        sys.setNumPtrArith(sys.getNumPtrArith() + sysPtrDict[sys.getSymbolName()])

        numMyAddressOps = 0
        addressOpIdFormer = ""
        for i in myAddressOps:
            addressOpId = i[0].ref
            if addressOpId != addressOpIdFormer:
                numMyAddressOps += 1
                addressOpIdFormer = addressOpId
            addressOpCode_new = Symbol.removeArrowDot(
                i[0].properties[u'code'])  # Just for a few cases involving -> or .
            if i[1] in addressOpCode_new:
                for sys in self.symsInstances:
                    if sys.getSymbolName() == i[1]:
                        sys.setNumPtrArith(sys.getNumPtrArith() + 1)
        self.numPointerOps += numMyAddressOps
        
        if len(myDereferenceOpsSys) > 0:
            sysPtrDict = {}
            for i in myDereferenceOpsSys:
                tmp = i.split(' ')
                if '->' in tmp or '[' in tmp or '(' in tmp:  # The same with ptrMemberAccess
                    continue
                name = tmp[len(tmp) - 1]
                if sysPtrDict.has_key(name):
                    sysPtrDict[name] += 1
                else:
                    sysPtrDict[name] = 1
            if len(sysPtrDict) > 0:
                for sys in self.symsInstances:
                    if sysPtrDict.has_key(sys.getSymbolName()):
                        sys.setNumPtrArith(sys.getNumPtrArith() + sysPtrDict[sys.getSymbolName()])

        addOpIdFormer = ""
        idsFormer = []
        for i in addOps:
            addOpId = i[0].ref
            if addOpId != addOpIdFormer:
                intersecPointer = [x for x in self.symsInstances if
                                   x.getSymbolName() in idsFormer and x.getDataType() == "Pointer"]
                if len(intersecPointer) > 0:  # At least one of the identifiers is Pointer
                    self.numPointerOps += 1
                    intersec = [x for x in self.symsInstances if x.getSymbolName() in idsFormer]
                    for sys in intersec:
                        sys.setNumPtrArith(sys.getNumPtrArith() + 1)
                addOpIdFormer = addOpId
                idsFormer = []
            idsFormer.append(i[1])

        for iname in incDecOps:
            # iname = i[u'code']
            inameSet = iname.split(' ')
            if '*' in inameSet or '[' in inameSet or '->' in inameSet:  # Already used in dereference or ptrmemberaccess, or related to arrays
                continue
            if '(' in inameSet:  # just for few cases like (i)++, sounds unnecessary,but sometimes may happen
                inameSet.remove('(')
                inameSet.remove(')')
            if len(inameSet) == 2:
                if inameSet[0] == '++' or inameSet[0] == '--':
                    nameIndex = 1
                else:
                    nameIndex = 0
                for sys in self.symsInstances:
                    if sys.getSymbolName() == inameSet[nameIndex] and sys.getDataType() == "Pointer":
                        sys.setNumPtrArith(sys.getNumPtrArith() + 1)
                        self.numPointerOps += 1
        # Operands of arrayIndex or function call will be excluded, someother bugs like cast(xx*),assignment
        relationOpIdFormer = ""
        idsFormer = []
        for i in relationOps:  # seems like similar with addops
            relationOpId = i[0].ref
            if relationOpId != relationOpIdFormer:
                intersecPointer = [x for x in self.symsInstances if
                                   x.getSymbolName() in idsFormer and x.getDataType() == "Pointer"]
                if len(intersecPointer) > 0:  # At least one of the identifiers is Pointer
                    self.numPointerOps += 1
                    # print "addRelation",i
                    intersec = [x for x in self.symsInstances if x.getSymbolName() in idsFormer]
                    for sys in intersec:
                        sys.setNumPtrArith(sys.getNumPtrArith() + 1)
                relationOpIdFormer = relationOpId
                idsFormer = []
            idsFormer.append(i[1])
        
        return self.numPointerOps

    def getSymsIns(self):
        return self.symsInstances

    def getCastOps(self):
        numCastExpressions = 0
        if self.functionIdStr in FunctionMetrics.castExpressions:
            numCastExpressions = FunctionMetrics.castExpressions[self.functionIdStr]        
        numCastApis = 0
        if self.functionIdStr in FunctionMetrics.castApis:
            numCastApis = FunctionMetrics.castApis[self.functionIdStr]                
        return numCastExpressions,numCastApis


    def run(self):
        filePath=self.getFilePath()        
        parameterList=self.getParameterList()        
        # Complexity related
        self.setPredicates()
        self.setLoops()

        vg = self.getVG()
        strictVG = self.getStrictVG()
        fanIn = self.getFanIn()
        fanOut = self.getFanOut()

        predicatenum = self.getNumPredicates()
        numNestedPre = self.getNestedPredicates()
        numMaxNestedPre = self.getMaxNestedPre()
        pathDictP,maxcontroldependencypre = self.getControlDependencyPrePath()
        independentpairnum = self.getIndependentPre()

        numLoops = self.getNumLoops()
        numNestedLoops = self.getNumNestedLoops()
        numMaxNestedLoops = self.getMaxNestedLoops()

        # Symbol related
        self.setSymbols()
        self.setParameters()
        self.setIdentifierDecl()
        self.mergeSymbols()
        self.setGlobal()
        self.setCallOrigin()
        self.setTaintSource()
        self.setReturnDes()
        self.setArgumentDes()

        self.getDefinedSymbol()  # !!!
        datadependresult,maxdatadependencypre = self.getDataDependencyPre() #!!!
        aliasingtimes = self.gettimesofaliasing()
        constantsperfun=self.getconstant()

        TotalArithmeticNum, IncDecPerFuncNum, shiftsPerFuncNum, BitOpPerFuncNum = self.getFuntionAttribute()
        sDict, spDict, loopvarnum, arithmenticvarnum, increvarnum, shiftvarnum, bitopvarnum = self.getSymbolAttribute()
        PDict, prevarnumber, vprePointer, vpreArray, vpreUserdefine, vpreBasicType,PredicateInvolvedVartypeset = self.getPredicateInvolvedVar()
        LDict, loopvarnumber, vloopPointer, vloopArray, vloopUserdefine, vloopBasicType = self.getLoopInvolvedVar()
        ConditionBasicArithmetic, ConditionIncre, ConditionShift, ConditionBitop = self.getPredicateInvolvedOp()

        isTaintSource = self.setTaintSource()
        isTaintSink = self.setTaintSink()
        numTaintSouceVar = 0
        numTaintSinkVar = 0
        numPointerArith = self.setPointerInvolved()
        numPointerVar = 0
        numArrayVar = 0
        numUserDefinedVar = 0
        numBasicTypeVar = 0
        numParameterVar = 0
        numLocalVar = 0
        numGlobalVar = 0
        numFunctionReturnVar = 0
        numLibraryReturnVar = 0
        numFunctionArgumentVar = 0
        numLibraryArgumentVar = 0
        numReturnInvolvedVar = 0
        numPointerArithInvolvedVar = 0
        maxTimesPointerArithInvolvedVar = 0
        avgTimesPointerArithInvolvedVar = 0

        sysins = self.getSymsIns()
        totalTimesPointerArithInvolvedVar = 0
        for sys in sysins:
            #print sys
            if sys.getDataType() == "Pointer":
                numPointerVar += 1
            elif sys.getDataType() == "Array":
                numArrayVar += 1
            elif sys.getDataType() == "User defined":
                numUserDefinedVar += 1
            else:
                numBasicTypeVar += 1

            origins = sys.getSymbolOrigin()
            if "Parameter" in origins:
                numParameterVar += 1
            elif "Local" in origins:
                numLocalVar += 1
            elif "Global" in origins:
                numGlobalVar += 1
            if "Function Call Return" in origins:
                numFunctionReturnVar += 1
            if "Library Call Return" in origins:
                numLibraryReturnVar += 1

            destinations = sys.getSymbolDestination()
            if "Return Value" in destinations:
                numReturnInvolvedVar += 1
            if "Argument of Function" in destinations:
                numFunctionArgumentVar += 1
            if "Argument of Library" in destinations:
                numLibraryArgumentVar += 1

            numPtrArith = sys.getNumPtrArith()
            if numPtrArith > 0:
                numPointerArithInvolvedVar += 1
                totalTimesPointerArithInvolvedVar += numPtrArith
                if numPtrArith > maxTimesPointerArithInvolvedVar:
                    maxTimesPointerArithInvolvedVar = numPtrArith

            if "Taint Source" in sys.getSymbolOrigin():
                numTaintSouceVar += 1
            if "Taint Sink" in sys.getSymbolDestination():
                numTaintSinkVar += 1

        if len(sysins) > 0:
            avgTimesPointerArithInvolvedVar = totalTimesPointerArithInvolvedVar / len(sysins)

        numCastExpressions,numCastApis = self.getCastOps()

        sysData = [self.functionId, self.functionName,TotalArithmeticNum, IncDecPerFuncNum, shiftsPerFuncNum,
                   BitOpPerFuncNum, numPointerVar, numArrayVar, numUserDefinedVar, numBasicTypeVar, numParameterVar,
                   numLocalVar, \
                   numGlobalVar, arithmenticvarnum, increvarnum, shiftvarnum, bitopvarnum, prevarnumber, vprePointer,
                   vpreArray, vpreUserdefine, vpreBasicType, ConditionBasicArithmetic, ConditionIncre, ConditionShift,
                   ConditionBitop, \
                   loopvarnumber, vloopPointer, vloopArray, vloopUserdefine, vloopBasicType, numPointerArith,
                   numFunctionReturnVar, numLibraryReturnVar, numFunctionArgumentVar, numLibraryArgumentVar,
                   numReturnInvolvedVar, numPointerArithInvolvedVar]
        

        complexityData = [vg, strictVG, fanIn, fanOut, numLoops, numNestedLoops, predicatenum,
                          numNestedPre,  independentpairnum]
        #test
        # addeddata=[constantsperfun,aliasingtimes,PredicateInvolvedVartypeset,numCastExpressions,numCastApis]

        sysData.extend(complexityData)
        # test
        # sysData.extend(addeddata)

        return sysData, maxcontroldependencypre, pathDictP, maxdatadependencypre, datadependresult

    @classmethod
    def clearDict(cls):        
        FunctionMetrics.conditionStms = {}  # condition nodes' AST parent node
        FunctionMetrics.conditionStmsChildren = {}  # condition nodes' astNodes
        FunctionMetrics.loopStms = {}  # loop statement        
        FunctionMetrics.fanOuts={}
        FunctionMetrics.cfgNode = {}  # CFG node
        FunctionMetrics.cfgEdge = {}  # CFG edge
        FunctionMetrics.cdgedge = {}
        FunctionMetrics.symbol = {}  # symbol node
        FunctionMetrics.BasicArithmetic = {} # + - * / % statement
        FunctionMetrics.assignmentStms = {}  #
        FunctionMetrics.shiftStms = {}  # shift statement
        FunctionMetrics.IncDecOp = {}  # ++ --
        FunctionMetrics.identifier = {}
        FunctionMetrics.loopconditionNodesChildren = {}
        FunctionMetrics.bitoperationNodes = {}  # & ^ |
        FunctionMetrics.IncDecChildren = {}
        FunctionMetrics.shiftstmsChildren = {}
        FunctionMetrics.BitOperationChildren = {}
        FunctionMetrics.BasicArithmeticChildren = {}
        FunctionMetrics.ConditionIncreChildren = {}
        FunctionMetrics.ConditionShiftChildren = {}
        FunctionMetrics.ConditionBitopChildren = {}
        FunctionMetrics.ConditionBasicArithmeticChildren = {}
        FunctionMetrics.conditionuselist={}
        FunctionMetrics.declStmsuse={}
        FunctionMetrics.declStmsdefine={}

        FunctionMetrics.identifierDecl = {}
        FunctionMetrics.parameters = {}
        FunctionMetrics.symbolReuse = {}        
        FunctionMetrics.returnDes = {}
        FunctionMetrics.ptrMemberAccess = {}
        FunctionMetrics.ptrMemberAccessSys = {}
        FunctionMetrics.addressOps = {}
        FunctionMetrics.addOps = {}
        FunctionMetrics.dereferenceOps = {}
        FunctionMetrics.dereferenceOpsSys = {}
        FunctionMetrics.relationOps = {}
        FunctionMetrics.parameterLists={}
        FunctionMetrics.constants={}

        FunctionMetrics.fileGlobals={}
        FunctionMetrics.fanIns={}

        FunctionMetrics.taintSymbolToSource = {}
        FunctionMetrics.taintSymbolToSink = {}
        FunctionMetrics.taintSourceFunctionId = []
        FunctionMetrics.taintSinkFunctionId = []

        FunctionMetrics.castExpressions={}
        FunctionMetrics.castApis={}

if __name__ == '__main__':
    j = JoernSteps()
    j.setGraphDbURL('http://localhost:7474/db/data/')
    # j.addStepsDir('Use this to inject custom steps')
    j.connectToDatabase()
    
    #Global view    
    query = """getNodesWithType("Condition").id"""
    FunctionMetrics.conditionnodeset = set(j.runGremlinQuery(query))

    query = """getNodesWithType("Function").map('id','name')"""
    funcIdsNames = j.runGremlinQuery(query)   
    funcIds = [x[u'id'] for x in funcIdsNames]
    ownFuncName = [str(x[u'name']) for x in funcIdsNames]
    del funcIdsNames    
    
    query = """getNodesWithType("File").out("IS_FILE_OF").filter{it.type=="Class" && it.name!="<unnamed>"}.name"""
    nameList = j.runGremlinQuery(query)
    Symbol.setStructName(nameList)
    del nameList

    query = """getNodesWithType("Callee").id"""
    calleeIds = j.runGremlinQuery(query)    
    ownCallee = []
    libCallee = []
    ownCalleeDict = {}
    libCalleeDict = {}
    CHUNK_SIZE = 256
    for chunk in j.chunks(ownFuncName, CHUNK_SIZE):
        nameTuple=tuple(chunk)
        nameStr=str(nameTuple)
        nameStr=nameStr.replace(',','')
        nameStr=nameStr.replace('\'','')    
        query = """queryNodeIndex("type:Callee AND code:%s").id"""%nameStr
        res = j.runGremlinQuery(query)
        ownCallee.extend(res)    
        for rchunk in j.chunks(res,CHUNK_SIZE):
            query = """idListToNodes(%s).groupBy{it.functionId}{it.id}.cap"""%rchunk
            resDict = j.runGremlinQuery(query)[0]
            for (funcId,ids) in resDict.items():
                funcIdint = int(funcId)
                if funcIdint in ownCalleeDict:
                    ownCalleeDict[funcIdint].extend(resDict[funcId])
                else:
                    ownCalleeDict[funcIdint] = resDict[funcId]    

    libCallee=list(set(calleeIds)-set(ownCallee))
    for chunk in j.chunks(libCallee, CHUNK_SIZE):
        query="""idListToNodes(%s).groupBy{it.functionId}{it.id}.cap"""%chunk
        resDict= j.runGremlinQuery(query)[0]
        for (funcId,ids) in resDict.items():
            funcIdint = int(funcId)
            if funcIdint in libCalleeDict:
                libCalleeDict[funcIdint].extend(resDict[funcId])
            else:
                libCalleeDict[funcIdint] = resDict[funcId]

    del ownFuncName
    del calleeIds
    del ownCallee
    del libCallee    

    taintlink = Links(j)
    taintSourceFuncCallee = taintlink.getLinksToSource() 
    taintSinkFuncCallee = taintlink.getLinksToSink() #sinks:6mins for Linux kernel        
    del taintlink

    #Used inside the loop  & Referenced <cstdlib> <cuchar> <cwchar> plus what Mahin gave
    castApiNames = ('atoi','atof','atol','atoll','mbtowc','mbstowcs','wcstombs','wctomb','strtod','strtol','strtof','strtold','stroll','strtoul','strtoull',\
    'itoa','c16rtomb','c32rtomb','mbrtoc16','mbrtoc32','wcstod','wcstof','wcstol','wcstold','wcstoll','wcstoul','wcstoull','btowc','mbrtowc','mbsrtowcs','wcrtomb',\
    'wctob','wcsrtombs','strongswan_atoul','strongswan_ttoul','strtonum','strtoumax','uni2ascii','l64a','ascii2uni') 
    castApisStr = str(castApiNames)
    castApisStr=castApisStr.replace(',','')
    castApisStr=castApisStr.replace('\'','')
    del castApiNames

    #Chunk of functions    
    CHUNK_SIZE = 256 ###
    for chunk in j.chunks(funcIds, CHUNK_SIZE):     
        FunctionMetrics.clearDict()

        # functionIdsList = [int(x[0].ref[5:]) for x in chunk]        
        functionIdsTuple = tuple(chunk)
        functionIds = str(functionIdsTuple)        
        functionIds = functionIds.replace(',', '')       
        del functionIdsTuple

        query = """idListToNodes(%s).as("x").in("IS_FILE_OF").as("y").select{it}{it}"""%(chunk)
        functions = j.runGremlinQuery(query)                  
           
        query = """idListToNodes(%s).in("IS_FILE_OF").as("x").out("IS_FILE_OF").filter{it.type=="Class"}.out("IS_CLASS_OF").out("DECLARES").as("y").select{it.id}{it}"""%(chunk)                        
        fileGlobalList = j.runGremlinQuery(query)        
        for fg in fileGlobalList:
            fileId = fg[0]
            if fileId not in FunctionMetrics.fileGlobals:        
                FunctionMetrics.fileGlobals[fileId] = []
                FunctionMetrics.fileGlobals[fileId].append(fg[1])        
            else:
                FunctionMetrics.fileGlobals[fileId].append(fg[1])
        del fileGlobalList        

        funcNameList=[str(x[0].properties[u'name']) for x in functions]
        funcNameTuple=tuple(funcNameList)
        funcNameStr=str(funcNameTuple)          
        funcNameStr=funcNameStr.replace(',','')
        funcNameStr=funcNameStr.replace('\'','')
        query = """queryNodeIndex("type:Callee AND code:%s").groupBy{it.code}{it}{it.size()}.cap"""%(funcNameStr)   
        FunctionMetrics.fanIns=j.runGremlinQuery(query)[0]            
        del funcNameList
        del funcNameTuple
        del funcNameStr

        cownCallee=[]
        clibCallee=[]
        for c in chunk:
            if c in ownCalleeDict:
                cownCallee.extend(ownCalleeDict[c])
                del ownCalleeDict[c]
            if c in libCalleeDict:
                clibCallee.extend(libCalleeDict[c])
                del libCalleeDict[c]
        query = """idListToNodes(%s).parents().loop(1){true}{true}.filter{it.type=="AssignmentExpr"}.out("DEF").groupBy{it.functionId}{it.code}.cap""" % (cownCallee)
        FunctionMetrics.symbolsInFunctionCall = j.runGremlinQuery(query)[0]       
        query = """idListToNodes(%s).parents().children().filter{it.type=="ArgumentList"}.children().out("USE").groupBy{it.functionId}{it.code}.cap""" % (cownCallee)
        FunctionMetrics.argumentsInFunctionCall=j.runGremlinQuery(query)[0]
        query = """idListToNodes(%s).parents().loop(1){true}{true}.filter{it.type=="AssignmentExpr"}.out("DEF").groupBy{it.functionId}{it.code}.cap""" % (clibCallee)
        FunctionMetrics.symbolsInLibraryCall = j.runGremlinQuery(query)[0]
        query = """idListToNodes(%s).parents().children().filter{it.type=="ArgumentList"}.children().out("USE").groupBy{it.functionId}{it.code}.cap""" % (clibCallee)
        FunctionMetrics.argumentsInLibraryCall = j.runGremlinQuery(query)[0]
        del cownCallee
        del clibCallee        

        csourceCallee=[]
        csinkCallee=[]
        for c in chunk:        
            if c in taintSourceFuncCallee:                
                FunctionMetrics.taintSourceFunctionId.append(c)
                csourceCallee.extend(taintSourceFuncCallee[c])
                del taintSourceFuncCallee[c]
            if c in taintSinkFuncCallee:
                FunctionMetrics.taintSinkFunctionId.append(c)
                csinkCallee.extend(taintSinkFuncCallee[c])
                del taintSinkFuncCallee[c]

        query = """idListToNodes(%s).as("x").parents().loop(1){true}{true}.filter{it.type=="AssignmentExpr"}.out("DEF").id.as("y").select{it.getProperty("code")}{it}""" % csourceCallee
        symbolSource = j.runGremlinQuery(query)
        for ss in symbolSource:
            if FunctionMetrics.taintSymbolToSource.has_key(ss[1]):
                FunctionMetrics.taintSymbolToSource[ss[1]].append(ss[0])
            else:
                FunctionMetrics.taintSymbolToSource[ss[1]] = []
                FunctionMetrics.taintSymbolToSource[ss[1]].append(ss[0])
    
        query = """idListToNodes(%s).as("x").parents().children().filter{it.type=="ArgumentList"}.children().out("USE").id.as("y").select{it.getProperty("code")}{it}""" % csinkCallee
        symbolSink = j.runGremlinQuery(query)
        for ss in symbolSink:
            if FunctionMetrics.taintSymbolToSink.has_key(ss[1]):
                FunctionMetrics.taintSymbolToSink[ss[1]].append(ss[0])
            else:
                FunctionMetrics.taintSymbolToSink[ss[1]] = []
                FunctionMetrics.taintSymbolToSink[ss[1]].append(ss[0]) 
        del symbolSource
        del symbolSink
        del csourceCallee
        del csinkCallee        

        query = """queryNodeIndex("functionId:%s AND isCFGNode:True").groupBy{it.functionId}{it.id}{it.size()}.cap"""%functionIds          
        FunctionMetrics.cfgNode = (j.runGremlinQuery(query))[0]
        query = """queryNodeIndex("functionId:%s AND isCFGNode:True").as("x").outE("FLOWS_TO").id.as("y").select{it.functionId}{it}"""%functionIds         
        cfgEdge = j.runGremlinQuery(query)
        for i in cfgEdge:
            if i[0] in FunctionMetrics.cfgEdge:
                FunctionMetrics.cfgEdge[i[0]].add(i[1])
            else:
                FunctionMetrics.cfgEdge[i[0]] = set()
                FunctionMetrics.cfgEdge[i[0]].add(i[1])
        del cfgEdge
        
        query = """queryNodeIndex("functionId:%s AND isCFGNode:True").as("x").outE("CONTROLS").as("y").select{it}{it}"""%functionIds
        cdgedge = j.runGremlinQuery(query)        
        for i in cdgedge:
            functionid = i[0].properties[u'functionId']
            temp = [i[0].properties[u'type'], i[1].start_node._id, i[1].end_node._id]
            if functionid in FunctionMetrics.cdgedge:
                FunctionMetrics.cdgedge[functionid].append(temp)
            else:
                FunctionMetrics.cdgedge[functionid] = [temp]
        del cdgedge
        
        query = """OR(queryNodeIndex("functionId:%s AND type:Condition").in("IS_AST_PARENT"),queryNodeIndex("functionId:%s AND type:ForStatement")).dedup().groupBy{it.functionId}{it.id}.cap"""%(functionIds,functionIds)
        FunctionMetrics.conditionStms = j.runGremlinQuery(query)[0]        
        query = """OR(queryNodeIndex("functionId:%s AND type:Condition").in("IS_AST_PARENT"),queryNodeIndex("functionId:%s AND type:ForStatement")).dedup().id"""%(functionIds,functionIds)
        conditionStmsId = j.runGremlinQuery(query)                    
        query = """idListToNodes(%s).as("x").astNodes().as("y").select{it.id}{it.id}""" % (conditionStmsId)
        conditionStmsChildren = j.runGremlinQuery(query)            
        for i in conditionStmsChildren:
            cid = i[0]
            if cid not in FunctionMetrics.conditionStmsChildren:
                FunctionMetrics.conditionStmsChildren[cid] = []
                FunctionMetrics.conditionStmsChildren[cid].append(i[1])                
            else:
                FunctionMetrics.conditionStmsChildren[cid].append(i[1])      
        del conditionStmsChildren

        query = """queryNodeIndex("functionId:%s AND type:Condition").as("x").uses().code.as("y").select{it}{it}"""%functionIds#InfiniteForNode won't use any symbols
        conditionuselist = j.runGremlinQuery(query)        
        for i in conditionuselist:
            funcId = i[0].properties[u'functionId']
            if funcId not in FunctionMetrics.conditionuselist:
                FunctionMetrics.conditionuselist[funcId] = []
                FunctionMetrics.conditionuselist[funcId].append([int(i[0].ref[5:]), i[1]])                
            else:
                FunctionMetrics.conditionuselist[funcId].append([int(i[0].ref[5:]), i[1]])
        del conditionuselist
        
        query = """queryNodeIndex("functionId:%s AND type:(ForStatement WhileStatement DoStatement)").groupBy{it.functionId}{it.id}.cap"""%functionIds        
        FunctionMetrics.loopStms = j.runGremlinQuery(query)[0]
        
        query = """queryNodeIndex("functionId:%s AND type:(ForStatement WhileStatement DoStatement)").children().filter{it.type=="Condition"}.astNodes().groupBy{it.functionId}{it.id}.cap"""%functionIds
        FunctionMetrics.loopconditionNodesChildren = j.runGremlinQuery(query)[0]
        
        query = """queryNodeIndex("functionId:%s AND type:Condition").astNodes().filter{it.type=="UnaryExpression" ||it.type=="IncDecOp"}.groupBy{it.functionId}{it.id}{it.size()}.cap"""%functionIds
        FunctionMetrics.ConditionIncreChildren = j.runGremlinQuery(query)[0]
        
        query = """queryNodeIndex("functionId:%s AND type:Condition").astNodes().filter{it.type=="ShiftExpression"}.groupBy{it.functionId}{it.id}{it.size()}.cap"""%functionIds
        FunctionMetrics.ConditionShiftChildren = j.runGremlinQuery(query)[0]
        
        query = """queryNodeIndex("functionId:%s AND type:Condition").astNodes().filter{it.type=="BitAndExpression" || it.type=="InclusiveOrExpression" ||it.type=="ExclusiveOrExpression" }.groupBy{it.functionId}{it.id}{it.size()}.cap"""%functionIds
        FunctionMetrics.ConditionBitopChildren = j.runGremlinQuery(query)[0]
        
        query = """queryNodeIndex("functionId:"""+functionIds+""" AND type:Condition").astNodes().filter{it.type=="AssignmentExpr" && it.operator in ["+=","-=","*=","/=","%="]}.groupBy{it.functionId}{it.id}{it.size()}.cap"""#%functionIds        
        ConditionAssignChildren1 = j.runGremlinQuery(query)[0]
        query = """queryNodeIndex("functionId:%s AND type:Condition").astNodes().filter{it.type=="AdditiveExpression" || it.type=="MultiplicativeExpression"}.groupBy{it.functionId}{it.id}{it.size()}.cap"""%functionIds
        ConditionAssignChildren2 = j.runGremlinQuery(query)[0]
        FunctionMetrics.ConditionBasicArithmeticChildren = ConditionAssignChildren1
        for (funcId, childrenNum) in FunctionMetrics.ConditionBasicArithmeticChildren.items():
            if ConditionAssignChildren2.has_key(funcId):
                FunctionMetrics.ConditionBasicArithmeticChildren[funcId] += ConditionAssignChildren2[funcId]
                del ConditionAssignChildren2[funcId]
        FunctionMetrics.ConditionBasicArithmeticChildren = dict(FunctionMetrics.ConditionBasicArithmeticChildren, **ConditionAssignChildren2)        
        del ConditionAssignChildren2
    
        # Metrics of Second series    
        query = """queryNodeIndex("functionId:%s AND type:(MultiplicativeExpression AdditiveExpression)").groupBy{it.functionId}{it.id}.cap"""%functionIds
        FunctionMetrics.BasicArithmetic = j.runGremlinQuery(query)[0]
        
        query = """queryNodeIndex("functionId:%s AND type:AdditiveExpression").as("x").children().filter{it.type=="Identifier"}.code.as("y").select"""%functionIds
        addOps = j.runGremlinQuery(query)               
        for i in addOps:
            funcId = i[0].properties[u'functionId']
            if funcId not in FunctionMetrics.addOps:
                FunctionMetrics.addOps[funcId] = []
                FunctionMetrics.addOps[funcId].append(i)                
            else:
                FunctionMetrics.addOps[funcId].append(i)
        del addOps
        
        query = """queryNodeIndex("functionId:"""+functionIds+""" AND type:AssignmentExpr").filter{it.operator in ["+=","-=","*=","/=","%="]}.groupBy{it.functionId}{it.id}{it.size()}.cap"""#%functionIds        
        FunctionMetrics.assignmentStms = j.runGremlinQuery(query)[0]        
        
        query = """queryNodeIndex("functionId:%s AND type:ShiftExpression").groupBy{it.functionId}{it.id}.cap"""%functionIds
        FunctionMetrics.shiftStms = j.runGremlinQuery(query)[0]
        
        query = """queryNodeIndex("functionId:%s AND type:(UnaryExpression IncDecOp)").groupBy{it.functionId}{it.code}.cap"""%functionIds  # ++ --
        FunctionMetrics.IncDecOp = j.runGremlinQuery(query)[0]
        
        query = """queryNodeIndex("functionId:%s AND type:(BitAndExpression InclusiveOrExpression ExclusiveOrExpression)").groupBy{it.functionId}{it.id}.cap"""%functionIds # & | ^
        FunctionMetrics.bitoperationNodes = j.runGremlinQuery(query)[0]

        query = """queryNodeIndex("functionId:%s AND type:Identifier").map("id","functionId","code").groupBy{it.functionId}{it}.cap"""%functionIds
        FunctionMetrics.identifier = j.runGremlinQuery(query)[0]
        
        query = """queryNodeIndex("functionId:%s AND type:Symbol").map("id","code","functionId").groupBy{it.functionId}{it}.cap"""%functionIds
        FunctionMetrics.symbol = j.runGremlinQuery(query)[0]
        
        query = """queryNodeIndex("functionId:%s AND type:Symbol").as("x").in("USE").astNodes().filter{it.type=="Identifier"}.code.as("y").select{it.id}{it}"""%functionIds
        symbolReuse = j.runGremlinQuery(query)        
        for i in symbolReuse:
            symId = i[0]
            if symId not in FunctionMetrics.symbolReuse:
                FunctionMetrics.symbolReuse[symId] = []
                FunctionMetrics.symbolReuse[symId].append(i[1])                
            else:
                FunctionMetrics.symbolReuse[symId].append(i[1])        
        del symbolReuse
        query = """queryNodeIndex("functionId:%s AND type:Callee").groupBy{it.functionId}{it.code}.cap"""%functionIds
        FunctionMetrics.fanOuts = j.runGremlinQuery(query)[0]
        
        query = """queryNodeIndex("functionId:%s AND type:IdentifierDecl").as("x").children().as("y").select{it.id}{it}"""%functionIds
        identifierDecl = j.runGremlinQuery(query)                
        for i in identifierDecl:
            funcId = i[1].properties[u'functionId']
            if funcId not in FunctionMetrics.identifierDecl:
                FunctionMetrics.identifierDecl[funcId] = []
                FunctionMetrics.identifierDecl[funcId].append(i)                
            else:
                FunctionMetrics.identifierDecl[funcId].append(i)
        del identifierDecl
        
        query = """queryNodeIndex("functionId:%s AND type:IdentifierDeclStatement").as("x").uses().code.as("y").select"""%functionIds
        declStmsuse = j.runGremlinQuery(query)        
        for i in declStmsuse:
            funcId = i[0].properties[u'functionId']
            if funcId not in FunctionMetrics.declStmsuse:
                FunctionMetrics.declStmsuse[funcId] = []
                FunctionMetrics.declStmsuse[funcId].append([i[0].ref[5:], i[1]])                
            else:
                FunctionMetrics.declStmsuse[funcId].append([i[0].ref[5:], i[1]])
        del declStmsuse
        
        query = """queryNodeIndex("functionId:%s AND type:IdentifierDeclStatement").as("x").defines().code.as("y").select"""%functionIds
        declStmsdefine = j.runGremlinQuery(query)        
        for i in declStmsdefine:     
            funcId = i[0].properties[u'functionId']
            if funcId not in FunctionMetrics.declStmsdefine:
                FunctionMetrics.declStmsdefine[funcId] = []
                FunctionMetrics.declStmsdefine[funcId].append([i[0].ref[5:], i[1]])                
            else:
                FunctionMetrics.declStmsdefine[funcId].append([i[0].ref[5:], i[1]])
        del declStmsdefine
        query = """queryNodeIndex("functionId:%s AND type:Parameter").as("x").children().as("y").select{it.id}{it}"""%functionIds
        parameters = j.runGremlinQuery(query)        
        for i in parameters:
            funcId = i[1].properties[u'functionId']
            if funcId not in FunctionMetrics.parameters:
                FunctionMetrics.parameters[funcId] = []
                FunctionMetrics.parameters[funcId].append(i)                
            else:
                FunctionMetrics.parameters[funcId].append(i)
        del parameters
        
        query = """queryNodeIndex("functionId:%s AND type:(UnaryExpression IncDecOp)").astNodes().filter{it.type=="Identifier"}.groupBy{it.functionId}{it.id}.cap"""%functionIds #% inDecOpId
        FunctionMetrics.IncDecChildren = j.runGremlinQuery(query)[0]
        #the below query maybe not faster than direct query as it's combined with a for loop    
        query = """queryNodeIndex("functionId:%s AND type:ShiftExpression").astNodes().filter{it.type=="Identifier"}.groupBy{it.functionId}{it.id}.cap"""%functionIds #% shiftStmsId
        FunctionMetrics.shiftstmsChildren = j.runGremlinQuery(query)[0]
        
        query = """queryNodeIndex("functionId:%s AND type:(BitAndExpression InclusiveOrExpression ExclusiveOrExpression)").astNodes().filter{it.type=="Identifier"}.groupBy{it.functionId}{it.id}.cap"""%functionIds #% bitoperationNodesId
        FunctionMetrics.BitOperationChildren = j.runGremlinQuery(query)[0]
        
        query = """queryNodeIndex("functionId:%s AND type:(MultiplicativeExpression AdditiveExpression)").astNodes().filter{it.type=="Identifier"}.groupBy{it.functionId}{it.id}.cap"""%functionIds  #% BasicArithmeticId
        BasicArithmetic = j.runGremlinQuery(query)[0]
        query = """queryNodeIndex("functionId:"""+functionIds+""" AND type:AssignmentExpr").filter{it.operator in ["+=","-=","*=","/=","%="]}.astNodes().filter{it.type=="Identifier"}.groupBy{it.functionId}{it.id}.cap"""#%functionIds 
        AssignmentExpr = j.runGremlinQuery(query)[0]
        FunctionMetrics.BasicArithmeticChildren = BasicArithmetic
        for (funcId, idlist) in FunctionMetrics.BasicArithmeticChildren.items():
            if AssignmentExpr.has_key(funcId):
                FunctionMetrics.BasicArithmeticChildren[funcId].extend(AssignmentExpr[funcId])
                del AssignmentExpr[funcId]
        FunctionMetrics.BasicArithmeticChildren = dict(FunctionMetrics.BasicArithmeticChildren,**AssignmentExpr)
        del AssignmentExpr
        
        query = """queryNodeIndex("functionId:%s AND type:ReturnStatement").astNodes().filter{it.type=="Identifier"}.dedup().groupBy{it.functionId}{it.code}.cap"""%functionIds 
        FunctionMetrics.returnDes = j.runGremlinQuery(query)[0]
        query = """queryNodeIndex("functionId:%s AND type:ReturnStatement").astNodes().filter{it.type in ["Condition","CallExpression"]}.astNodes().filter{it.type=="Identifier"}.dedup().groupBy{it.functionId}{it.code}.cap"""%functionIds 
        returnConCallIds = j.runGremlinQuery(query)[0]
        for (funcId, idsList) in returnConCallIds.items():
            for i in idsList:
                FunctionMetrics.returnDes[funcId].remove(i)       
        del returnConCallIds
        # Pointer involved statements
        query = """queryNodeIndex("functionId:%s AND type:PtrMemberAccess").groupBy{it.functionId}{it}{it.size()}.cap"""%functionIds 
        FunctionMetrics.ptrMemberAccess = j.runGremlinQuery(query)[0]
        
        query = """queryNodeIndex("functionId:%s AND type:PtrMemberAccess").out("USE").groupBy{it.functionId}{it.code}.cap"""%functionIds 
        FunctionMetrics.ptrMemberAccessSys = j.runGremlinQuery(query)[0]
        
        query = """queryNodeIndex("functionId:%s AND type:UnaryOp").as("op").children().filter{it.type=="UnaryOperator" && it.code=="&"}.back("op").astNodes().filter{it.type=="Identifier"}.code.as("sys").select"""%functionIds 
        addressOps = j.runGremlinQuery(query)
        for i in addressOps:
            funcId = i[0].properties[u'functionId']
            if funcId not in FunctionMetrics.addressOps:
                FunctionMetrics.addressOps[funcId] = []
                FunctionMetrics.addressOps[funcId].append(i)            
            else:
                FunctionMetrics.addressOps[funcId].append(i)
        del addressOps
        
        query = """queryNodeIndex("functionId:%s AND type:UnaryOp").as("here").children().filter{it.type=="UnaryOperator" && it.code=="*"}.back("here").groupBy{it.functionId}{it}{it.size()}.cap"""%functionIds 
        FunctionMetrics.dereferenceOps = j.runGremlinQuery(query)[0]
        
        query = """queryNodeIndex("functionId:%s AND type:UnaryOp").as("here").children().filter{it.type=="UnaryOperator" && it.code=="*"}.back("here").out("USE").groupBy{it.functionId}{it.code}.cap"""%functionIds  #% dereferenceOpsId
        FunctionMetrics.dereferenceOpsSys = j.runGremlinQuery(query)[0]
        
        query = """queryNodeIndex("functionId:%s AND type:(RelationalExpression EqualityExpression)").as("x").children().filter{it.type=="Identifier"}.code.as("y").select"""%functionIds 
        relationOps = j.runGremlinQuery(query)
        for i in relationOps:
            funcId = i[0].properties[u'functionId']
            if funcId not in FunctionMetrics.relationOps:
                FunctionMetrics.relationOps[funcId] = []
                FunctionMetrics.relationOps[funcId].append(i)            
            else:
                FunctionMetrics.relationOps[funcId].append(i)
        del relationOps
        query="""queryNodeIndex("functionId:%s AND type:ParameterList").groupBy{it.functionId}{it.code}.cap"""%functionIds 
        FunctionMetrics.parameterLists = (j.runGremlinQuery(query))[0]

        query = """queryNodeIndex("functionId:%s AND type:Condition").astNodes().filter{it.type=="PrimaryExpression"}.groupBy{it.functionId}{it.code}.cap""" % functionIds
        FunctionMetrics.constants = j.runGremlinQuery(query)[0]

        # query="""queryNodeIndex("functionId:%s AND type:PrimaryExpression").as("x").parents().as("y").parents().as("z").select{it}{it}{it}"""%functionIds
        # allconstants=j.runGremlinQuery(query)
        #
        # for i in allconstants:
        #     # print i[0],i[1],i[2]
        #     funcId = i[0].properties[u'functionId']
        #     if i[1].properties[u'type']!="ReturnStatement" and i[2].properties[u'type']!="ReturnStatement":
        #         if funcId not in FunctionMetrics.constants:
        #             FunctionMetrics.constants[funcId] = []
        #             FunctionMetrics.constants[funcId].append(i[0].properties[u'code'])
        #         else:
        #             FunctionMetrics.constants[funcId].append(i[0].properties[u'code'])
        # del allconstants

        query="""queryNodeIndex("functionId:%s AND type:CastExpression").groupBy{it.functionId}{it}{it.size()}.cap"""%functionIds        
        FunctionMetrics.castExpressions = j.runGremlinQuery(query)[0]
        
        query = """queryNodeIndex("functionId:%s AND type:Callee AND code:%s").groupBy{it.functionId}{it}{it.size()}.cap"""%(functionIds,castApisStr)
        FunctionMetrics.castApis = j.runGremlinQuery(query)[0]        

        totalData = []
        maxcontroldependencypre = 0 #Maybe should be put outside the for loop to maintain the max value of whole program not of each 1000 functions
        maxdatadependencypre = 0 #Maybe should be put outside the for loop to maintain the max value of whole program not of each 1000 functions
        controldependencyresult = []
        datadependencyresult = []
        maxcontroldependencyprenum = []
        maxdatadependencyprenum = []
        for i in functions:
            functionId = int(i[0].ref[5:])
            fileNode = int(i[1].ref[5:])        
            function = FunctionMetrics(functionId, fileNode)
            function.setFunctionName(i[0].properties[u'name'])
            function.setFilePath(i[1].properties[u'filepath'])
            # data = function.run()
            # totalData.append(data)
            data, maxcontrol, controlresult, maxdata, dataresult = function.run()
            if maxcontrol > maxcontroldependencypre:
                maxcontroldependencypre = maxcontrol
            if maxdata > maxdatadependencypre:
                maxdatadependencypre = maxdata
            totalData.append(data)
            controldependencyresult.append(controlresult)
            datadependencyresult.append(dataresult)
            maxcontroldependencyprenum.append(maxcontrol)
            maxdatadependencyprenum.append(maxdata)
        
        with open('xxx', 'ab') as csvfile:#Append data to the end of file
            writer = csv.writer(csvfile)            
            functionnum = len(totalData)
            for i in range(functionnum):
                data = totalData[i]
                for d in data:
                    csvfile.write(str(d))
                    csvfile.write(',')

                # csvfile.write(str(maxcontroldependencyprenum[i]))
                # csvfile.write(',')

                # controldata = controldependencyresult[i]
                # for ii in range(95):
                #     # 95->maxcontroldependencypre ?
                #     if ii == 0:
                #         continue
                #     if (controldata.has_key(ii)):
                #         csvfile.write(str((controldata.get(ii))))
                #         csvfile.write(',')
                #     else:
                #         csvfile.write('0')
                #         csvfile.write(',')

                # csvfile.write(str(maxdatadependencyprenum[i]))
                # csvfile.write(',')
                # datadependencydata = datadependencyresult[i]
                # for ii in range(150):
                #     # 150->maxdatadependencypre ?
                #     if ii == 0:
                #         continue
                #     if (datadependencydata.has_key(ii)):
                #         csvfile.write(str((datadependencydata.get(ii))))
                #         csvfile.write(',')
                #     else:
                #         csvfile.write('0')
                #         csvfile.write(',')

                csvfile.write('\n')
            csvfile.close()
