#include"parser.h"
#include"record.h"
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
		node = parser_declaration(node);
	}
	else if (node->name == "function_definition") {
		node = parser_function_definition(node);
	}
	else if (node->name == "statement") {
		node = parser_statement(node);
	}

	//�������·���
	if (node != NULL) {
		parseTree(node->left);
		parseTree(node->right);
	}
}

TreePtr Parser::parser_statement(TreePtr node) {
	TreePtr next = node->left;
	if (node->left->name == "labeled_statement") {

	}
	if (node->left->name == "compound_statement") {
		parser_compound_statement(node->left);
	}
	if (node->left->name == "expression_statement") {
		parser_expression_statement(node->left);
	}
	if (node->left->name == "selection_statement") {
		parser_selection_statement(node->left);
	}
	if (node->left->name == "iteration_statement") {
		parser_iteration_statement(node->left);
	}
	if (node->left->name == "jump_statement") {
		parser_jump_statement(node->left);
	}

	return node->right;
}

void Parser::parser_jump_statement(TreePtr node) {
	if (node->left->name == "BREAK") {
		int num = getBreakRecordNumber();
		if (num < 0) {
			error(node->left->line, "This scope doesn't support break.");
		}
	
		innerCode.addCode("JUMP " + recordStack[num].breakLabelname);
	}
	else if (node->left->name == "RETURN") {
		string funcType = getFuncRType();
		if (node->left->right->name == "expression") {//return expression
			varNode R = parser_expression(node->left->right);
			innerCode.addCode(innerCode.createCodeforReturn(R));
			if (R.type != funcType) {
				error(node->left->right->line, "return type doesn't equal to function return type.");
			}
		}
		else if (node->left->right->name == ";"){//return ;
			innerCode.addCode("RETURN");
			if (funcType != "void") {
				error(node->left->right->line, "You should return " + recordStack.back().func.rtype);
			}
		}
	}
}

void Parser::parser_expression_statement(TreePtr node) {
	if (node->left->name == "expression") {
		parser_expression(node->left);
	}
}

varNode Parser::parser_expression(TreePtr node) {
	if (node->left->name == "expression") {
		return parser_expression(node->left);
	}
	else if (node->left->name == "assignment_expression") {
		return parser_assignment_expression(node->left);
	}
	if (node->right->name == ",") {
		return parser_assignment_expression(node->right->right);
	}
}

void Parser::parser_compound_statement(TreePtr node) {
	//������������compound_statement
	parseTree(node);
}

//if else
void Parser::parser_selection_statement(TreePtr node) {


	if (node->left->name == "IF") {
		if (node->left->right->right->right->right->right == NULL) {
			//���һ���µ�block
			Record newrecord;
			recordStack.push_back(newrecord);

			TreePtr expression = node->left->right->right;
			varNode exp_rnode = parser_expression(expression);
			TreePtr statement = node->left->right->right->right->right;

			string label1 = innerCode.getLabelName();
			string label2 = innerCode.getLabelName();

			if (exp_rnode.type == "bool") {
				innerCode.addCode("IF " + exp_rnode.boolString + " JUMP " + label1);
			}
			else {
				string tempzeroname = "temp" + inttostr(innerCode.tempNum);
				++innerCode.tempNum;
				varNode newznode = TempCreater(tempzeroname, "int");
				innerCode.addCode(tempzeroname + " := #0");

				innerCode.addCode("IF " + innerCode.getNodeName(exp_rnode) + " != " + tempzeroname + " JUMP " + label1);
			}
			
			innerCode.addCode("JUMP " + label2);
			innerCode.addCode("LABEL " + label1 + " :");


			parser_statement(statement);
			
			innerCode.addCode("LABEL " + label2 + " :");

			//������ӵ�block
			recordStack.pop_back();

		}
		else if (node->left->right->right->right->right->right->name == "ELSE") {
			//���һ���µ�block
			Record newrecord1;
			recordStack.push_back(newrecord1);

			TreePtr expression = node->left->right->right;
			varNode exp_rnode = parser_expression(expression);
			TreePtr statement1 = node->left->right->right->right->right;
			TreePtr statement2 = node->left->right->right->right->right->right->right;

			string label1 = innerCode.getLabelName();
			string label2 = innerCode.getLabelName();
			string label3 = innerCode.getLabelName();

			if (exp_rnode.type == "bool") {
				innerCode.addCode("IF " + exp_rnode.boolString + " JUMP " + label1);
			}
			else {
				string tempzeroname = "temp" + inttostr(innerCode.tempNum);
				++innerCode.tempNum;
				varNode newznode = TempCreater(tempzeroname, "int");
				innerCode.addCode(tempzeroname + " := #0");

				innerCode.addCode("IF " + innerCode.getNodeName(exp_rnode) + " != " + tempzeroname + " JUMP " + label1);
			}

			innerCode.addCode("JUMP " + label2);
			innerCode.addCode("LABEL " + label1 + " :");

			parser_statement(statement1);
			
			innerCode.addCode("JUMP " + label3);
			//������ӵ�block
			recordStack.pop_back();

			//else
			innerCode.addCode("LABEL " + label2 + " :");

			Record newrecord2;
			recordStack.push_back(newrecord2);

			parser_statement(statement2);

			innerCode.addCode("LABEL " + label3 + " :");

			//������ӵ�block
			recordStack.pop_back();

		}
	}
	else if (node->left->name == "SWITCH") {

	}
	
}

