#include"parser.h"
#include"record.h"
#include "tools.h"
#include<map>
using namespace std;

Parser::Parser(TreePtr root) {
	this->root = root;
	Init();
}

Parser::~Parser() {
	print_code();
}

void Parser::Init() {
	Record wholeRecord;
	recordStack.push_back(wholeRecord);

	//�������ú���print��read
	funcNode printFunc;
	printFunc.name = "print";
	printFunc.rtype = "void";
	varNode pnode;
	pnode.type = "int";
	printFunc.paralist.push_back(pnode);

	funcPool.insert({ "print", printFunc });

	funcNode readFunc;
	readFunc.name = "read";
	readFunc.rtype = "int";
	funcPool.insert({"read",readFunc });

	parseTree(root);		//��ʼ�����﷨��
}

void Parser::parseTree(TreePtr node) {
	if (node == NULL || node->line == -1)
		return;

	if (node->name == "declaration") {
		node = parserDeclaration(node);
	}
	else if (node->name == "function_definition") {
		node = parserFunctionDefinition(node);
	}
	else if (node->name == "statement") {
		node = parserStatement(node);
	}

	//�������·���
	if (node != NULL) {
		parseTree(node->left);
		parseTree(node->right);
	}
}

TreePtr Parser::parserStatement(TreePtr node) {
	TreePtr next = node->left;
	if (node->left->name == "labeled_statement") {

	}
	if (node->left->name == "compound_statement") {
		parserCompoundStatement(node->left);
	}
	if (node->left->name == "expression_statement") {
		parserExpressionStatement(node->left);
	}
	if (node->left->name == "selection_statement") {
		parserSelectionStatement(node->left);
	}
	if (node->left->name == "iteration_statement") {
		parserIterationStatement(node->left);
	}
	if (node->left->name == "jump_statement") {
		parserJumpStatement(node->left);
	}

	return node->right;
}

void Parser::parserJumpStatement(TreePtr node) {
	if (node->left->name == "BREAK") {
		int recordNum = getBreakRecordNumber();
		if (recordNum < 0) {
			printError(node->left->line, "This scope doesn't support break.");
		}
	
		innerCode.addCode("GOTO " + recordStack[recordNum].breakLabel);
	}
	else if (node->left->name == "RETURN") {
		string funcType = getFuncRType();
		if (node->left->right->name == "expression") {//return expression
			varNode rnode = parserExpression(node->left->right);
			innerCode.addCode(innerCode.createCodeforReturn(rnode));
			if (rnode.type != funcType) {
				printError(node->left->right->line, "return type doesn't equal to function return type.");
			}
		}
		else if (node->left->right->name == ";"){//return ;
			innerCode.addCode("RETURN");
			if (funcType != "void") {
				printError(node->left->right->line, "You should return " + recordStack.back().func.rtype);
			}
		}
	}
}

void Parser::parserExpressionStatement(TreePtr node) {
	if (node->left->name == "expression") {
		parserExpression(node->left);
	}
}

varNode Parser::parserExpression(TreePtr node) {
	if (node->left->name == "expression") {
		return parserExpression(node->left);
	}
	else if (node->left->name == "assignment_expression") {
		return parserAssignmentExpression(node->left);
	}
	if (node->right->name == ",") {
		return parserAssignmentExpression(node->right->right);
	}
}

void Parser::parserCompoundStatement(TreePtr node) {
	//������������compound_statement
	parseTree(node);
}

//if else
void Parser::parserSelectionStatement(TreePtr node) {


	if (node->left->name == "IF") {
		if (node->left->right->right->right->right->right == NULL) {
			//���һ���µ�block
			Record newrecord;
			recordStack.push_back(newrecord);

			Tree* expression = node->left->right->right;
			varNode exp_rnode = parserExpression(expression);
			Tree* statement = node->left->right->right->right->right;

			string label1 = innerCode.getLabelName();
			string label2 = innerCode.getLabelName();

			if (exp_rnode.type == "bool") {
				innerCode.addCode("IF " + exp_rnode.boolString + " GOTO " + label1);
			}
			else {
				string tempzeroname = "temp" + inttostr(innerCode.tempNum);
				++innerCode.tempNum;
				varNode newznode = createVar(tempzeroname, "int");
				innerCode.addCode(tempzeroname + " := #0");

				innerCode.addCode("IF " + innerCode.getNodeName(exp_rnode) + " != " + tempzeroname + " GOTO " + label1);
			}
			
			innerCode.addCode("GOTO " + label2);
			innerCode.addCode("LABEL " + label1 + " :");


			parserStatement(statement);
			
			innerCode.addCode("LABEL " + label2 + " :");

			//������ӵ�block
			recordStack.pop_back();

		}
		else if (node->left->right->right->right->right->right->name == "ELSE") {
			//���һ���µ�block
			Record newrecord1;
			recordStack.push_back(newrecord1);

			Tree* expression = node->left->right->right;
			varNode exp_rnode = parserExpression(expression);
			Tree* statement1 = node->left->right->right->right->right;
			Tree* statement2 = node->left->right->right->right->right->right->right;

			string label1 = innerCode.getLabelName();
			string label2 = innerCode.getLabelName();
			string label3 = innerCode.getLabelName();

			if (exp_rnode.type == "bool") {
				innerCode.addCode("IF " + exp_rnode.boolString + " GOTO " + label1);
			}
			else {
				string tempzeroname = "temp" + inttostr(innerCode.tempNum);
				++innerCode.tempNum;
				varNode newznode = createVar(tempzeroname, "int");
				innerCode.addCode(tempzeroname + " := #0");

				innerCode.addCode("IF " + innerCode.getNodeName(exp_rnode) + " != " + tempzeroname + " GOTO " + label1);
			}

			innerCode.addCode("GOTO " + label2);
			innerCode.addCode("LABEL " + label1 + " :");

			parserStatement(statement1);
			
			innerCode.addCode("GOTO " + label3);
			//������ӵ�block
			recordStack.pop_back();

			//else
			innerCode.addCode("LABEL " + label2 + " :");

			Record newrecord2;
			recordStack.push_back(newrecord2);

			parserStatement(statement2);

			innerCode.addCode("LABEL " + label3 + " :");

			//������ӵ�block
			recordStack.pop_back();

		}
	}
	else if (node->left->name == "SWITCH") {

	}
	
}

