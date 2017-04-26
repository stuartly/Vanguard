#include "BufferToStruct.h"

void BufferToStruct::check(ASTFunction *F, FunctionDecl *FD, CFG *function_cfg)
{
	clock_t start = clock(); 
    readConfig();   
	if(F != NULL)
	{	        
		visitStmt(F,FD,function_cfg);        
	}
	run_time += clock() - start;
}

clock_t BufferToStruct::get_time() 
{    
    return run_time;
}

BufferToStruct::BufferToStruct()
{
	WarningNum=0;
}

BufferToStruct::~BufferToStruct()
{	
	if(WarningNum != 0)
	{
		std::string xmlFile = (*common::configure)["pathToReport"]["path"].asString() + "MOLINT.BUFFET.TO.STRUCT.xml";
		doc.save_file(xmlFile.c_str(),"\t", pugi::format_default|pugi::format_no_declaration);
	}
}

string BufferToStruct::getString_of_Expr(const Expr* expr)
{	
	LangOptions L0;
	L0.CPlusPlus=1;
	std::string buffer1;
	llvm::raw_string_ostream strout1(buffer1);
	expr->printPretty(strout1,nullptr,PrintingPolicy(L0));
	return strout1.str();
}

string BufferToStruct::printStmt(const Stmt* stmt)
{
	LangOptions L0;
	L0.CPlusPlus=1;
	std::string buffer1;
	llvm::raw_string_ostream strout1(buffer1);
	stmt->printPretty(strout1,nullptr,PrintingPolicy(L0));
	return ""+strout1.str()+"";
}


void BufferToStruct::visitStmt(ASTFunction *F, FunctionDecl *FD, CFG *function_cfg)
{ 	
	for (CFGBlock* block :  *function_cfg)
	{
		for (auto element : *block)
		{
			if (element.getKind() == CFGElement::Statement)
			{
				const Stmt* stmt = element.castAs<CFGStmt>().getStmt();
				if(stmt->getStmtClass() == Stmt::ImplicitCastExprClass)
				{
					const CastExpr* castExpr = dyn_cast<CastExpr>(stmt);
					const Expr* subExpr=castExpr->getSubExpr();

					string sbs=getString_of_Expr(subExpr);

					string st=printStmt(stmt);

					writingToXML(sbs.c_str(),st.c_str(),"c","d");
				}
			}
		}
	}
	
}

//read the informaion from config, including the memory operation funcation names, check level, check mode, etc.
void BufferToStruct::readConfig()
{
    if((*common::configure)["MemOPCheckLevel"]["N"].empty())  
	{
		CheckLevel = 1;
	}

}

    

//write the warning information into the XML file.
void BufferToStruct::writingToXML(std::string fileName, std::string funName, std::string descr, std::string locLine)
{	
	WarningNum++;
    pugi::xml_node node = doc.append_child("error");
    pugi::xml_node checker = node.append_child("checker");
    checker.append_child(pugi::node_pcdata).set_value("MOLINT.BUFFER.TO.STRUCT");
	pugi::xml_node domain = node.append_child("domain");
    domain.append_child(pugi::node_pcdata).set_value("STATIC_C");
	pugi::xml_node file = node.append_child("file");
	file.append_child(pugi::node_pcdata).set_value(fileName.c_str());
	pugi::xml_node function = node.append_child("function");
    function.append_child(pugi::node_pcdata).set_value(funName.c_str());
	pugi::xml_node score = node.append_child("score");
    score.append_child(pugi::node_pcdata).set_value("100");
	pugi::xml_node ordered = node.append_child("ordered");
    ordered.append_child(pugi::node_pcdata).set_value("false");
	pugi::xml_node event = node.append_child("event");
	pugi::xml_node main = event.append_child("main");
    main.append_child(pugi::node_pcdata).set_value("true");
	pugi::xml_node tag = event.append_child("tag");
    tag.append_child(pugi::node_pcdata).set_value("Error");
	pugi::xml_node description = event.append_child("description");
    description.append_child(pugi::node_pcdata).set_value(descr.c_str());
	pugi::xml_node line = event.append_child("line");
    line.append_child(pugi::node_pcdata).set_value(locLine.c_str());
	pugi::xml_node extra = node.append_child("extra");
    extra.append_child(pugi::node_pcdata).set_value("none");
	pugi::xml_node subcategory = node.append_child("subcategory");
    subcategory.append_child(pugi::node_pcdata).set_value("none");
}