//ѭ�� while for do while
void Parser::parser_iteration_statement(TreePtr node) {
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

		recordStack.back().breakLabelname = label3;

		innerCode.addCode("LABEL " + label1 + " :");

		varNode var = parser_expression(expression);

		if (var.type == "bool") {  
			innerCode.addCode("IF " + var.boolString + " JUMP " + label2);
		}
		else {
			string tempzeroname = "temp" + inttostr(innerCode.tempNum);
			++innerCode.tempNum;
			varNode newznode = TempCreater(tempzeroname, "int");
			innerCode.addCode(tempzeroname + " := #0");

			innerCode.addCode("IF " + innerCode.getNodeName(var) + " != " + tempzeroname + " JUMP " + label2);
		}
		innerCode.addCode("JUMP " + label3);
		innerCode.addCode("LABEL " + label2 + " :");

		parser_statement(statement);

		innerCode.addCode("JUMP " + label1);
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

		recordStack.back().breakLabelname = label2;

		innerCode.addCode("LABEL " + label1 + " :");

		parser_statement(statement);

		varNode var = parser_expression(expression);

		if (var.type == "bool") {
			innerCode.addCode("IF " + var.boolString + " JUMP " + label1);
		}
		else {
			string tempzeroname = "temp" + inttostr(innerCode.tempNum);
			++innerCode.tempNum;
			varNode newznode = TempCreater(tempzeroname, "int");
			innerCode.addCode(tempzeroname + " := #0");

			innerCode.addCode("IF " + innerCode.getNodeName(var) + " != " + tempzeroname + " JUMP " + label1);
		}

		/*innerCode.addCode("JUMP " + label1);*/
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

				TreePtr exp_state1 = node->left->right->right;
				TreePtr exp_state2 = exp_state1->right;
				TreePtr statement = exp_state2->right->right;

				string label1 = innerCode.getLabelName();
				string label2 = innerCode.getLabelName();
				string label3 = innerCode.getLabelName();

				recordStack.back().breakLabelname = label3;

				if (exp_state1->left->name == "expression") {
					parser_expression(exp_state1->left);
				}
				innerCode.addCode("LABEL " + label1 + " :");

				varNode var;
				if (exp_state2->left->name == "expression") {
					var = parser_expression(exp_state2->left);
					if (var.type == "bool") {
						innerCode.addCode("IF " + var.boolString + " JUMP " + label2);
					}
					else {
						string tempzeroname = "temp" + inttostr(innerCode.tempNum);
						++innerCode.tempNum;
						varNode newznode = TempCreater(tempzeroname, "int");
						innerCode.addCode(tempzeroname + " := #0");

						innerCode.addCode("IF " + innerCode.getNodeName(var) + " != " + tempzeroname + " JUMP " + label2);
					}
				}
				else {
					innerCode.addCode("JUMP " + label2);
				}

				innerCode.addCode("JUMP " + label3);
				innerCode.addCode("LABEL " + label2 + " :");

				parser_statement(statement);

				innerCode.addCode("JUMP " + label1);
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

				TreePtr exp_state1 = node->left->right->right;
				TreePtr exp_state2 = exp_state1->right;
				TreePtr exp = exp_state2->right;
				TreePtr statement = exp->right->right;

				string label1 = innerCode.getLabelName();
				string label2 = innerCode.getLabelName();
				string label3 = innerCode.getLabelName();

				recordStack.back().breakLabelname = label3;

				if (exp_state1->left->name == "expression") {
					parser_expression(exp_state1->left);
				}
				innerCode.addCode("LABEL " + label1 + " :");

				varNode var;
				if (exp_state2->left->name == "expression") {
					var = parser_expression(exp_state2->left);

					if (var.type == "bool") {
						innerCode.addCode("IF " + var.boolString + " JUMP " + label2);
					}
					else {
						string tempzeroname = "temp" + inttostr(innerCode.tempNum);
						++innerCode.tempNum;
						varNode newznode = TempCreater(tempzeroname, "int");
						innerCode.addCode(tempzeroname + " := #0");

						innerCode.addCode("IF " + innerCode.getNodeName(var) + " != " + tempzeroname + " JUMP " + label2);
					}
				}
				else {
					innerCode.addCode("JUMP " + label2);
				}

				innerCode.addCode("JUMP " + label3);
				innerCode.addCode("LABEL " + label2 + " :");

				parser_statement(statement);

				parser_expression(exp);

				innerCode.addCode("JUMP " + label1);
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

				recordStack.back().breakLabelname = label3;

				parser_declaration(declaration);
				innerCode.addCode("LABEL " + label1 + " :");

				varNode var;
				if (expression_statement->left->name == "expression") {

					var = parser_expression(expression_statement->left);

					if (var.type == "bool") {
						innerCode.addCode("IF " + var.boolString + " JUMP " + label2);
					}
					else {
						string tempzeroname = "temp" + inttostr(innerCode.tempNum);
						++innerCode.tempNum;
						varNode newznode = TempCreater(tempzeroname, "int");
						innerCode.addCode(tempzeroname + " := #0");

						innerCode.addCode("IF " + innerCode.getNodeName(var) + " != " + tempzeroname + " JUMP " + label2);
					}
				}
				else {
					innerCode.addCode("JUMP " + label2);
				}
				innerCode.addCode("JUMP " + label3);
				innerCode.addCode("LABEL " + label2 + " :");

				parser_statement(statement);

				//cout << "here" << endl;
				innerCode.addCode("JUMP " + label1);
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

				recordStack.back().breakLabelname = label3;

				parser_declaration(declaration);
				innerCode.addCode("LABEL " + label1 + " :");

				varNode var;
				if (expression_statement->left->name == "expression") {
					var = parser_expression(expression_statement->left);

					if (var.type == "bool") {
						innerCode.addCode("IF " + var.boolString + " JUMP " + label2);
					}
					else {
						string tempzeroname = "temp" + inttostr(innerCode.tempNum);
						++innerCode.tempNum;
						varNode newznode = TempCreater(tempzeroname, "int");
						innerCode.addCode(tempzeroname + " := #0");

						innerCode.addCode("IF " + innerCode.getNodeName(var) + " != " + tempzeroname + " JUMP " + label2);
					}
				}
				else {
					innerCode.addCode("JUMP " + label2);
				}
				innerCode.addCode("JUMP " + label3);
				innerCode.addCode("LABEL " + label2 + " :");

				parser_statement(statement);

				parser_expression(expression);
				//cout << "here" << endl;
				innerCode.addCode("JUMP " + label1);
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
TreePtr Parser::parser_function_definition(TreePtr node) {
	TreePtr type_specifier = node->left;
	TreePtr declarator = node->left->right;
	TreePtr compound_statement = declarator->right;
	
	string funcType = type_specifier->left->content;
	string funcName = declarator->left->left->content;

	bool isdeclared = false;
	funcNode declarFunc;
	if (funcPool.find(funcName) != funcPool.end()) {
		//�����ظ�����
		if (funcPool[funcName].isdefinied) {
			error(declarator->left->left->line, "Function " + funcName + " is duplicated definition.");
		}
		//������������������û�ж���
		else {
			isdeclared = true;
			//��ɾ�����������еĺ���������
			declarFunc = funcPool[funcName];
			funcPool.erase(funcPool.find(funcName));
		}
	}

	//�����µ�record
	Record funRecord;
	funRecord.isfunc = true;
	funRecord.func.name = funcName;
	funRecord.func.rtype = funcType;
	funRecord.func.isdefinied = true;
	//��������¼�ڿ��ڲ���ӵ�������
	recordStack.push_back(funRecord);
	funcPool.insert({funcName,funRecord.func});

	innerCode.addCode("FUNCTION " + funcName + " :");

	//��ȡ�����β��б�
	if(declarator->left->right->right->name == "parameter_list")
		parser_parameter_list(declarator->left->right->right, funcName,true);

	//��ʱ�������е�func�Ѿ�����˲����б�
	funcNode func = funcPool[funcName];
	//���������������������ȽϺ����Ĳ����б�ͷ�������
	if (isdeclared) {
		if (func.rtype != declarFunc.rtype) {
			error(type_specifier->left->line, "Function return type doesn't equal to the function declared before.");
		}
		cout << funRecord.func.paralist.size() << endl;
		if (func.paralist.size() != declarFunc.paralist.size()) {
			error(declarator->left->right->right->line, "The number of function parameters doesn't equal to the function declared before.");
		}
		for (int i = 0; i < funRecord.func.paralist.size(); i++) {
			if (func.paralist[i].type != declarFunc.paralist[i].type)
				error(declarator->left->right->right->line, "The parameter " + funRecord.func.paralist[i].name + "'s type doesn't equal to the function declared before." );
		}
	}
	//����Block��func�Ĳ����б�
	funRecord.func = func;
	//��������������
	parser_compound_statement(compound_statement);

	//���������󣬵�����Ӧ��block
	recordStack.pop_back();

	return node->right;
}

//��ȡ�����β��б�����������Ҫ��ȡ�βΣ���������Ҫ
void Parser::parser_parameter_list(TreePtr node,string funcName,bool definite) {
	if (node->left->name == "parameter_list") {
		parser_parameter_list(node->left, funcName,definite);
	}
	else if (node->left->name == "parameter_declaration") {
		parser_parameter_declaration(node->left,funcName,definite);
	}

	if (node->right->name == ",") {
		parser_parameter_declaration(node->right->right, funcName,definite);
	}
}

//��ȡ�����β�����,����������Ҫ��ȡ�βΣ���������Ҫ
void Parser::parser_parameter_declaration(TreePtr node, string funcName,bool definite) {
	//cout << "parser_parameter_declaration" << endl;
	TreePtr type_specifier = node->left;
	TreePtr declarator = node->left->right;
	string typeName = type_specifier->left->content;
	if (typeName == "void") {
		error(type_specifier->line, "Void can't definite parameter.");
	}
	//================================================
	//��ʱֻ���Ǳ�����������������Ϊ�β�
	string varName = declarator->left->content;
	varNode Node1;
	Node1.name = varName;
	Node1.type = typeName;
	if (definite) {
		Node1.num = innerCode.varNum++;
		recordStack.back().func.paralist.push_back(Node1);
	}

	funcPool[funcName].paralist.push_back(Node1);
	
	//���������β���ӵ���ǰ��ı�������
	recordStack.back().varInsert(Node1);
	if(definite)
		innerCode.addCode(innerCode.createCodeforParameter(Node1));
}


TreePtr Parser::parser_declaration(TreePtr node) {
	//cout << "at " << node->name << endl;
	//node = declaration
	TreePtr begin = node->left;	//begin:type_specifier
	if (begin->right->name == ";")
		return node->right;
	
	string vartype = begin->left->content;

	if (vartype == "void") {
		error(begin->left->line,"void type can't assign to variable");	//����
 	}
	TreePtr decl = begin->right;	//init_declarator_list


	/*while (decl->right) {
		parser_init_declarator(vartype, decl->right->right);
		decl = decl->left;
	}
	parser_init_declarator(vartype, decl);*/
	parser_init_declarator_list(vartype, decl);
	return node->right;

}

void Parser::parser_init_declarator_list(string vartype, TreePtr node) {
	if (node->left->name == "init_declarator_list") {
		parser_init_declarator_list(vartype, node->left);
	}
	else if (node->left->name == "init_declarator") {
		parser_init_declarator(vartype, node->left);
	}

	if (node->right->name == ",") {
		parser_init_declarator(vartype, node->right->right);
	}
}


//����������ʼ��
void Parser::parser_init_declarator(string vartype, TreePtr node) {
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
				recordStack.back().varInsert(newvar);
			}
			else error(declarator->left->line, "Variable multiple declaration.");
		}
		else {
			//��������
			if (declarator->left->right->name == "(") {
				string funcName = declarator->left->left->content;
				string funcType = vartype;
				if (recordStack.size() > 1) {
					error(declarator->left->right->line, "Functinon declaration must at global environment.");
				}
				TreePtr parameter_list = declarator->left->right->right;
				funcNode newFunc;
				newFunc.isdefinied = false;
				newFunc.name = funcName;
				newFunc.rtype = funcType;
				funcPool.insert({ funcName,newFunc });
				//���������β��б�
				parser_parameter_list(parameter_list,funcName,false);
			}
			//��������
			else if (declarator->left->right->name == "[") {
				string arrayName = declarator->left->left->content;
				string arrayType = vartype;
				TreePtr assign_exp = declarator->left->right->right;
				varNode R = parser_assignment_expression(assign_exp);

				if (R.type != "int") {
					error(declarator->left->right->line,"Array size must be int.");
				}
				

				varNode tnode;
				if (arrayType == "int") {
					//����һ���µ���ʱ��������������Ĵ�С
					string Tname = "temp" + inttostr(innerCode.tempNum);
					++innerCode.tempNum;
					tnode = TempCreater(Tname, "int");

					recordStack.back().varInsert(tnode);
					varNode tempVar3;
					string tempName3 = "temp" + inttostr(innerCode.tempNum);
					++innerCode.tempNum;
					tempVar3.name = tempName3;
					tempVar3.type = "int";
					recordStack.back().varInsert(tempVar3);
					innerCode.addCode(tempName3 + " := #4");

					innerCode.addCode(tnode.name + " := " + tempName3 +" * " + R.name);
				}
				else if (arrayType == "double") {
					//����һ���µ���ʱ��������������Ĵ�С
					string Tname = "temp" + inttostr(innerCode.tempNum);
					++innerCode.tempNum;
					tnode = TempCreater(Tname, "int");

					recordStack.back().varInsert(tnode);
					varNode tempVar3;
					string tempName3 = "temp" + inttostr(innerCode.tempNum);
					++innerCode.tempNum;
					tempVar3.name = tempName3;
					tempVar3.type = "int";
					recordStack.back().varInsert(tempVar3);
					innerCode.addCode(tempName3 + " := #8");

					innerCode.addCode(tnode.name + " := " + tempName3 + " * " + R.name);
				}
				else if (arrayType == "bool") {
					tnode = R;
				}
				

				arrayNode anode;
				anode.name = arrayName;
				anode.type = arrayType;
				anode.num = innerCode.arrayNum++;
				innerCode.addCode("DEC " + innerCode.getarrayNodeName(anode) + " " + tnode.name);

				recordStack.back().arrayInsert(anode);
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
				recordStack.back().varInsert(newvar);
			}
			else error(declarator->left->line, "Variable multiple declaration.");
		}
		else error(declarator->left->line, "It's not a variable!");


		TreePtr initializer = declarator->right->right;
		if (initializer == NULL) {
			error(declarator->line, "Lack the initializer for variable.");
		}
		else {
			if (initializer->left->name == "assignment_expression") {
				varNode R = parser_assignment_expression(initializer->left);
				innerCode.addCode(innerCode.createCodeforAssign(newvar,R));
				string rtype = R.type;
				if (rtype != vartype)
					error(initializer->left->line, "Wrong type to variable " + declarator->left->content + ": " + 
					rtype + " to " + vartype);
			}
		}
	}
	else error(declarator->right->line, "Wrong value to variable");
}

