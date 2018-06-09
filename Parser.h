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

	Parser(Tree*);	//���캯��
	~Parser();	//��������

private:
	map<string, funcNode> funcPool;			//������
	vector<Record> recordStack;				//ά����ջ
	InnerCode innerCode;					//�м�������ɹ���
	//set<string> build_in_function;

	struct Tree* root;

	void Init();
	void parseTree(struct Tree* node);

	
	struct Tree* parser_declaration(struct Tree* node);		//����parser_declaration�Ľڵ�
	void parser_init_declarator_list(string, struct Tree*);
	void parser_init_declarator(string, struct Tree* );			//����parser_init_declarator�Ľڵ�

	struct Tree* parser_function_definition(struct Tree*);
	void parser_parameter_list(struct Tree*,string,bool);			//��ȡ�����β��б�
	void parser_parameter_declaration(struct Tree*, string,bool);	//��ȡ���������β�

	struct Tree* parser_statement(struct Tree*);

	void parser_expression_statement(struct Tree*);
	varNode parser_expression(struct Tree*);

	void parser_argument_expression_list(struct Tree*,string);

	void parser_jump_statement(struct Tree*);
	void parser_compound_statement(struct Tree*);
	void parser_selection_statement(struct Tree*);
	void parser_iteration_statement(struct Tree*);

	varNode parser_assignment_expression(struct Tree*);			//��ֵ���ʽ
	varNode parser_logical_or_expression(struct Tree*);			//�߼�����ʽ
	varNode parser_logical_and_expression(struct Tree*);		//�߼�����ʽ
	varNode parser_inclusive_or_expression(struct Tree*);
	varNode parser_exclusive_or_expression(struct Tree*);
	varNode parser_and_expression(struct Tree*);
	varNode parser_equality_expression(struct Tree*);
	varNode parser_relational_expression(struct Tree*);
	varNode parser_shift_expression(struct Tree*);
	varNode parser_additive_expression(struct Tree*);
	varNode parser_multiplicative_expression(struct Tree*);
	varNode parser_unary_expression(struct Tree*);
	varNode parser_postfix_expression(struct Tree*);
	varNode parser_primary_expression(struct Tree*);


	string lookupVar(string name);			//���ر������ͣ��Ҳ�������""
	bool lookupCurruntVar(string name);		//���ҵ�ǰ���var
	struct varNode lookupNode(string name);	//���ر����ڵ�
	string getFuncRType();
	string getArrayType(string);
	struct arrayNode getArrayNode(string);

	int getBreakRecordNumber();

	struct varNode createTempVar(string name, string type);

	void error(int line, string error);

	void print_map();
	void print_code();
};




#endif // !_parser_H_