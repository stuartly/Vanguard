import os
import string
import time
import csv
import numpy as np
import re
import py2neo
from joern.all import JoernSteps
from symbol import Symbol

class Links(object):
	def __init__(self,j):
		self.j=j
		self.iniSources=[]
		self.iniSinks=[]
		file = open("source")		
		self.iniSources = file.readlines()
		file.close()
		file = open("sink")		
		self.iniSinks = file.readlines()
		file.close()

	def getIniSources(self):
		return self.iniSources

	def getIniSinks(self):
		return self.iniSinks
		
	def getLinksToSource(self):					
		sourceName=[]			
		sourceFuncCallees={}
		iniResult=[]
		for i in self.iniSources:					
			query="""queryNodeIndex("type:Callee AND code:*%s*").as("x").functions().map('id','name').as("y").select{it.id}{it}"""%(i[:-2])				
			results=self.j.runGremlinQuery(query)			
			iniResult.extend(results)
		for res in iniResult:			
			funcId=int(res[1][u'id'])
			if funcId in sourceFuncCallees:
				sourceFuncCallees[funcId].append(res[0])
			else:
				sourceFuncCallees[funcId]=[]
				sourceFuncCallees[funcId].append(res[0])
			sourceName.append(str(res[1][u'name']))
			
		called = {}
		while True:				
			sourceName=list(set(sourceName))
			CHUNK_SIZE=1024
			stageResults=[]						
			for chunk in self.j.chunks(sourceName, CHUNK_SIZE):						
				nameTuple=tuple(chunk)
				nameStr=str(nameTuple)			
				nameStr=nameStr.replace(',','')
				nameStr=nameStr.replace('\'','')							
				query="""queryNodeIndex("type:Callee AND code:%s").as("x").functions().map('id','name').as("y").select{it.id}{it}"""%nameStr		
				res=self.j.runGremlinQuery(query)					
				stageResults.extend(res)
				for each in chunk:
					called[each]=1
			
			if len(stageResults) > 0:		
				sourceName=[]									
				for res in stageResults:
					funcId=int(res[1][u'id'])
					if funcId in sourceFuncCallees:
						sourceFuncCallees[funcId].append(res[0])
					else:
						sourceFuncCallees[funcId]=[]
						sourceFuncCallees[funcId].append(res[0])				

					name = str(res[1][u'name'])							
					if name not in called:
						sourceName.append(name)																				
			else:				
				break								
		return sourceFuncCallees

	def getLinksToSink(self):							
		sinkName=[]				
		sinkFuncCallees={}
		iniResult=[]
		for i in self.iniSinks:		
			query="""queryNodeIndex("type:Callee AND code:*%s*").as("x").functions().map('id','name').as("y").select{it.id}{it}"""%(i[:-2])			
			results=self.j.runGremlinQuery(query)		
			iniResult.extend(results)
		for res in iniResult:
			funcId=int(res[1][u'id'])
			if funcId in sinkFuncCallees:
				sinkFuncCallees[funcId].append(res[0])
			else:
				sinkFuncCallees[funcId]=[]
				sinkFuncCallees[funcId].append(res[0])			
			sinkName.append(str(res[1][u'name']))					
				
		called = {}
		while True:						
			sinkName=list(set(sinkName))
			CHUNK_SIZE=1024
			stageResults=[]			
			for chunk in self.j.chunks(sinkName, CHUNK_SIZE):				
				nameTuple=tuple(chunk)
				nameStr=str(nameTuple)
				nameStr=nameStr.replace(',','')
				nameStr=nameStr.replace('\'','')
				query="""queryNodeIndex("type:Callee AND code:%s").as("x").functions().map('id','name').as("y").select{it.id}{it}"""%nameStr						
				res=self.j.runGremlinQuery(query)
				stageResults.extend(res)
				for each in chunk:
					called[each]=1
			
			if len(stageResults) > 0:	
				sinkName=[]
				for res in stageResults:		
					funcId=int(res[1][u'id'])
					if funcId in sinkFuncCallees:
						sinkFuncCallees[funcId].append(res[0])
					else:
						sinkFuncCallees[funcId]=[]
						sinkFuncCallees[funcId].append(res[0])								
					name = str(res[1][u'name'])					
					if name not in called:
						sinkName.append(name)					
			else:
				break					
		return sinkFuncCallees

	# def getNumCallSource(self):
	# 	return self.numCallSource

	# def getNumCallSink(self):
	# 	return self.numCallSink

if __name__ == '__main__':
	j = JoernSteps()
	j.setGraphDbURL('http://localhost:7474/db/data/')
	# j.addStepsDir('Use this to inject custom steps')
	j.connectToDatabase()
	
	# taintlink=Links(j)
	# iniSources=taintlink.getIniSources()
	# iniSinks=taintlink.getIniSinks()	
	query = """getNodesWithType("File").out("IS_FILE_OF").filter{it.type=="Class" && it.name!="<unnamed>"}.name"""
	nameList = j.runGremlinQuery(query)
	Symbol.setStructName(nameList)
	del nameList
	print "oh"
	print Symbol.StructName