varNode Parser::parser_assignment_expression(TreePtr assign_exp) {	//���ر����ڵ�

	//cout << "parser_assignment_expression" << endl;

	if (assign_exp->left->name == "logical_or_expression") {
		TreePtr logical_or_exp = assign_exp->left;

		return parser_logical_or_expression(logical_or_exp);
	}
	//��ֵ����
	else if(assign_exp->left->name == "unary_expression"){
		TreePtr unary_exp = assign_exp->left;
		string Operater = assign_exp->left->right->left->name;
		TreePtr next_assign_exp = assign_exp->left->right->right;
		varNode node1 = parser_unary_expression(unary_exp);
		varNode node2 = parser_assignment_expression(next_assign_exp);
		varNode node3;
		if (Operater == "=") {
			node3 = node2;
		}
		else {
			string Tname = "temp" + inttostr(innerCode.tempNum);
			++innerCode.tempNum;
			node3 = TempCreater(Tname, node1.type);

			recordStack.back().varInsert(node3);
			if (Operater == "MUL_ASSIGN") { //*=
				if (node1.type != node2.type) {
					error(assign_exp->left->line, "Different type for two variables.");
				}
				innerCode.addCode(innerCode.createCodeforVar(Tname, "*", node1, node2));
			}
			else if (Operater == "DIV_ASSIGN") { //*=
				if (node1.type != node2.type) {
					error(assign_exp->left->line, "Different type for two variables.");
				}
				innerCode.addCode(innerCode.createCodeforVar(Tname, "/", node1, node2));
			}
			else if (Operater == "MOD_ASSIGN") { //*=
				if (node1.type != "int" || node2.type != "int") {
					error(assign_exp->left->line, "The two variables must be int.");
				}
				innerCode.addCode(innerCode.createCodeforVar(Tname, "%", node1, node2));
			}
			else if (Operater == "ADD_ASSIGN") { //*=
				if (node1.type != node2.type) {
					error(assign_exp->left->line, "Different type for two variables.");
				}
				innerCode.addCode(innerCode.createCodeforVar(Tname, "+", node1, node2));
			}
			else if (Operater == "SUB_ASSIGN") { //*=
				if (node1.type != node2.type) {
					error(assign_exp->left->line, "Different type for two variables.");
				}
				innerCode.addCode(innerCode.createCodeforVar(Tname, "-", node1, node2));
			}
			else if (Operater == "LEFT_ASSIGN") { //*=
				if (node1.type != "int" || node2.type != "int") {
					error(assign_exp->left->line, "The two variables must be int.");
				}
				innerCode.addCode(innerCode.createCodeforVar(Tname, "<<", node1, node2));
			}
			else if (Operater == "RIGHT_ASSIGN") { //*=
				if (node1.type != "int" || node2.type != "int") {
					error(assign_exp->left->line, "The two variables must be int.");
				}
				innerCode.addCode(innerCode.createCodeforVar(Tname, ">>", node1, node2));
			}
			else if (Operater == "AND_ASSIGN") { //*=
				if (node1.type != "int" || node2.type != "int") {
					error(assign_exp->left->line, "The two variables must be int.");
				}
				innerCode.addCode(innerCode.createCodeforVar(Tname, "&", node1, node2));
			}
			else if (Operater == "XOR_ASSIGN") { //*=
				if (node1.type != "int" || node2.type != "int") {
					error(assign_exp->left->line, "The two variables must be int.");
				}
				innerCode.addCode(innerCode.createCodeforVar(Tname, "^", node1, node2));
			}
			else if (Operater == "OR_ASSIGN") { //*=
				if (node1.type != "int" || node2.type != "int") {
					error(assign_exp->left->line, "The two variables must be int.");
				}
				innerCode.addCode(innerCode.createCodeforVar(Tname, "|", node1, node2));
			}
		}

		innerCode.addCode(innerCode.createCodeforAssign(node1, node3));
		return node1;
	}
}