//ѭ�� while for do while
void Parser::parserIterationStatement(TreePtr node) {
	if (node->left->name == "WHILE") {

		//���һ���µ�block
		Record newrecord;
		newrecord.canBreak = true;
		recordStack.push_back(newrecord);

		TreePtr expression = node->left->right->right;
		TreePtr statement = node->left->right->right->right->right;

		string label1 = innerCode.getLabelName();
		string label2 = innerCode.getLabelName();
		string label3 = innerCode.getLabelName();

		recordStack.back().breakLabel = label3;

		innerCode.addCode("LABEL " + label1 + " :");

		varNode var = parserExpression(expression);

		if (var.type == "bool") {  
			innerCode.addCode("IF " + var.boolString + " GOTO " + label2);
		}
		else {
			string tempzeroname = "temp" + inttostr(innerCode.tempNum);
			++innerCode.tempNum;
			varNode newznode = createVar(tempzeroname, "int");
			innerCode.addCode(tempzeroname + " := #0");

			innerCode.addCode("IF " + innerCode.getNodeName(var) + " != " + tempzeroname + " GOTO " + label2);
		}
		innerCode.addCode("GOTO " + label3);
		innerCode.addCode("LABEL " + label2 + " :");

		parserStatement(statement);

		innerCode.addCode("GOTO " + label1);
		innerCode.addCode("LABEL " + label3 + " :");
		

		//������ӵ�block
		recordStack.pop_back();
	}
	else if (node->left->name == "DO") {
		//���һ���µ�block
		Record newrecord;
		newrecord.canBreak = true;
		recordStack.push_back(newrecord);

		TreePtr statement = node->left->right;
		TreePtr expression = node->left->right->right->right->right;

		string label1 = innerCode.getLabelName();
		string label2 = innerCode.getLabelName();

		recordStack.back().breakLabel = label2;

		innerCode.addCode("LABEL " + label1 + " :");

		parserStatement(statement);

		varNode var = parserExpression(expression);

		if (var.type == "bool") {
			innerCode.addCode("IF " + var.boolString + " GOTO " + label1);
		}
		else {
			string tempzeroname = "temp" + inttostr(innerCode.tempNum);
			++innerCode.tempNum;
			varNode newznode = createVar(tempzeroname, "int");
			innerCode.addCode(tempzeroname + " := #0");

			innerCode.addCode("IF " + innerCode.getNodeName(var) + " != " + tempzeroname + " GOTO " + label1);
		}

		/*innerCode.addCode("GOTO " + label1);*/
		innerCode.addCode("LABEL " + label2 + " :");

		//������ӵ�block
		recordStack.pop_back();

	}
	else if (node->left->name == "FOR") {
		if (node->left->right->right->name == "expression_statement") {
			//FOR '(' expression_statement expression_statement ')'statement
			if (node->left->right->right->right->right->name == ")") {
				//���һ���µ�block
				Record newblock;
				newblock.canBreak = true;
				recordStack.push_back(newblock);

				Tree* exp_state1 = node->left->right->right;
				Tree* exp_state2 = exp_state1->right;
				Tree* statement = exp_state2->right->right;

				string label1 = innerCode.getLabelName();
				string label2 = innerCode.getLabelName();
				string label3 = innerCode.getLabelName();

				recordStack.back().breakLabel = label3;

				if (exp_state1->left->name == "expression") {
					parserExpression(exp_state1->left);
				}
				innerCode.addCode("LABEL " + label1 + " :");

				varNode var;
				if (exp_state2->left->name == "expression") {
					var = parserExpression(exp_state2->left);
					if (var.type == "bool") {
						innerCode.addCode("IF " + var.boolString + " GOTO " + label2);
					}
					else {
						string tempzeroname = "temp" + inttostr(innerCode.tempNum);
						++innerCode.tempNum;
						varNode newznode = createVar(tempzeroname, "int");
						innerCode.addCode(tempzeroname + " := #0");

						innerCode.addCode("IF " + innerCode.getNodeName(var) + " != " + tempzeroname + " GOTO " + label2);
					}
				}
				else {
					innerCode.addCode("GOTO " + label2);
				}

				innerCode.addCode("GOTO " + label3);
				innerCode.addCode("LABEL " + label2 + " :");

				parserStatement(statement);

				innerCode.addCode("GOTO " + label1);
				innerCode.addCode("LABEL " + label3 + " :");

				////�����Ҫbreak
				//if (blockStack.back().breakLabelNum > 0) {
				//	innerCode.addCode("LABEL label" + inttostr(blockStack.back().breakLabelNum) + " :");
				//}

				//������ӵ�block
				recordStack.pop_back();
			}
			//FOR ( expression_statement expression_statement expression ) statement
			else if (node->left->right->right->right->right->name == "expression") {
				//���һ���µ�block
				Record newblock;
				newblock.canBreak = true;
				recordStack.push_back(newblock);

				Tree* exp_state1 = node->left->right->right;
				Tree* exp_state2 = exp_state1->right;
				Tree* exp = exp_state2->right;
				Tree* statement = exp->right->right;

				string label1 = innerCode.getLabelName();
				string label2 = innerCode.getLabelName();
				string label3 = innerCode.getLabelName();

				recordStack.back().breakLabel = label3;

				if (exp_state1->left->name == "expression") {
					parserExpression(exp_state1->left);
				}
				innerCode.addCode("LABEL " + label1 + " :");

				varNode var;
				if (exp_state2->left->name == "expression") {
					var = parserExpression(exp_state2->left);

					if (var.type == "bool") {
						innerCode.addCode("IF " + var.boolString + " GOTO " + label2);
					}
					else {
						string tempzeroname = "temp" + inttostr(innerCode.tempNum);
						++innerCode.tempNum;
						varNode newznode = createVar(tempzeroname, "int");
						innerCode.addCode(tempzeroname + " := #0");

						innerCode.addCode("IF " + innerCode.getNodeName(var) + " != " + tempzeroname + " GOTO " + label2);
					}
				}
				else {
					innerCode.addCode("GOTO " + label2);
				}

				innerCode.addCode("GOTO " + label3);
				innerCode.addCode("LABEL " + label2 + " :");

				parserStatement(statement);

				parserExpression(exp);

				innerCode.addCode("GOTO " + label1);
				innerCode.addCode("LABEL " + label3 + " :");

				////�����Ҫbreak
				//if (blockStack.back().breakLabelNum > 0) {
				//	innerCode.addCode("LABEL label" + inttostr(blockStack.back().breakLabelNum) + " :");
				//}

				//������ӵ�block
				recordStack.pop_back();
			}
		}
		if (node->left->right->right->name == "declaration") {
			//FOR '(' declaration expression_statement ')' statement
			if (node->left->right->right->right->right->name == ")") {
				//���һ���µ�block
				Record newblock;
				newblock.canBreak = true;
				recordStack.push_back(newblock);

				Tree *declaration = node->left->right->right;
				Tree *expression_statement = declaration->right;
				Tree *statement = expression_statement->right->right;

				string label1 = innerCode.getLabelName();
				string label2 = innerCode.getLabelName();
				string label3 = innerCode.getLabelName();

				recordStack.back().breakLabel = label3;

				parserDeclaration(declaration);
				innerCode.addCode("LABEL " + label1 + " :");

				varNode var;
				if (expression_statement->left->name == "expression") {

					var = parserExpression(expression_statement->left);

					if (var.type == "bool") {
						innerCode.addCode("IF " + var.boolString + " GOTO " + label2);
					}
					else {
						string tempzeroname = "temp" + inttostr(innerCode.tempNum);
						++innerCode.tempNum;
						varNode newznode = createVar(tempzeroname, "int");
						innerCode.addCode(tempzeroname + " := #0");

						innerCode.addCode("IF " + innerCode.getNodeName(var) + " != " + tempzeroname + " GOTO " + label2);
					}
				}
				else {
					innerCode.addCode("GOTO " + label2);
				}
				innerCode.addCode("GOTO " + label3);
				innerCode.addCode("LABEL " + label2 + " :");

				parserStatement(statement);

				//cout << "here" << endl;
				innerCode.addCode("GOTO " + label1);
				innerCode.addCode("LABEL " + label3 + " :");

				////�����Ҫbreak
				//if (blockStack.back().breakLabelNum > 0) {
				//	innerCode.addCode("LABEL label" + inttostr(blockStack.back().breakLabelNum) + " :");
				//}

				//������ӵ�block
				recordStack.pop_back();

			}
			//FOR ( declaration expression_statement expression ) statement
			else if (node->left->right->right->right->right->name == "expression") {
				//���һ���µ�block
				Record newblock;
				newblock.canBreak = true;
				recordStack.push_back(newblock);

				Tree *declaration = node->left->right->right;
				Tree *expression_statement = declaration->right;
				Tree *expression = expression_statement->right;
				Tree *statement = expression->right->right;

				string label1 = innerCode.getLabelName();
				string label2 = innerCode.getLabelName();
				string label3 = innerCode.getLabelName();

				recordStack.back().breakLabel = label3;

				parserDeclaration(declaration);
				innerCode.addCode("LABEL " + label1 + " :");

				varNode var;
				if (expression_statement->left->name == "expression") {
					var = parserExpression(expression_statement->left);

					if (var.type == "bool") {
						innerCode.addCode("IF " + var.boolString + " GOTO " + label2);
					}
					else {
						string tempzeroname = "temp" + inttostr(innerCode.tempNum);
						++innerCode.tempNum;
						varNode newznode = createVar(tempzeroname, "int");
						innerCode.addCode(tempzeroname + " := #0");

						innerCode.addCode("IF " + innerCode.getNodeName(var) + " != " + tempzeroname + " GOTO " + label2);
					}
				}
				else {
					innerCode.addCode("GOTO " + label2);
				}
				innerCode.addCode("GOTO " + label3);
				innerCode.addCode("LABEL " + label2 + " :");

				parserStatement(statement);

				parserExpression(expression);
				//cout << "here" << endl;
				innerCode.addCode("GOTO " + label1);
				innerCode.addCode("LABEL " + label3 + " :");

				////�����Ҫbreak
				//if (blockStack.back().breakLabelNum > 0) {
				//	innerCode.addCode("LABEL label" + inttostr(blockStack.back().breakLabelNum) + " :");
				//}

				//������ӵ�block
				recordStack.pop_back();
			}
		}
	}
}

