#ifndef _RECORD_H_
#define _RECORD_H_

#include<iostream>
#include<string>
#include<vector>
#include<map>
#include "tree.h"
using namespace std;
unsigned int MyHash(string str);


//�����ڵ�
struct varNode {
	string name;
	string type;
	int num = -1;
	bool useAddress = false;
	string boolString;
};

//�����ڵ�
struct funcNode {
	bool isdefinied = false;
	string name;				//������
	string rtype;				//������������
	vector<varNode> paralist;	//��¼�β��б�
};

//����ڵ�
struct arrayNode {
	string name;
	string type;
	int num = -1;
};

struct hashmap {
	varNode table[128];
	arrayNode table2[128];
};


//block������
class Record {
public:
	funcNode func;	//����Ǻ�������¼������
	bool isfunc = false;//��¼�Ƿ��Ǻ���
	hashmap H;
	string breakLabelname;
	bool canBreak = false;
	Record()
	{
		for (int i = 0; i < 128; i++)
		{
			varNode newvar;
			newvar.name = "NULL";
			newvar.type = "int";
			newvar.num = -1;
			bool useAddress = false;
			string boolString = "false";
			arrayNode newarray;
			newarray.name = "NULL";
			newarray.type = "int";
			newarray.num = -1;
			H.table[i] = newvar;
			H.table2[i] = newarray;
		}
	};
	void varInsert(varNode var)
	{
		int hash = (MyHash(var.name) % 128);
		while (H.table[hash].name.compare("NULL") != 0 && H.table[hash].name.compare(var.name) != 0)
		{
			hash = (hash + 1) % 128;
		}
		H.table[hash] = var;
		return;
	}
	void arrayInsert(arrayNode array)
	{
		cout << "Inserting Array Started" << endl;
		int hash = (MyHash(array.name) % 128);
		while (H.table2[hash].name.compare("NULL") != 0 && H.table2[hash].name.compare(array.name) != 0)
		{
			hash = (hash + 1) % 128;
		}
		cout << "Inserting " << array.name << endl;
		cout << "Hash Num: " << hash << endl;
		H.table2[hash] = array;
		cout << H.table2[hash].name << "Inserting Finished!" << hash << endl;
		return;
	}
	bool varFind(string varname)
	{
		int hash = (MyHash(varname) % 128);
		while (H.table[hash].name.compare("NULL") != 0 && H.table[hash].name.compare(varname) != 0)
		{
			hash = (hash + 1) % 128;
		}
		if (H.table[hash].name.compare("NULL") == 0)
		{
			return false;
		}
		else  return true;
	}
	bool arrayFind(string name)
	{
		cout << "Finding " << name << "!" << endl;
		int hash = (MyHash(name) % 128);
		cout << hash << endl;
		cout << H.table2[hash].name << endl;
		while (H.table2[hash].name.compare("NULL") != 0 && H.table2[hash].name.compare(name) != 0)
		{
			hash = (hash + 1) % 128;
		}
		if (H.table2[hash].name.compare("NULL") == 0)
		{
			cout << "Not Find!" << endl;
			return false;
		}
		else  return true;
	}
	varNode varGet(string varname)
	{
		int hash = (MyHash(varname) % 128);
		while (H.table[hash].name.compare("NULL") != 0 && H.table[hash].name.compare(varname) != 0)
		{
			hash = (hash + 1) % 128;
		}
		return H.table[hash];
	}
	arrayNode arrayGet(string name)
	{
		int hash = (MyHash(name) % 128);
		while (H.table2[hash].name.compare("NULL") != 0 && H.table2[hash].name.compare(name) != 0)
		{
			hash = (hash + 1) % 128;
		}
		return H.table2[hash];
	}
};

#endif // !_BLOCK_H_