varNode Parser::parser_logical_or_expression(TreePtr logical_or_exp) {

	if(logical_or_exp->left->name == "logical_and_expression"){
		TreePtr logical_and_exp = logical_or_exp->left;
		return parser_logical_and_expression(logical_and_exp);
	}
	else if (logical_or_exp->left->name == "logical_or_expression") {
		//logical_or_expression -> logical_or_expression OR_OP logical_and_expression
		varNode node1 = parser_logical_or_expression(logical_or_exp->left);
		varNode node2 = parser_logical_and_expression(logical_or_exp->left->right->right);

		if (node1.type != "bool" || node2.type != "bool") {
			error(logical_or_exp->left->right->line, "Logical Or operation should only used to bool. ");
		}

		string Tname = "temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;
		varNode Node1 = TempCreater(Tname, node1.type);

		recordStack.back().varInsert(Node1);
		innerCode.addCode(innerCode.createCodeforVar(Tname, "||", node1, node2));

		Node1.boolString = innerCode.getNodeName(node1) + " || " + innerCode.getNodeName(node2);

		return Node1;

	}

}

varNode Parser::parser_logical_and_expression(TreePtr logical_and_exp) {
	
	if (logical_and_exp->left->name == "inclusive_or_expression") {
		TreePtr inclusive_or_exp = logical_and_exp->left;
		return parser_inclusive_or_expression(inclusive_or_exp);
	}
	else if (logical_and_exp->left->name == "logical_and_expression") {
		varNode node1 = parser_logical_and_expression(logical_and_exp->left);
		varNode node2 = parser_inclusive_or_expression(logical_and_exp->left->right->right);

		if (node1.type != "bool" || node2.type != "bool") {
			error(logical_and_exp->left->right->line, "Logical And operation should only used to bool. ");
		}

		string Tname = "temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;
		varNode Node1 = TempCreater(Tname, node1.type);
		recordStack.back().varInsert(Node1);
		innerCode.addCode(innerCode.createCodeforVar(Tname, "&&", node1, node2));

		Node1.boolString = innerCode.getNodeName(node1) + " && " + innerCode.getNodeName(node2);

		return Node1;

	}
}