//��������
TreePtr Parser::parserFunctionDefinition(TreePtr node) {
	Tree* type_specifier = node->left;
	Tree* declarator = node->left->right;
	Tree* compound_statement = declarator->right;
	
	string funcType = type_specifier->left->content;
	string funcName = declarator->left->left->content;

	/*if (build_in_function.find(funcName) != build_in_function.end()) {
		error(declarator->left->left->line, "Function name can't be bulid in function.");
	}*/

	bool isdeclared = false;
	funcNode declarFunc;
	if (funcPool.find(funcName) != funcPool.end()) {
		//�����ظ�����
		if (funcPool[funcName].isdefinied) {
			printError(declarator->left->left->line, "Function " + funcName + " is duplicated definition.");
		}
		//������������������û�ж���
		else {
			isdeclared = true;
			//��ɾ�����������еĺ���������
			declarFunc = funcPool[funcName];
			funcPool.erase(funcPool.find(funcName));
		}
	}

	//�����µ�block
	Record funBlock;
	funBlock.isfunc = true;
	funBlock.func.name = funcName;
	funBlock.func.rtype = funcType;
	funBlock.func.isdefinied = true;
	//��������¼�ڿ��ڲ���ӵ�������
	recordStack.push_back(funBlock);
	funcPool.insert({funcName,funBlock.func});

	innerCode.addCode("FUNCTION " + funcName + " :");

	//��ȡ�����β��б�
	if(declarator->left->right->right->name == "parameter_list")
		parserParameterList(declarator->left->right->right, funcName,true);

	//��ʱ�������е�func�Ѿ�����˲����б�
	funcNode func = funcPool[funcName];
	//���������������������ȽϺ����Ĳ����б�ͷ�������
	if (isdeclared) {
		if (func.rtype != declarFunc.rtype) {
			printError(type_specifier->left->line, "Function return type doesn't equal to the function declared before.");
		}
		cout << funBlock.func.paralist.size() << endl;
		if (func.paralist.size() != declarFunc.paralist.size()) {
			printError(declarator->left->right->right->line, "The number of function parameters doesn't equal to the function declared before.");
		}
		for (int i = 0; i < funBlock.func.paralist.size(); i++) {
			if (func.paralist[i].type != declarFunc.paralist[i].type)
				printError(declarator->left->right->right->line, "The parameter " + funBlock.func.paralist[i].name + "'s type doesn't equal to the function declared before." );
		}
	}
	//����Block��func�Ĳ����б�
	funBlock.func = func;
	//��������������
	parserCompoundStatement(compound_statement);

	//���������󣬵�����Ӧ��block
	recordStack.pop_back();

	return node->right;
}

//��ȡ�����β��б�����������Ҫ��ȡ�βΣ���������Ҫ
void Parser::parserParameterList(TreePtr node,string funcName,bool definite) {
	if (node->left->name == "parameter_list") {
		parserParameterList(node->left, funcName,definite);
	}
	else if (node->left->name == "parameter_declaration") {
		parserParameterDeclaration(node->left,funcName,definite);
	}

	if (node->right->name == ",") {
		parserParameterDeclaration(node->right->right, funcName,definite);
	}
}

//��ȡ�����β�����,����������Ҫ��ȡ�βΣ���������Ҫ
void Parser::parserParameterDeclaration(TreePtr node, string funcName,bool definite) {
	//cout << "parserParameterDeclaration" << endl;
	Tree* type_specifier = node->left;
	Tree* declarator = node->left->right;
	string typeName = type_specifier->left->content;
	if (typeName == "void") {
		printError(type_specifier->line, "Void can't definite parameter.");
	}
	//================================================
	//��ʱֻ���Ǳ�����������������Ϊ�β�
	string varName = declarator->left->content;
	varNode newnode;
	newnode.name = varName;
	newnode.type = typeName;
	if (definite) {
		newnode.num = innerCode.varNum++;
		recordStack.back().func.paralist.push_back(newnode);
	}

	funcPool[funcName].paralist.push_back(newnode);
	
	//���������β���ӵ���ǰ��ı�������
	recordStack.back().varMap.insert({varName,newnode});

	if(definite)
		innerCode.addCode(innerCode.createCodeforParameter(newnode));
}


