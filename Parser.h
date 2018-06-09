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

	TreePtr root;

	void Init();
	void parseTree(TreePtr node);

	TreePtr parserDeclaration(TreePtr node);		//����parserDeclaration�Ľڵ�
	void parserInitDeclaratorList(string, TreePtr);
	void parserInitDeclarator(string, TreePtr );			//����parserInitDeclarator�Ľڵ�

	TreePtr parserFunctionDefinition(TreePtr);
	void parserParameterList(TreePtr,string,bool);			//��ȡ�����β��б�
	void parserParameterDeclaration(TreePtr, string,bool);	//��ȡ���������β�

	TreePtr parserStatement(TreePtr);

	void parserExpressionStatement(TreePtr);
	void parserArgumentExpressionList(TreePtr,string);
	void parserJumpStatement(TreePtr);
	void parserCompoundStatement(TreePtr);
	void parserSelectionStatement(TreePtr);
	void parserIterationStatement(TreePtr);

	varNode parserExpression(TreePtr);
	varNode parserAssignmentExpression(TreePtr);			//��ֵ���ʽ
	varNode parserLogicalOrExpression(TreePtr);			//�߼�����ʽ
	varNode parserLogicalAndExpression(TreePtr);		//�߼�����ʽ
	varNode parserInclusiveOrExpression(TreePtr);
	varNode parserExclusiveOrExpression(TreePtr);
	varNode parserAndExpression(TreePtr);
	varNode parserEqualityExpression(TreePtr);
	varNode parserRelationalExpression(TreePtr);
	varNode parserShiftExpression(TreePtr);
	varNode parserAdditiveExpression(TreePtr);
	varNode parserMultiplicativeExpression(TreePtr);
	varNode parserUnaryExpression(TreePtr);
	varNode parserPostfixExpression(TreePtr);
	varNode parserPrimaryExpression(TreePtr);


	string findVar(string name);			//���ر������ͣ��Ҳ�������""
	bool findCurruntVar(string name);		//���ҵ�ǰ���var
	struct varNode findNode(string name);	//���ر����ڵ�
	string getReturnType();
	string getArrayType(string name);
	struct arrayNode getArrayNode(string name);

	int getBreakRecordNumber();

	struct varNode createVar(string name, string type);

	void printError(int line, string error);

	void print_map();
};




#endif // !_parser_H_