varNode Parser::parser_inclusive_or_expression(TreePtr inclusive_or_exp) {
	
	if (inclusive_or_exp->left->name == "exclusive_or_expression") {
		TreePtr exclusive_or_exp = inclusive_or_exp->left;
		return parser_exclusive_or_expression(exclusive_or_exp);
	}
	else if (inclusive_or_exp->left->name == "inclusive_or_expression") {
		varNode node1 = parser_inclusive_or_expression(inclusive_or_exp->left);
		varNode node2 = parser_exclusive_or_expression(inclusive_or_exp->left->right->right);

		if (node1.type != "int" || node2.type != "int") {
			error(inclusive_or_exp->left->right->line, "Inclusive Or operation should only used to int. ");
		}

		string Tname = "temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;
		varNode Node1 = TempCreater(Tname, node1.type);
		recordStack.back().varInsert(Node1);
		innerCode.addCode(innerCode.createCodeforVar(Tname, "|", node1, node2));
		return Node1;
	}
}

varNode Parser::parser_exclusive_or_expression(TreePtr exclusive_or_exp) {
	
	if (exclusive_or_exp->left->name == "and_expression") {
		TreePtr and_exp = exclusive_or_exp->left;
		return parser_and_expression(and_exp);
	}
	else if (exclusive_or_exp->left->name == "exclusive_or_expression") {
		varNode node1 = parser_exclusive_or_expression(exclusive_or_exp->left);
		varNode node2 = parser_and_expression(exclusive_or_exp->left->right->right);

		if (node1.type != "int" || node2.type != "int") {
			error(exclusive_or_exp->left->right->line, "Exclusive Or operation should only used to int. ");
		}

		string Tname = "temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;
		varNode Node1 = TempCreater(Tname, node1.type);
		recordStack.back().varInsert(Node1);
		innerCode.addCode(innerCode.createCodeforVar(Tname, "^", node1, node2));
		return Node1;
	}
}

varNode Parser::parser_and_expression(TreePtr and_exp) {
	if (and_exp->left->name == "equality_expression") {
		TreePtr equality_exp = and_exp->left;
		return parser_equality_expression(equality_exp);
	}
	else if (and_exp->left->name == "and_expression") {
		varNode node1 = parser_and_expression(and_exp->left);
		varNode node2 = parser_equality_expression(and_exp->left->right->right);

		if (node1.type != "int" || node2.type != "int") {
			error(and_exp->left->right->line, "And operation should only used to int. ");
		}

		string Tname = "temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;

		varNode Node1 = TempCreater(Tname, node1.type);

		recordStack.back().varInsert(Node1);
		innerCode.addCode(innerCode.createCodeforVar(Tname, "&", node1, node2));
		return Node1;
	}
}

