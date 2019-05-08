import os
import string
import time
import csv
from joern.all import JoernSteps

class Symbol(object):

	#Reference http://en.cppreference.com/w/cpp/language/types(more than those of http://www.cplusplus.com/doc/tutorial/variables/, add string and size_t by myself)	 
	basicSymbolType=["void","nullptr_t","decltype(nullptr)","bool","signed char","unsigned char","char","wchar_t","char16_t","char32_t","int","short","short int",\
	"signed short","signed short int","unsigned short","unsigned short int","signed","signed int","unsigned","unsigned int","long","long int","signed long",\
	"signed long int","unsigned long","unsigned long int","long long","long long int","signed long long","signed long long int","unsigned long long",\
	"unsigned long long int","float","double","long double","string","size_t"]
	
	StructName=[]

	def __init__(self,functionId,symbolName):
		self.functionId=functionId
		self.symbolName=symbolName		
		self.symbolReuse=[]
		self.origin=[]
		self.destination=[]
		self.ptrArith=0
		self.taintSource=[]
		self.taintSink=[]

	def getSymbolName(self):
		return self.symbolName

	def getSymbolReuse(self):
		return self.symbolReuse

	def getDataType(self):
		return self.dataType

	def getNumPtrArith(self):
		return self.ptrArith

	def getSymbolOrigin(self):
		return self.origin

	def getSymbolDestination(self):
		return self.destination

	def getNumPtrArith(self):
		return self.ptrArith

	def setDataType(self,dataType):
		self.dataType=dataType

	def addSymbolReuse(self,otherSymbolName):
		self.symbolReuse.append(otherSymbolName)

	def addOrigin(self,origin):
		self.origin.append(origin)

	def addDestination(self,destination):
		self.destination.append(destination)

	def setNumPtrArith(self,ptrArith):
		self.ptrArith=ptrArith	

	def setTaintSource(self,sourceFuncs):
		self.taintSource=sourceFuncs

	def setTaintSink(self,sinkFuncs):
		self.taintSink=sinkFuncs

	@classmethod
	def setStructName(cls,nameList):#Class function, not instance function
		Symbol.StructName = [str(x) for x in nameList]		

	@classmethod
	def getSymbolDataType(cls,typeCode):#Class function, not instance function
		#print "paramin: ",typeCode," type: ",type(typeCode)
		stype=""
		if "*" in typeCode:					
			stype="Pointer"
		elif "[" in typeCode and "]" in typeCode:
			stype="Array"
		elif typeCode not in Symbol.basicSymbolType:	
			stype="User defined"
			typeCodeList = typeCode.split(' ')
			for t in typeCodeList:
				if t=="struct" or t in Symbol.StructName:
					stype="Struct"		
					break							
		else:
			stype=typeCode
		return stype

	@classmethod
	def removeArrowDot(cls,symbolName):#Class function, not instance function			
		symbol_new = symbolName.split(' ')
		arrowPos=len(symbol_new)
		dotPos=len(symbol_new)
		if '->' in symbol_new:	
			arrowPos=symbol_new.index('->')
		if '.' in symbol_new:
			dotPos=symbol_new.index('.')			
		removePos=min(arrowPos,dotPos)
		if removePos < len(symbol_new):
			symbol_new=symbol_new[:removePos]
		return symbol_new

	def __str__(self):
		return "Symbol %s: type %s---(Function %d)\n"% (self.symbolName, self.dataType,self.functionId)+"origin: %s\n"%self.origin+\
		"Reuse: %s\n"%self.symbolReuse+"destination:%s\n"%self.destination+"numPtrArith:%d\n"%self.ptrArith

if __name__ == '__main__':	
	j = JoernSteps()
	j.setGraphDbURL('http://localhost:7474/db/data/')
	# j.addStepsDir('Use this to inject custom steps')
	j.connectToDatabase()
	
	# query = """getNodesWithType("File").out("IS_FILE_OF").filter{it.type=="Class" && it.name!="<unnamed>"}.name"""
	# nameList = j.runGremlinQuery(query)
	# Symbol.setStructName(nameList)
	# del nameList

	# stype=Symbol.getSymbolDataType("name_funcs_st")
	# astype=Symbol.getSymbolDataType("const name_funcs_st")
	# abstype=Symbol.getSymbolDataType("const name_funcs_st *")
	# abctype=Symbol.getSymbolDataType("const struct new")
	# print stype
	# print astype
	# print abstype
	# print abctype	