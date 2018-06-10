#ifndef _PARSER_H_
#define _PARSER_H_
#include "record.h"
#include "tree.h"
#include "innerCode.h"
#include <vector>
#include <set>

#include"record.h"
using namespace std;

class Parser {
public:

	Parser(TreePtr);	//���캯��
	~Parser();	//��������

private:
	map<string, funcNode> funcPool;			//������
	vector<Record> recordStack;				//ά����ջ
	InnerCode innerCode;					//�м�������ɹ���
	//set<string> build_in_function;

	TreePtr root;

	void Init();
	void parseTree(TreePtr node);

	
	TreePtr parser_declaration(TreePtr node);		//����parser_declaration�Ľڵ�
	void parser_init_declarator_list(string, TreePtr);
	void parser_init_declarator(string, TreePtr );			//����parser_init_declarator�Ľڵ�

	TreePtr parser_function_definition(TreePtr);
	void parser_parameter_list(TreePtr,string,bool);			//��ȡ�����β��б�
	void parser_parameter_declaration(TreePtr, string,bool);	//��ȡ���������β�

	TreePtr parser_statement(TreePtr);

	void parser_expression_statement(TreePtr);
	varNode parser_expression(TreePtr);

	void parser_argument_expression_list(TreePtr,string);

	void parser_jump_statement(TreePtr);
	void parser_compound_statement(TreePtr);
	void parser_selection_statement(TreePtr);
	void parser_iteration_statement(TreePtr);

	varNode parser_assignment_expression(TreePtr);			//��ֵ���ʽ
	varNode parser_logical_or_expression(TreePtr);			//�߼�����ʽ
	varNode parser_logical_and_expression(TreePtr);		//�߼�����ʽ
	varNode parser_inclusive_or_expression(TreePtr);
	varNode parser_exclusive_or_expression(TreePtr);
	varNode parser_and_expression(TreePtr);
	varNode parser_equality_expression(TreePtr);
	varNode parser_relational_expression(TreePtr);
	varNode parser_shift_expression(TreePtr);
	varNode parser_additive_expression(TreePtr);
	varNode parser_multiplicative_expression(TreePtr);
	varNode parser_unary_expression(TreePtr);
	varNode parser_postfix_expression(TreePtr);
	varNode parser_primary_expression(TreePtr);


	string lookupVar(string name);			//���ر������ͣ��Ҳ�������""
	bool lookupCurruntVar(string name);		//���ҵ�ǰ���var
	struct varNode lookupNode(string name);	//���ر����ڵ�
	string getFuncRType();
	string getArrayType(string);
	struct arrayNode getArrayNode(string);

	int getBreakRecordNumber();

	struct varNode TempCreater(string name, string type);

	void error(int line, string error);

	void print_map();
	void print_code();
};




#endif // !_parser_H_