varNode Parser::parser_equality_expression(TreePtr equality_exp) {
	
	if (equality_exp->left->name == "relational_expression") {
		TreePtr relational_exp = equality_exp->left;
		return parser_relational_expression(relational_exp);
	}
	else if (equality_exp->left->right->name == "EQ_OP" || equality_exp->left->right->name == "NE_OP") {
		string Operater;
		if (equality_exp->left->right->name == "EQ_OP")
			Operater = "==";
		else Operater = "!=";

		varNode node1 = parser_equality_expression(equality_exp->left);
		varNode node2 = parser_relational_expression(equality_exp->left->right->right);

		if (node1.type != node2.type) {
			error(equality_exp->left->right->line, "Different type for two variables.");
		}

		string Tname = "temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;

		varNode Node1 = TempCreater(Tname, "bool");
		recordStack.back().varInsert(Node1);
		innerCode.addCode(innerCode.createCodeforVar(Tname, Operater, node1, node2));

		Node1.boolString = innerCode.getNodeName(node1) + " " + Operater + " " + innerCode.getNodeName(node2);

		return Node1;
	}
}

varNode Parser::parser_relational_expression(TreePtr relational_exp) {
	if (relational_exp->left->name == "shift_expression") {
		TreePtr shift_exp = relational_exp->left;
		return parser_shift_expression(shift_exp);
	}
	else {
		string Operater = relational_exp->left->right->name;
		if (Operater == "LE_OP")
			Operater = "<=";
		else if (Operater == "GE_OP")
			Operater = ">=";
		if (Operater == ">" || Operater == "<" || Operater == ">=" || Operater == "<=") {
			varNode node1 = parser_relational_expression(relational_exp->left);
			varNode node2 = parser_shift_expression(relational_exp->left->right->right);

			if (node1.type != node2.type) {
				error(relational_exp->left->right->line, "Different type for two variables.");
			}

			string Tname = "temp" + inttostr(innerCode.tempNum);
			++innerCode.tempNum;

			varNode Node1 = TempCreater(Tname, "bool");
			recordStack.back().varInsert(Node1);
			innerCode.addCode(innerCode.createCodeforVar(Tname, Operater, node1, node2));

			Node1.boolString = innerCode.getNodeName(node1) + " " + Operater + " " + innerCode.getNodeName(node2);

			return Node1;
		}
	}
}

varNode Parser::parser_shift_expression(TreePtr shift_exp) {
	if (shift_exp->left->name == "additive_expression") {
		TreePtr additive_exp = shift_exp->left;
		return parser_additive_expression(additive_exp);
	}
	else if (shift_exp->left->right->name == "LEFT_OP" || shift_exp->left->right->name == "RIGHT_OP") {
		string Operater;
		if (shift_exp->left->right->name == "LEFT_OP") {
			Operater = "<<";
		}
		else Operater = ">>";

		varNode node1 = parser_shift_expression(shift_exp->left);
		varNode node2 = parser_additive_expression(shift_exp->left->right->right);

		if (node1.type != "int" || node2.type != "int" ) {
			error(shift_exp->left->right->line, "Shift operation should only used to int. ");
		}

		string Tname = "temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;

		varNode Node1 = TempCreater(Tname, node1.type);

		recordStack.back().varInsert(Node1);
		innerCode.addCode(innerCode.createCodeforVar(Tname, Operater, node1, node2));
		return Node1;
	}
}

varNode Parser::parser_additive_expression(TreePtr additive_exp) {
	if (additive_exp->left->name == "multiplicative_expression") {
		TreePtr mult_exp = additive_exp->left;
		return parser_multiplicative_expression(mult_exp);
	}
	else if (additive_exp->left->right->name == "+" || additive_exp->left->right->name == "-") {
		varNode node1 = parser_additive_expression(additive_exp->left);
		varNode node2 = parser_multiplicative_expression(additive_exp->left->right->right);

		if (node1.type != node2.type) {
			error(additive_exp->left->right->line, "Different type for two variables.");
		}

		string Tname = "temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;
		varNode Node1 = TempCreater(Tname, node1.type);
		recordStack.back().varInsert(Node1);
		innerCode.addCode(innerCode.createCodeforVar(Tname, additive_exp->left->right->name, node1, node2));
		return Node1;
	}
}

varNode Parser::parser_multiplicative_expression(TreePtr mult_exp) {

	if (mult_exp->left->name == "unary_expression") {
		TreePtr unary_exp = mult_exp->left;
		return parser_unary_expression(unary_exp);
	}
	else if (mult_exp->left->right->name == "*" || mult_exp->left->right->name == "/" || 
		mult_exp->left->right->name == "%") {
		varNode node1 = parser_multiplicative_expression(mult_exp->left);
		varNode node2 = parser_unary_expression(mult_exp->left->right->right);

		if (node1.type != node2.type) {
			error(mult_exp->left->right->line, "Different type for two variables.");
		}

		string Tname = "temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;
		varNode NNode = TempCreater(Tname, node1.type);
		recordStack.back().varInsert(NNode);
		innerCode.addCode(innerCode.createCodeforVar(Tname, mult_exp->left->right->name,node1,node2));
		return NNode;

	}
}