TreePtr Parser::parserDeclaration(TreePtr node) {
	//cout << "at " << node->name << endl;
	//node = declaration
	TreePtr begin = node->left;	//begin:type_specifier
	if (begin->right->name == ";")
		return node->right;
	
	string vartype = begin->left->content;

	if (vartype == "void") {
		printError(begin->left->line,"void type can't assign to variable");	//����
 	}
	TreePtr decl = begin->right;	//init_declarator_list


	/*while (decl->right) {
		parserInitDeclarator(vartype, decl->right->right);
		decl = decl->left;
	}
	parserInitDeclarator(vartype, decl);*/
	parserInitDeclaratorList(vartype, decl);
	return node->right;

}

void Parser::parserInitDeclaratorList(string vartype, TreePtr node) {
	if (node->left->name == "init_declarator_list") {
		parserInitDeclaratorList(vartype, node->left);
	}
	else if (node->left->name == "init_declarator") {
		parserInitDeclarator(vartype, node->left);
	}

	if (node->right->name == ",") {
		parserInitDeclarator(vartype, node->right->right);
	}
}


//����������ʼ��
void Parser::parserInitDeclarator(string vartype, TreePtr node) {
	//cout << "at " << node->name << endl;
	TreePtr declarator = node->left;

	if (!declarator->right) {
		//��ȡ����������
		if (declarator->left->name == "IDENTIFIER") {
			TreePtr id = declarator->left;
			string var = id->content;
			if (!lookupCurruntVar(var)) {
				varNode newvar;
				newvar.name = var;
				newvar.type = vartype;
				newvar.num = innerCode.varNum++;
				recordStack.back().varMap.insert({ var,newvar });
			}
			else printError(declarator->left->line, "Variable multiple declaration.");
		}
		else {
			//��������
			if (declarator->left->right->name == "(") {
				string funcName = declarator->left->left->content;
				string funcType = vartype;
				if (recordStack.size() > 1) {
					printError(declarator->left->right->line, "Functinon declaration must at global environment.");
				}
				Tree* parameter_list = declarator->left->right->right;
				funcNode newFunc;
				newFunc.isdefinied = false;
				newFunc.name = funcName;
				newFunc.rtype = funcType;
				funcPool.insert({ funcName,newFunc });
				//���������β��б�
				parserParameterList(parameter_list,funcName,false);
			}
			//��������
			else if (declarator->left->right->name == "[") {
				string arrayName = declarator->left->left->content;
				string arrayType = vartype;
				Tree* assign_exp = declarator->left->right->right;
				varNode rnode = parserAssignmentExpression(assign_exp);

				if (rnode.type != "int") {
					printError(declarator->left->right->line,"Array size must be int.");
				}
				

				varNode tnode;
				if (arrayType == "int") {
					//����һ���µ���ʱ��������������Ĵ�С
					string tempname = "temp" + inttostr(innerCode.tempNum);
					++innerCode.tempNum;
					tnode = createVar(tempname, "int");

					recordStack.back().varMap.insert({ tempname,tnode });

					varNode tempVar3;
					string tempName3 = "temp" + inttostr(innerCode.tempNum);
					++innerCode.tempNum;
					tempVar3.name = tempName3;
					tempVar3.type = "int";
					recordStack.back().varMap.insert({ tempName3,tempVar3 });

					innerCode.addCode(tempName3 + " := #4");

					innerCode.addCode(tnode.name + " := " + tempName3 +" * " + rnode.name);
				}
				else if (arrayType == "double") {
					//����һ���µ���ʱ��������������Ĵ�С
					string tempname = "temp" + inttostr(innerCode.tempNum);
					++innerCode.tempNum;
					tnode = createVar(tempname, "int");

					recordStack.back().varMap.insert({ tempname,tnode });
					
					varNode tempVar3;
					string tempName3 = "temp" + inttostr(innerCode.tempNum);
					++innerCode.tempNum;
					tempVar3.name = tempName3;
					tempVar3.type = "int";
					recordStack.back().varMap.insert({ tempName3,tempVar3 });

					innerCode.addCode(tempName3 + " := #8");

					innerCode.addCode(tnode.name + " := " + tempName3 + " * " + rnode.name);
				}
				else if (arrayType == "bool") {
					tnode = rnode;
				}
				

				arrayNode anode;
				anode.name = arrayName;
				anode.type = arrayType;
				anode.num = innerCode.arrayNum++;
				innerCode.addCode("DEC " + innerCode.getarrayNodeName(anode) + " " + tnode.name);

				recordStack.back().arrayMap.insert({ arrayName,anode });
			}
		}
	}
	//�г�ʼ��
	else if (declarator->right->name == "=") {	
		//��ȡ����������
		varNode newvar;
		if (declarator->left->name == "IDENTIFIER") {
			TreePtr id = declarator->left;
			string var = id->content;
			if (!lookupCurruntVar(var)) {
				newvar.name = var;
				newvar.type = vartype;
				newvar.num = innerCode.varNum++;
				recordStack.back().varMap.insert({ var,newvar });
			}
			else printError(declarator->left->line, "Variable multiple declaration.");
		}
		else printError(declarator->left->line, "It's not a variable!");


		Tree* initializer = declarator->right->right;
		if (initializer == NULL) {
			printError(declarator->line, "Lack the initializer for variable.");
		}
		else {
			if (initializer->left->name == "assignment_expression") {
				varNode rnode = parserAssignmentExpression(initializer->left);
				innerCode.addCode(innerCode.createCodeforAssign(newvar,rnode));
				string rtype = rnode.type;
				if (rtype != vartype)
					printError(initializer->left->line, "Wrong type to variable " + declarator->left->content + ": " + 
					rtype + " to " + vartype);
			}
		}
	}
	else printError(declarator->right->line, "Wrong value to variable");
}

varNode Parser::parserAssignmentExpression(TreePtr assign_exp) {	//���ر����ڵ�

	//cout << "parserAssignmentExpression" << endl;

	if (assign_exp->left->name == "logical_or_expression") {
		TreePtr logical_or_exp = assign_exp->left;

		return parserLogicalOrExpression(logical_or_exp);
	}
	//��ֵ����
	else if(assign_exp->left->name == "unary_expression"){
		TreePtr unary_exp = assign_exp->left;
		string op = assign_exp->left->right->left->name;
		TreePtr next_assign_exp = assign_exp->left->right
			
			->right;
		varNode node1 = parserUnaryExpression(unary_exp);
		varNode node2 = parserAssignmentExpression(next_assign_exp);
		varNode node3;
		if (op == "=") {
			node3 = node2;
		}
		else {
			string tempname = "temp" + inttostr(innerCode.tempNum);
			++innerCode.tempNum;
			node3 = createVar(tempname, node1.type);

			recordStack.back().varMap.insert({ tempname,node3 });

			if (op == "MUL_ASSIGN") { //*=
				if (node1.type != node2.type) {
					printError(assign_exp->left->line, "Different type for two variables.");
				}
				innerCode.addCode(innerCode.createCodeforVar(tempname, "*", node1, node2));
			}
			else if (op == "DIV_ASSIGN") { //*=
				if (node1.type != node2.type) {
					printError(assign_exp->left->line, "Different type for two variables.");
				}
				innerCode.addCode(innerCode.createCodeforVar(tempname, "/", node1, node2));
			}
			else if (op == "MOD_ASSIGN") { //*=
				if (node1.type != "int" || node2.type != "int") {
					printError(assign_exp->left->line, "The two variables must be int.");
				}
				innerCode.addCode(innerCode.createCodeforVar(tempname, "%", node1, node2));
			}
			else if (op == "ADD_ASSIGN") { //*=
				if (node1.type != node2.type) {
					printError(assign_exp->left->line, "Different type for two variables.");
				}
				innerCode.addCode(innerCode.createCodeforVar(tempname, "+", node1, node2));
			}
			else if (op == "SUB_ASSIGN") { //*=
				if (node1.type != node2.type) {
					printError(assign_exp->left->line, "Different type for two variables.");
				}
				innerCode.addCode(innerCode.createCodeforVar(tempname, "-", node1, node2));
			}
			else if (op == "LEFT_ASSIGN") { //*=
				if (node1.type != "int" || node2.type != "int") {
					printError(assign_exp->left->line, "The two variables must be int.");
				}
				innerCode.addCode(innerCode.createCodeforVar(tempname, "<<", node1, node2));
			}
			else if (op == "RIGHT_ASSIGN") { //*=
				if (node1.type != "int" || node2.type != "int") {
					printError(assign_exp->left->line, "The two variables must be int.");
				}
				innerCode.addCode(innerCode.createCodeforVar(tempname, ">>", node1, node2));
			}
			else if (op == "AND_ASSIGN") { //*=
				if (node1.type != "int" || node2.type != "int") {
					printError(assign_exp->left->line, "The two variables must be int.");
				}
				innerCode.addCode(innerCode.createCodeforVar(tempname, "&", node1, node2));
			}
			else if (op == "XOR_ASSIGN") { //*=
				if (node1.type != "int" || node2.type != "int") {
					printError(assign_exp->left->line, "The two variables must be int.");
				}
				innerCode.addCode(innerCode.createCodeforVar(tempname, "^", node1, node2));
			}
			else if (op == "OR_ASSIGN") { //*=
				if (node1.type != "int" || node2.type != "int") {
					printError(assign_exp->left->line, "The two variables must be int.");
				}
				innerCode.addCode(innerCode.createCodeforVar(tempname, "|", node1, node2));
			}
		}

		innerCode.addCode(innerCode.createCodeforAssign(node1, node3));
		return node1;
	}
}

varNode Parser::parserLogicalOrExpression(TreePtr logical_or_exp) {

	if(logical_or_exp->left->name == "logical_and_expression"){
		TreePtr logical_and_exp = logical_or_exp->left;
		return parserLogicalAndExpression(logical_and_exp);
	}
	else if (logical_or_exp->left->name == "logical_or_expression") {
		//logical_or_expression -> logical_or_expression OR_OP logical_and_expression
		varNode node1 = parserLogicalOrExpression(logical_or_exp->left);
		varNode node2 = parserLogicalAndExpression(logical_or_exp->left->right->right);

		if (node1.type != "bool" || node2.type != "bool") {
			printError(logical_or_exp->left->right->line, "Logical Or operation should only used to bool. ");
		}

		string tempname = "temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;
		varNode newnode = createVar(tempname, node1.type);

		recordStack.back().varMap.insert({ tempname,newnode });
		innerCode.addCode(innerCode.createCodeforVar(tempname, "||", node1, node2));

		newnode.boolString = innerCode.getNodeName(node1) + " || " + innerCode.getNodeName(node2);

		return newnode;

	}

}

varNode Parser::parserLogicalAndExpression(TreePtr logical_and_exp) {
	
	if (logical_and_exp->left->name == "inclusive_or_expression") {
		Tree* inclusive_or_exp = logical_and_exp->left;
		return parserInclusiveOrExpression(inclusive_or_exp);
	}
	else if (logical_and_exp->left->name == "logical_and_expression") {
		varNode node1 = parserLogicalAndExpression(logical_and_exp->left);
		varNode node2 = parserInclusiveOrExpression(logical_and_exp->left->right->right);

		if (node1.type != "bool" || node2.type != "bool") {
			printError(logical_and_exp->left->right->line, "Logical And operation should only used to bool. ");
		}

		string tempname = "temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;
		varNode newnode = createVar(tempname, node1.type);
		recordStack.back().varMap.insert({ tempname,newnode });
	
		innerCode.addCode(innerCode.createCodeforVar(tempname, "&&", node1, node2));

		newnode.boolString = innerCode.getNodeName(node1) + " && " + innerCode.getNodeName(node2);

		return newnode;

	}
}

varNode Parser::parserInclusiveOrExpression(TreePtr inclusive_or_exp) {
	
	if (inclusive_or_exp->left->name == "exclusive_or_expression") {
		Tree* exclusive_or_exp = inclusive_or_exp->left;
		return parserExclusiveOrExpression(exclusive_or_exp);
	}
	else if (inclusive_or_exp->left->name == "inclusive_or_expression") {
		varNode node1 = parserInclusiveOrExpression(inclusive_or_exp->left);
		varNode node2 = parserExclusiveOrExpression(inclusive_or_exp->left->right->right);

		if (node1.type != "int" || node2.type != "int") {
			printError(inclusive_or_exp->left->right->line, "Inclusive Or operation should only used to int. ");
		}

		string tempname = "temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;
		varNode newnode = createVar(tempname, node1.type);
		recordStack.back().varMap.insert({ tempname,newnode });
		innerCode.addCode(innerCode.createCodeforVar(tempname, "|", node1, node2));
		return newnode;
	}
}

varNode Parser::parserExclusiveOrExpression(TreePtr exclusive_or_exp) {
	
	if (exclusive_or_exp->left->name == "and_expression") {
		Tree* and_exp = exclusive_or_exp->left;
		return parserAndExpression(and_exp);
	}
	else if (exclusive_or_exp->left->name == "exclusive_or_expression") {
		varNode node1 = parserExclusiveOrExpression(exclusive_or_exp->left);
		varNode node2 = parserAndExpression(exclusive_or_exp->left->right->right);

		if (node1.type != "int" || node2.type != "int") {
			printError(exclusive_or_exp->left->right->line, "Exclusive Or operation should only used to int. ");
		}

		string tempname = "temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;
		varNode newnode = createVar(tempname, node1.type);
		recordStack.back().varMap.insert({ tempname,newnode });
		innerCode.addCode(innerCode.createCodeforVar(tempname, "^", node1, node2));
		return newnode;
	}
}