varNode Parser::parser_unary_expression(TreePtr unary_exp) {
	if (unary_exp->left->name == "postfix_expression") {
		TreePtr post_exp = unary_exp->left;
		return parser_postfix_expression(post_exp);
	}
	else if (unary_exp->left->name == "INC_OP") {
		varNode R = parser_unary_expression(unary_exp->left->right);
		if (R.type != "int")
			error(unary_exp->left->right->line, "++ operation can only use for int type.");

		string Tname = "temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;
		varNode NNode = TempCreater(Tname, "int");
		recordStack.back().varInsert(NNode);
		innerCode.addCode(Tname + " := #1");

		//����������ǵ�ַ
		if (R.useAddress) {
			innerCode.addCode("*" + R.name + " := *" + R.name + " + " + Tname);
		}
		else {
			innerCode.addCode(innerCode.getNodeName(R) + " := " + innerCode.getNodeName(R) + " + "  + Tname);
		}

		return R;

	}
	else if (unary_exp->left->name == "DEC_OP") {

		varNode R = parser_unary_expression(unary_exp->left->right);
		if (R.type != "int")
			error(unary_exp->left->right->line, "-- operation can only use for int type.");

		string Tname = "temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;
		varNode NNode = TempCreater(Tname, "int");
		recordStack.back().varInsert(NNode);
		innerCode.addCode(Tname + " := #1");

		//����������ǵ�ַ
		if (R.useAddress) {
			innerCode.addCode("*" + R.name + " := *" + R.name + " - " + Tname);
		}
		else {
			innerCode.addCode(innerCode.getNodeName(R) + " := " + innerCode.getNodeName(R) + " - " + Tname);
		}

		return R;
	}
	else if (unary_exp->left->name == "unary_operator") {
		string Operater = unary_exp->left->left->name;
		varNode R = parser_unary_expression(unary_exp->left->right);
		if (Operater == "+") {

			if (R.type != "int" && R.type != "double")
				error(unary_exp->left->left->line, "operator '+' can only used to int or double");
			return R;
		}
		else if (Operater == "-") {

			if (R.type != "int" && R.type != "double")
				error(unary_exp->left->left->line, "operator '-' can only used to int or double");

			string tempzeroname = "temp" + inttostr(innerCode.tempNum);
			++innerCode.tempNum;
			varNode ZNode = TempCreater(tempzeroname, R.type);
			recordStack.back().varInsert(ZNode);
			innerCode.addCode(tempzeroname + " := #0");

			string Tname = "temp" + inttostr(innerCode.tempNum);
			++innerCode.tempNum;
			varNode NNode = TempCreater(Tname, R.type);
			recordStack.back().varInsert(NNode);

			if (R.useAddress) {
				innerCode.addCode(Tname + " := " + tempzeroname + " - *" + R.name);
			}
			else {
				innerCode.addCode(Tname + " := " + tempzeroname + " - " + innerCode.getNodeName(R));
			}
			return NNode;
		}
		else if (Operater == "~") {

		}
		else if (Operater == "!") {

		}
	}
}