varNode Parser::parserAndExpression(TreePtr and_exp) {
	if (and_exp->left->name == "equality_expression") {
		Tree* equality_exp = and_exp->left;
		return parserEqualityExpression(equality_exp);
	}
	else if (and_exp->left->name == "and_expression") {
		varNode node1 = parserAndExpression(and_exp->left);
		varNode node2 = parserEqualityExpression(and_exp->left->right->right);

		if (node1.type != "int" || node2.type != "int") {
			printError(and_exp->left->right->line, "And operation should only used to int. ");
		}

		string tempname = "temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;

		varNode newnode = createVar(tempname, node1.type);

		recordStack.back().varMap.insert({ tempname,newnode });
		innerCode.addCode(innerCode.createCodeforVar(tempname, "&", node1, node2));
		return newnode;
	}
}

varNode Parser::parserEqualityExpression(TreePtr equality_exp) {
	
	if (equality_exp->left->name == "relational_expression") {
		Tree* relational_exp = equality_exp->left;
		return parserRelationalExpression(relational_exp);
	}
	else if (equality_exp->left->right->name == "EQ_OP" || equality_exp->left->right->name == "NE_OP") {
		string op;
		if (equality_exp->left->right->name == "EQ_OP")
			op = "==";
		else op = "!=";

		varNode node1 = parserEqualityExpression(equality_exp->left);
		varNode node2 = parserRelationalExpression(equality_exp->left->right->right);

		if (node1.type != node2.type) {
			printError(equality_exp->left->right->line, "Different type for two variables.");
		}

		string tempname = "temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;

		varNode newnode = createVar(tempname, "bool");
		recordStack.back().varMap.insert({ tempname,newnode});
		innerCode.addCode(innerCode.createCodeforVar(tempname, op, node1, node2));

		newnode.boolString = innerCode.getNodeName(node1) + " " + op + " " + innerCode.getNodeName(node2);

		return newnode;
	}
}

varNode Parser::parserRelationalExpression(TreePtr relational_exp) {
	if (relational_exp->left->name == "shift_expression") {
		Tree* shift_exp = relational_exp->left;
		return parserShiftExpression(shift_exp);
	}
	else {
		string op = relational_exp->left->right->name;
		if (op == "LE_OP")
			op = "<=";
		else if (op == "GE_OP")
			op = ">=";
		if (op == ">" || op == "<" || op == ">=" || op == "<=") {
			varNode node1 = parserRelationalExpression(relational_exp->left);
			varNode node2 = parserShiftExpression(relational_exp->left->right->right);

			if (node1.type != node2.type) {
				printError(relational_exp->left->right->line, "Different type for two variables.");
			}

			string tempname = "temp" + inttostr(innerCode.tempNum);
			++innerCode.tempNum;

			varNode newnode = createVar(tempname, "bool");
			recordStack.back().varMap.insert({ tempname,newnode });
			innerCode.addCode(innerCode.createCodeforVar(tempname, op, node1, node2));

			newnode.boolString = innerCode.getNodeName(node1) + " " + op + " " + innerCode.getNodeName(node2);

			return newnode;
		}
	}
}

varNode Parser::parserShiftExpression(TreePtr shift_exp) {
	if (shift_exp->left->name == "additive_expression") {
		Tree* additive_exp = shift_exp->left;
		return parserAdditiveExpression(additive_exp);
	}
	else if (shift_exp->left->right->name == "LEFT_OP" || shift_exp->left->right->name == "RIGHT_OP") {
		string op;
		if (shift_exp->left->right->name == "LEFT_OP") {
			op = "<<";
		}
		else op = ">>";

		varNode node1 = parserShiftExpression(shift_exp->left);
		varNode node2 = parserAdditiveExpression(shift_exp->left->right->right);

		if (node1.type != "int" || node2.type != "int" ) {
			printError(shift_exp->left->right->line, "Shift operation should only used to int. ");
		}

		string tempname = "temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;

		varNode newnode = createVar(tempname, node1.type);

		recordStack.back().varMap.insert({ tempname,newnode });

		innerCode.addCode(innerCode.createCodeforVar(tempname, op, node1, node2));
		return newnode;
	}
}

varNode Parser::parserAdditiveExpression(TreePtr additive_exp) {
	if (additive_exp->left->name == "multiplicative_expression") {
		Tree* mult_exp = additive_exp->left;
		return parserMultiplicativeExpression(mult_exp);
	}
	else if (additive_exp->left->right->name == "+" || additive_exp->left->right->name == "-") {
		varNode node1 = parserAdditiveExpression(additive_exp->left);
		varNode node2 = parserMultiplicativeExpression(additive_exp->left->right->right);

		if (node1.type != node2.type) {
			printError(additive_exp->left->right->line, "Different type for two variables.");
		}

		string tempname = "temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;
		varNode newnode = createVar(tempname, node1.type);
		recordStack.back().varMap.insert({ tempname,newnode});

		innerCode.addCode(innerCode.createCodeforVar(tempname, additive_exp->left->right->name, node1, node2));
		return newnode;
	}
}

varNode Parser::parserMultiplicativeExpression(TreePtr mult_exp) {

	if (mult_exp->left->name == "unary_expression") {
		Tree* unary_exp = mult_exp->left;
		return parserUnaryExpression(unary_exp);
	}
	else if (mult_exp->left->right->name == "*" || mult_exp->left->right->name == "/" || 
		mult_exp->left->right->name == "%") {
		varNode node1 = parserMultiplicativeExpression(mult_exp->left);
		varNode node2 = parserUnaryExpression(mult_exp->left->right->right);

		if (node1.type != node2.type) {
			printError(mult_exp->left->right->line, "Different type for two variables.");
		}

		string tempname = "temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;
		varNode newNode = createVar(tempname, node1.type);
		recordStack.back().varMap.insert({ tempname,newNode });

		innerCode.addCode(innerCode.createCodeforVar(tempname, mult_exp->left->right->name,node1,node2));
		return newNode;

	}
}

varNode Parser::parserUnaryExpression(TreePtr unary_exp) {
	if (unary_exp->left->name == "postfix_expression") {
		Tree* post_exp = unary_exp->left;
		return parserPostfixExpression(post_exp);
	}
	else if (unary_exp->left->name == "INC_OP") {
		varNode rnode = parserUnaryExpression(unary_exp->left->right);
		if (rnode.type != "int")
			printError(unary_exp->left->right->line, "++ operation can only use for int type.");

		string tempname = "temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;
		varNode newNode = createVar(tempname, "int");
		recordStack.back().varMap.insert({ tempname,newNode });

		innerCode.addCode(tempname + " := #1");

		//����������ǵ�ַ
		if (rnode.useAddress) {
			innerCode.addCode("*" + rnode.name + " := *" + rnode.name + " + " + tempname);
		}
		else {
			innerCode.addCode(innerCode.getNodeName(rnode) + " := " + innerCode.getNodeName(rnode) + " + "  + tempname);
		}

		return rnode;

	}
	else if (unary_exp->left->name == "DEC_OP") {

		varNode rnode = parserUnaryExpression(unary_exp->left->right);
		if (rnode.type != "int")
			printError(unary_exp->left->right->line, "-- operation can only use for int type.");

		string tempname = "temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;
		varNode newNode = createVar(tempname, "int");
		recordStack.back().varMap.insert({ tempname,newNode });

		innerCode.addCode(tempname + " := #1");

		//����������ǵ�ַ
		if (rnode.useAddress) {
			innerCode.addCode("*" + rnode.name + " := *" + rnode.name + " - " + tempname);
		}
		else {
			innerCode.addCode(innerCode.getNodeName(rnode) + " := " + innerCode.getNodeName(rnode) + " - " + tempname);
		}

		return rnode;
	}
	else if (unary_exp->left->name == "unary_operator") {
		string op = unary_exp->left->left->name;
		varNode rnode = parserUnaryExpression(unary_exp->left->right);
		if (op == "+") {

			if (rnode.type != "int" && rnode.type != "double")
				printError(unary_exp->left->left->line, "operator '+' can only used to int or double");
			return rnode;
		}
		else if (op == "-") {

			if (rnode.type != "int" && rnode.type != "double")
				printError(unary_exp->left->left->line, "operator '-' can only used to int or double");

			string tempzeroname = "temp" + inttostr(innerCode.tempNum);
			++innerCode.tempNum;
			varNode newzeronode = createVar(tempzeroname, rnode.type);
			recordStack.back().varMap.insert({ tempzeroname,newzeronode });
			innerCode.addCode(tempzeroname + " := #0");

			string tempname = "temp" + inttostr(innerCode.tempNum);
			++innerCode.tempNum;
			varNode newnode = createVar(tempname, rnode.type);
			recordStack.back().varMap.insert({ tempname,newnode });


			if (rnode.useAddress) {
				innerCode.addCode(tempname + " := " + tempzeroname + " - *" + rnode.name);
			}
			else {
				innerCode.addCode(tempname + " := " + tempzeroname + " - " + innerCode.getNodeName(rnode));
			}
			return newnode;
		}
		else if (op == "~") {

		}
		else if (op == "!") {

		}
	}
}

varNode Parser::parserPostfixExpression(TreePtr post_exp) {
	//cout << "here" << endl;
	if (post_exp->left->name == "primary_expression") {
		Tree* primary_exp = post_exp->left;
		return parserPrimaryExpression(primary_exp);
	}
	else if (post_exp->left->right->name == "[") {
		//�������
		string arrayName = post_exp->left->left->left->content;
		Tree* expression = post_exp->left->right->right;
		varNode enode = parserExpression(expression);
		arrayNode anode = getArrayNode(arrayName);

		if (anode.num < 0)
			printError(post_exp->left->right->line, "Undifined array " + arrayName);

		varNode tempVar;
		string tempName = "temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;
		tempVar.name = tempName;
		tempVar.type = anode.type;
		tempVar.useAddress = true;
		recordStack.back().varMap.insert({tempName,tempVar});


		if (anode.type == "int" || anode.type == "double") {
			varNode tempVar2;
			string tempName2 = "temp" + inttostr(innerCode.tempNum);
			++innerCode.tempNum;
			tempVar2.name = tempName2;
			tempVar2.type = "int";
			recordStack.back().varMap.insert({ tempName2,tempVar2 });

			if (anode.type == "int") {

				varNode tempVar3;
				string tempName3 = "temp" + inttostr(innerCode.tempNum);
				++innerCode.tempNum;
				tempVar3.name = tempName3;
				tempVar3.type = "int";
				recordStack.back().varMap.insert({ tempName3,tempVar3 });

				innerCode.addCode(tempName3 + " := #4");

				innerCode.addCode(tempName2 + " := " + innerCode.getNodeName(enode) + " * " + tempName3);
			}
			else if (anode.type == "double") {
				varNode tempVar3;
				string tempName3 = "temp" + inttostr(innerCode.tempNum);
				++innerCode.tempNum;
				tempVar3.name = tempName3;
				tempVar3.type = "int";
				recordStack.back().varMap.insert({ tempName3,tempVar3 });

				innerCode.addCode(tempName3 + " := #8");

				innerCode.addCode(tempName2 + " := " + innerCode.getNodeName(enode) + " * " + tempName3);
			}

			innerCode.addCode(tempName + " := &" + innerCode.getarrayNodeName(anode) + " + " + innerCode.getNodeName(tempVar2));
			return tempVar;
		}

		innerCode.addCode(tempName + " := &" + innerCode.getarrayNodeName(anode) + " + " + innerCode.getNodeName(enode));
		return tempVar;
	}
	else if (post_exp->left->right->name == "(") {
		//��������
		string funcName = post_exp->left->left->left->content;
		varNode newNode;
		
		if (funcPool.find(funcName) == funcPool.end()) {
			printError(post_exp->left->left->left->line, "Undefined function " + funcName);
		}

		if (post_exp->left->right->right->name == "argument_expression_list") {
			Tree* argument_exp_list = post_exp->left->right->right;
			parserArgumentExpressionList(argument_exp_list, funcName);
			//cout << "funcCall" << endl;

		}

		funcNode func = funcPool[funcName];
		
		if (func.rtype == "void") {
			innerCode.addCode("CALL " + funcName);
		}
		else {
			string tempname = "temp" + inttostr(innerCode.tempNum);
			++innerCode.tempNum;

			newNode = createVar(tempname, funcPool[funcName].rtype);
			innerCode.addCode(tempname + " := CALL " + funcName);

		}

		return newNode;
		
	}
	else if (post_exp->left->right->name == "INC_OP") {
		varNode rnode = parserPostfixExpression(post_exp->left);

		if (rnode.type != "int")
			printError(post_exp->left->right->line, "++ operation can only use for int type.");

		string tempname = "temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;
		varNode newnode = createVar(tempname, "int");
		recordStack.back().varMap.insert({ tempname,newnode });

		string tempnameone = "temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;
		varNode newNode = createVar(tempnameone, "int");
		recordStack.back().varMap.insert({ tempnameone,newNode });

		innerCode.addCode(tempnameone + " := #1");

		//����������ǵ�ַ
		if (rnode.useAddress) {
			innerCode.addCode(tempname + " := *" + rnode.name);
			innerCode.addCode("*" + rnode.name + " := *" + rnode.name + " + " + tempnameone);
		}
		else {
			innerCode.addCode(tempname += " := " + innerCode.getNodeName(rnode));
			innerCode.addCode(innerCode.getNodeName(rnode) +  " := " + innerCode.getNodeName(rnode) + " + " + tempnameone);
		}

		return newnode;
	}
	else if (post_exp->left->right->name == "DEC_OP") {

		varNode rnode = parserPostfixExpression(post_exp->left);

		if (rnode.type != "int")
			printError(post_exp->left->right->line, "-- operation can only use for int type.");

		string tempname = "temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;
		varNode newnode = createVar(tempname, "int");
		recordStack.back().varMap.insert({ tempname,newnode });

		string tempnameone = "temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;
		varNode newNode = createVar(tempnameone, "int");
		recordStack.back().varMap.insert({ tempnameone,newNode });

		innerCode.addCode(tempnameone + " := #1");

		//����������ǵ�ַ
		if (rnode.useAddress) {
			innerCode.addCode(tempname + " := *" + rnode.name);
			innerCode.addCode("*" + rnode.name + " := *" + rnode.name + " - " + tempnameone);
		}
		else {
			innerCode.addCode(tempname += " := " + innerCode.getNodeName(rnode));
			innerCode.addCode(innerCode.getNodeName(rnode) + " := " + innerCode.getNodeName(rnode) + " - " + tempnameone);
		}

		return newnode;
	}
}