varNode Parser::parser_postfix_expression(TreePtr post_exp) {
	//cout << "here" << endl;
	if (post_exp->left->name == "primary_expression") {
		TreePtr primary_exp = post_exp->left;
		return parser_primary_expression(primary_exp);
	}
	else if (post_exp->left->right->name == "[") {
		//�������
		string arrayName = post_exp->left->left->left->content;
		TreePtr expression = post_exp->left->right->right;
		varNode enode = parser_expression(expression);
		arrayNode anode = getArrayNode(arrayName);

		if (anode.num < 0)
			error(post_exp->left->right->line, "Undifined array " + arrayName);

		varNode tempVar;
		string tempName = "temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;
		tempVar.name = tempName;
		tempVar.type = anode.type;
		tempVar.useAddress = true;
		recordStack.back().varInsert(tempVar);

		if (anode.type == "int" || anode.type == "double") {
			varNode tempVar2;
			string tempName2 = "temp" + inttostr(innerCode.tempNum);
			++innerCode.tempNum;
			tempVar2.name = tempName2;
			tempVar2.type = "int";
			recordStack.back().varInsert(tempVar2);
			if (anode.type == "int") {

				varNode tempVar3;
				string tempName3 = "temp" + inttostr(innerCode.tempNum);
				++innerCode.tempNum;
				tempVar3.name = tempName3;
				tempVar3.type = "int";
				recordStack.back().varInsert(tempVar3);
				innerCode.addCode(tempName3 + " := #4");

				innerCode.addCode(tempName2 + " := " + innerCode.getNodeName(enode) + " * " + tempName3);
			}
			else if (anode.type == "double") {
				varNode tempVar3;
				string tempName3 = "temp" + inttostr(innerCode.tempNum);
				++innerCode.tempNum;
				tempVar3.name = tempName3;
				tempVar3.type = "int";
				recordStack.back().varInsert(tempVar3);
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
		varNode NNode;
		
		if (funcPool.find(funcName) == funcPool.end()) {
			error(post_exp->left->left->left->line, "Undefined function " + funcName);
		}

		if (post_exp->left->right->right->name == "argument_expression_list") {
			TreePtr argument_exp_list = post_exp->left->right->right;
			parser_argument_expression_list(argument_exp_list, funcName);
			//cout << "funcCall" << endl;

		}

		funcNode func = funcPool[funcName];
		
		if (func.rtype == "void") {
			innerCode.addCode("CALL " + funcName);
		}
		else {
			string Tname = "temp" + inttostr(innerCode.tempNum);
			++innerCode.tempNum;

			NNode = TempCreater(Tname, funcPool[funcName].rtype);
			innerCode.addCode(Tname + " := CALL " + funcName);

		}

		return NNode;
		
	}
	else if (post_exp->left->right->name == "INC_OP") {
		varNode R = parser_postfix_expression(post_exp->left);

		if (R.type != "int")
			error(post_exp->left->right->line, "++ operation can only use for int type.");

		string Tname = "temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;
		varNode Node1 = TempCreater(Tname, "int");
		recordStack.back().varInsert(Node1);
		string tempnameone = "temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;
		varNode NNode = TempCreater(tempnameone, "int");
		recordStack.back().varInsert(NNode);
		innerCode.addCode(tempnameone + " := #1");

		//����������ǵ�ַ
		if (R.useAddress) {
			innerCode.addCode(Tname + " := *" + R.name);
			innerCode.addCode("*" + R.name + " := *" + R.name + " + " + tempnameone);
		}
		else {
			innerCode.addCode(Tname += " := " + innerCode.getNodeName(R));
			innerCode.addCode(innerCode.getNodeName(R) +  " := " + innerCode.getNodeName(R) + " + " + tempnameone);
		}

		return Node1;
	}
	else if (post_exp->left->right->name == "DEC_OP") {

		varNode R = parser_postfix_expression(post_exp->left);

		if (R.type != "int")
			error(post_exp->left->right->line, "-- operation can only use for int type.");

		string Tname = "temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;
		varNode Node1 = TempCreater(Tname, "int");
		recordStack.back().varInsert(Node1);
		string T = "temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;
		varNode NNode = TempCreater(T, "int");
		recordStack.back().varInsert(NNode);
		innerCode.addCode(T + " := #1");

		//����������ǵ�ַ
		if (R.useAddress) {
			innerCode.addCode(Tname + " := *" + R.name);
			innerCode.addCode("*" + R.name + " := *" + R.name + " - " + T);
		}
		else {
			innerCode.addCode(Tname += " := " + innerCode.getNodeName(R));
			innerCode.addCode(innerCode.getNodeName(R) + " := " + innerCode.getNodeName(R) + " - " + T);
		}

		return Node1;
	}
}

void Parser::parser_argument_expression_list(TreePtr node, string funcName) {
	TreePtr argu_exp_list = node->left;
	funcNode func = funcPool[funcName];
	int i = 0;
	while (argu_exp_list->name == "argument_expression_list") {
		varNode R = parser_assignment_expression(argu_exp_list->right->right);

		innerCode.addCode(innerCode.createCodeforArgument(R));

		argu_exp_list = argu_exp_list->left;
		i++;
		if (func.paralist[func.paralist.size() - i].type != R.type) {
			error(argu_exp_list->line, "Wrong type arguments to function " + funcName);
		}
	}
	varNode R = parser_assignment_expression(argu_exp_list);
	innerCode.addCode(innerCode.createCodeforArgument(R));
	i++;
	if (func.paralist[func.paralist.size() - i].type != R.type) {
		error(argu_exp_list->line, "Wrong type arguments to function " + funcName);
	}
	if (i != func.paralist.size()) {
		error(argu_exp_list->line, "The number of arguments doesn't equal to the function parameters number.");
	}
}

varNode Parser::parser_primary_expression(TreePtr primary_exp) {
	if (primary_exp->left->name == "IDENTIFIER") {
		string content = primary_exp->left->content;
		varNode R = lookupNode(content);
		if (R.num < 0) {
			error(primary_exp->left->line, "Undefined variable " + content);
		}
		return R;
	}
	else if (primary_exp->left->name == "TRUE" || primary_exp->left->name == "FALSE") {
		string content = primary_exp->left->content;
		string T = "temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;
		varNode NNode = TempCreater(T, "bool");
		recordStack.back().varInsert(NNode);
		if(primary_exp->left->name == "TRUE") 
			innerCode.addCode(T + " := #1");
		else {
			innerCode.addCode(T + " := #0");
		}
		return NNode;
	}
	else if (primary_exp->left->name == "CONSTANT_INT") {
		string content = primary_exp->left->content;
		string T = "temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;
		
		varNode NNode = TempCreater(T, "int");
		recordStack.back().varInsert(NNode);
		innerCode.addCode(T + " := #"  + content);
		return NNode;
	}
	else if (primary_exp->left->name == "CONSTANT_DOUBLE") {
		string content = primary_exp->left->content;
		string T = "temp" + inttostr(innerCode.tempNum);
		++innerCode.tempNum;

		varNode NNode = TempCreater(T, "double");

		recordStack.back().varInsert(NNode);
		innerCode.addCode(T + " := F" + content);
		return NNode;
	}
	else if (primary_exp->left->name == "(") {
		TreePtr expression = primary_exp->left->right;
		return parser_expression(expression);
	}
}


//ȫ�ֲ���
string Parser::lookupVar(string name) {
	cout << "ȫ�ֲ��ң�" << endl;
	int N = recordStack.size();
	for (int i = N - 1; i >= 0; i--) {
		if (recordStack[i].varFind(name)==true)
			return recordStack[i].varGet(name).type;
	}
	return "";
}
//��ǰ�����
bool Parser::lookupCurruntVar(string name) {
	return recordStack.back().varFind(name);
}

struct varNode Parser::lookupNode(string name) {
	int N = recordStack.size();
	for (int i = N - 1; i >= 0; i--) {
		if (recordStack[i].varFind(name) == true)
			return recordStack[i].varGet(name);
	}
	varNode NON;
	NON.num = -1;
	return NON;
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
	cout << "GetArrayType!" << endl;
	int N = recordStack.size();
	for (int i = N - 1; i >= 0; i--) {
		if (recordStack[i].arrayFind(name) == true)
			return recordStack[i].arrayGet(name).type;
	}
	return "";
}

struct arrayNode Parser::getArrayNode(string name) {
	cout << "GetArrayNode!" << endl;
	int N = recordStack.size();
	for (int i = N - 1; i >= 0; i--) {
		if (recordStack[i].arrayFind(name) == true)
			return recordStack[i].arrayGet(name);
	}
	arrayNode NON;
	NON.num = -1;
	return NON;
}

int Parser::getBreakRecordNumber() {
	int N = recordStack.size();
	for (int i = N - 1; i >= 0; i--) {
		if (recordStack[i].canBreak)
			return i;
	}
	return -1;
}

void Parser::error(int line, string error) {

	print_code();

	cout << "Error! line " << line << ": ";
	cout << error << endl;
	exit(1);
}

struct varNode Parser::TempCreater(string name, string type) {
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
	}
}

void Parser::print_code() {
	innerCode.printCode();
}