void Parser::parserArgumentExpressionList(TreePtr node, string funcName) {
	Tree* argu_exp_list = node->left;
	funcNode func = funcPool[funcName];
	int i = 0;
	while (argu_exp_list->name == "argument_expression_list") {
		varNode rnode = parserAssignmentExpression(argu_exp_list->right->right);

		innerCode.addCode(innerCode.createCodeforArgument(rnode));

		argu_exp_list = argu_exp_list->left;
		i++;
		if (func.paralist[func.paralist.size() - i].type != rnode.type) {
			printError(argu_exp_list->line, "Wrong type arguments to function " + funcName);
		}
	}
	varNode rnode = parserAssignmentExpression(argu_exp_list);
	innerCode.addCode(innerCode.createCodeforArgument(rnode));
	i++;
	if (func.paralist[func.paralist.size() - i].type != rnode.type) {
		printError(argu_exp_list->line, "Wrong type arguments to function " + funcName);
	}
	if (i != func.paralist.size()) {
		printError(argu_exp_list->line, "The number of arguments doesn't equal to the function parameters number.");
	}
}

varNode Parser::parserPrimaryExpression(TreePtr primary_exp) {
	if (primary_exp->left->name == "IDENTIFIER") {
		string content = primary_exp->left->content;
		varNode rnode = lookupNode(content);
		if (rnode.num < 0) {
			printError(primary_exp->left->line, "Undefined variable " + content);
		}
		return rnode;
	}
	else if (primary_exp->left->name == "TRUE" || primary_exp->left->name == "FALSE") {
		string content = primary_exp->left->content;
		string tempname = "temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;
		varNode newNode = createVar(tempname, "bool");
		recordStack.back().varMap.insert({ tempname,newNode });
		if(primary_exp->left->name == "TRUE") 
			innerCode.addCode(tempname + " := #1");
		else {
			innerCode.addCode(tempname + " := #0");
		}
		return newNode;
	}
	else if (primary_exp->left->name == "CONSTANT_INT") {
		string content = primary_exp->left->content;
		string tempname = "temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;
		
		varNode newNode = createVar(tempname, "int");
		recordStack.back().varMap.insert({ tempname,newNode });
		innerCode.addCode(tempname + " := #"  + content);
		return newNode;
	}
	else if (primary_exp->left->name == "CONSTANT_DOUBLE") {
		string content = primary_exp->left->content;
		string tempname = "temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;

		varNode newNode = createVar(tempname, "double");

		recordStack.back().varMap.insert({ tempname,newNode});
		innerCode.addCode(tempname + " := F" + content);
		return newNode;
	}
	else if (primary_exp->left->name == "(") {
		TreePtr expression = primary_exp->left->right;
		return parserExpression(expression);
	}
}


//ȫ�ֲ���
string Parser::lookupVar(string name) {
	int N = recordStack.size();
	for (int i = N - 1; i >= 0; i--) {
		if (recordStack[i].varMap.find(name) != recordStack[i].varMap.end())
			return recordStack[i].varMap[name].type;
	}
	return "";
}
//��ǰ�����
bool Parser::lookupCurruntVar(string name) {
	return recordStack.back().varMap.find(name) != recordStack.back().varMap.end();
}

struct varNode Parser::lookupNode(string name) {
	int N = recordStack.size();
	for (int i = N - 1; i >= 0; i--) {
		if (recordStack[i].varMap.find(name) != recordStack[i].varMap.end())
			return recordStack[i].varMap[name];
	}
	varNode temp;
	temp.num = -1;
	return temp;
}

string Parser::getFuncRType() {
	int N = recordStack.size();
	for (int i = N - 1; i >= 0; i--) {
		if (recordStack[i].isfunc)
			return recordStack[i].func.rtype;
	}
	return "";
}

string Parser::getArrayType(string name) {
	int N = recordStack.size();
	for (int i = N - 1; i >= 0; i--) {
		if (recordStack[i].arrayMap.find(name) != recordStack[i].arrayMap.end())
			return recordStack[i].arrayMap[name].type;
	}
	return "";
}

struct arrayNode Parser::getArrayNode(string name) {
	int N = recordStack.size();
	for (int i = N - 1; i >= 0; i--) {
		if (recordStack[i].arrayMap.find(name) != recordStack[i].arrayMap.end())
			return recordStack[i].arrayMap[name];
	}
	arrayNode temp;
	temp.num = -1;
	return temp;
}

int Parser::getBreakRecordNumber() {
	int N = recordStack.size();
	for (int i = N - 1; i >= 0; i--) {
		if (recordStack[i].canBreak)
			return i;
	}
	return -1;
}

void Parser::printError(int line, string error) {

	print_code();

	cout << "Error! line " << line << ": ";
	cout << error << endl;
	exit(1);
}

struct varNode Parser::createVar(string name, string type) {
	varNode var;
	var.name = name;
	var.type = type;
	var.num = -1;
	return var;
}

void Parser::print_map() {
	int N = recordStack.size();
	for (int i = N - 1; i >= 0; i--) {
		cout << "Block " << i << endl;
		for (auto it = recordStack[i].varMap.begin(); it != recordStack[i].varMap.end(); it++) {
			cout << "     " << it->first << " " << it->second.type << endl;
		}
	}
}

void Parser::print_code() {
	innerCode.printCode();
}

