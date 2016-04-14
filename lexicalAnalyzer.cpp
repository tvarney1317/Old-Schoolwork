// Thomas Varney
// January 2016
// This program takes source code as an input and splits it up into its component tokens along with 
// creating a symbol table, then parses those tokens.

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <iterator>
using namespace std;

#define IDENTIFIER 100
#define RESERVED 101
#define INTEGER 102
#define OPERATOR 103
#define CHARACTER 104
#define STRING 105
#define PUNCTUATION 106
#define CONSTINT 107
#define CONSTCHAR 108
#define VARINT 109
#define VARCHAR 110
#define ARRAY 111
#define ARRAYASSIGNMENT 10 //a[4] = 100; -> opcode value index array
#define ARRAYINDEXREFERENCE 20 //x = a[4]; OR a[a[5]] - the inner reference -> opcode array index target

class Entry
{
public:
	string name;
	int type;
	int value;
	int size;
	int elType;
	Entry() {};
	Entry(string entryName, int entryType, int entryValue);
};
class SymbolTable
{
private:
	vector<Entry> identifiers;
	vector<Entry> strings;
	vector<Entry> symbolTable[32];
	int idCount;
	int stringCount;
	int hash(int type, int value);
	int findID(string name);
	int findString(string name);
public:
	SymbolTable() { idCount = 0; stringCount = 0; };
	void add(Entry *newEntry);
	bool findEntry(string name, int type, int value);
	Entry * getEntry(int type, int value);
	Entry * getUpdatedID(int type, int value);
	void printToFile();
	void generateDeclarations(ofstream * outfile);
};
class LexicalAnalyzer
{
private:
	SymbolTable symbolTable;
	vector<char> currentToken;
	unordered_map<string, Entry> searchMap; //contains all reserved words, punctuation, and operators
	void processCurrentToken(ofstream *outfile);
	void idOrReserved(ofstream *outfile);
	void integerToken(ofstream *outfile);
	void generateMap();
public:
	LexicalAnalyzer() {};
	SymbolTable * analyze(string fileName);
	void error(int errorCode);
};
class Parser {
private:
	SymbolTable * symbolTable;
	unordered_map<int, Entry> idVals; //maps old id values to their new initialized values - only for initialized variables
	unordered_map<int, int> idTypes; //maps old id values to their updated data types - only for uninitialized variables
	int currentTokenType;
	int currentTokenValue;
	int lineNumber; //for user side errors
	int tempCount; //number of temporary variables in this expression or line
	int highestTempCount;
	int labelCount;
	bool elseClauseCheck; 
	void nextToken(ifstream *infile);
	bool validType();
	void includeStmt(ifstream *infile);
	void declarations(ifstream *infile);
	int typeEval(ifstream *infile);
	bool processVariables(ifstream *infile, int varType);
	bool identifierAssignment(ifstream *infile, int varType);
	void arrayHandler(ifstream *infile, int varType, Entry *id);
	void assignmentHandler(ifstream *infile, int varType, Entry *id);
	void methodBody(ifstream * infile, ofstream *outfile);
	void statementType(ifstream * infile, ofstream * outfile);
	void variableAssignment(ifstream * infile, ofstream * outfile, string varName);
	void arrayAssignment(ifstream * infile, ofstream * outfile, string varName);
	void printStatement(ifstream * infile, ofstream * outfile);
	void inputStatement(ifstream * infile, ofstream * outfile);
	void ifStatement(ifstream * infile, ofstream * outfile); 
	void whileStatement(ifstream * infile, ofstream * outfile);
	void orExp(ifstream *infile, ofstream *outfile, string *result);
	void andExp(ifstream * infile, ofstream * outfile, string * result);
	void notExp(ifstream * infile, ofstream * outfile, string * result);
	void exp(ifstream * infile, ofstream * outfile, string * result);
	void term(ifstream * infile, ofstream * outfile, string * result);
	void factor(ifstream * infile, ofstream * outfile, string * result);
	void newTemp(string * result);
	void newLabel(string * result);
	Entry * findEntry(int type, int value);
public:
	Parser() { currentTokenType = -1; currentTokenValue = -1; lineNumber = 1; tempCount = 0; labelCount = 1; elseClauseCheck = false; highestTempCount = 0; };
	int parse(string parseFile, SymbolTable * st);
	void error(int errorCode); //errorCode is thrown from try-catch block
};
class CodeGen
{
private:
	SymbolTable * sym;
public:
	void generate(int numTemps, SymbolTable * sym);
};
Entry::Entry(string entryName, int entryType, int entryValue)
{
	name = entryName;
	type = entryType;
	value = entryValue;
	size = 0;
	elType = 0;
}
int SymbolTable::hash(int type, int value) //NEW HASH
{
	return (type * value) % 32;
}
int SymbolTable::findID(string name)
{
	if ((int)identifiers.size() > 0)
	{
		for (vector<Entry>::iterator it = identifiers.begin(); it != identifiers.end(); ++it)
		{
			if (it->name == name) return it->value;
		}
	}

	idCount++;
	return idCount;
}
int SymbolTable::findString(string name)
{
	if ((int)strings.size() > 0)
	{
		for (vector<Entry>::iterator it = strings.begin(); it != strings.end(); ++it)
		{
			if (it->name == name) return it->value;
		}
	}

	stringCount++;
	return stringCount;
}
void SymbolTable::add(Entry *newEntry)
{
	int hashVal;

	if (newEntry->type == IDENTIFIER)
	{
		newEntry->value = findID(newEntry->name); //MODIFIED ID HANDLING
		if (newEntry->value == idCount)
		{
			identifiers.push_back(*newEntry);
			hashVal = hash(newEntry->type, newEntry->value);
			symbolTable[hashVal].push_back(*newEntry);
		}
	}
	else if (newEntry->type == STRING)
	{
		newEntry->value = findString(newEntry->name);
		if (newEntry->value == stringCount)
		{
			strings.push_back(*newEntry);
			hashVal = hash(newEntry->type, newEntry->value);
			symbolTable[hashVal].push_back(*newEntry);
		}
	}
	else
	{
		hashVal = hash(newEntry->type, newEntry->value);
		symbolTable[hashVal].push_back(*newEntry);
	}
}
void SymbolTable::printToFile() 
{
	ofstream outfile;

	outfile.open("symbolTable.txt");
	for (int i = 0; i < 31; i++)
	{
		if ((int)symbolTable[i].size() > 0)
		{
			outfile << (i + 1) << " " << symbolTable[i].size() << endl;
			for (vector<Entry>::iterator it = symbolTable[i].begin(); it != symbolTable[i].end(); ++it)
				if (it->type == STRING)
					outfile << "\"" << it->name << "\"" << " " << it->type << " " << it->value << " " << it->size << " " << it->elType << endl;
				else
					outfile << (*it).name << " " << (*it).type << " " << (*it).value << " " << (*it).size << " " << (*it).elType << endl;
		}
	}
	outfile.close();
} 
void SymbolTable::generateDeclarations(ofstream * outfile)
{
	for (int i = 0; i < 31; i++)
	{
		if ((int)symbolTable[i].size() > 0)
		{
			for (vector<Entry>::iterator it = symbolTable[i].begin(); it != symbolTable[i].end(); ++it) {
				if (it->type == CONSTINT)
					*outfile << "const int " << it->name << " = " << it->value << ";" << endl;
				else if (it->type == CONSTCHAR)
					*outfile << "const char " << it->name << " = " << it->value << ";" << endl;
				else if (it->type == VARINT)
					*outfile << "int " << it->name << " = " << it->value << ";" << endl;
				else if (it->type == VARCHAR)
					*outfile << "char " << it->name << " = " << it->value << ";" << endl;
				else if (it->type == ARRAY) {
					if (it->elType == VARINT)
						*outfile << "int " << it->name << "[" << it->size << "];" << endl;
					else if (it->elType == VARCHAR)
						*outfile << "char " << it->name << "[" << it->size << "];" << endl;
				}
			}
		}
	}
}
bool SymbolTable::findEntry(string findName, int type, int value)
{
	int row = hash(type, value);
	for (vector<Entry>::iterator it = symbolTable[row].begin(); it != symbolTable[row].end(); ++it)
	{
		if ((*it).name == findName)
			return true;
	}
	return false;
}
Entry * SymbolTable::getEntry(int type, int value)
{
	int row = hash(type, value);
	for (vector<Entry>::iterator it = symbolTable[row].begin(); it != symbolTable[row].end(); ++it)
	{
		if ((*it).type == type && (*it).value == value)
			return &(*it);
	}
	return nullptr;
}
Entry * SymbolTable::getUpdatedID(int type, int value)
{
	int row = hash(IDENTIFIER, value);
	for (vector<Entry>::iterator it = symbolTable[row].begin(); it != symbolTable[row].end(); ++it)
	{
		if ((*it).type == type && (*it).value == value)
			return &(*it);
	}
	return nullptr;
}
SymbolTable * LexicalAnalyzer::analyze(string fileName) //main loop for most character processing
{
	ofstream outfile;
	ifstream infile(fileName);
	char currentCharacter;
	string toFind; //used to find entries from map
	unordered_map<string, Entry>::iterator it;
	Entry newEntry; //use to create entries not found in map

	outfile.open("token.txt");
	generateMap(); //populates searchMap

	while (infile.get(currentCharacter))
	{
		switch (currentCharacter)
		{
		case '\n': break;
		case ' ': case '\t': //whitespace
		{
			if ((int)currentToken.size() > 0)
				processCurrentToken(&outfile);
			break;
		}
		case '@': case '$': case '^': case '?': case '~': case '`': error(1); break; //illegal character
		case '+': case '-': case '*': case '/': case '>': case '<': case '=': case '!': case '&': case '|': //operator
		{
			if ((int)currentToken.size() > 0)
				processCurrentToken(&outfile);
			currentToken.push_back(currentCharacter);
			currentCharacter = infile.peek(); //peek next character to see if operator
			if (currentCharacter == '+' || currentCharacter == '-' || currentCharacter == '*' || currentCharacter == '/' ||
				currentCharacter == '>' || currentCharacter == '<' || currentCharacter == '=' || currentCharacter == '!' ||
				currentCharacter == '&' || currentCharacter == '|') //operator with 2 characters i.e. ++, --, etc.
			{
				infile.get(currentCharacter); //advance stream even though we have next character
				currentToken.push_back(currentCharacter);
				toFind = string(currentToken.data(), currentToken.size());
				it = searchMap.find(toFind);
				if (it != searchMap.end()) //found operator in data map
					outfile << it->second.type << " " << it->second.value << endl;
				else if (toFind == "//") //single line comment
				{
					infile.get(currentCharacter);
					while (currentCharacter != '\n') //while not newline character
						infile.get(currentCharacter);
				}
				else if (toFind == "/*") //block comment
				{
					while (infile.get(currentCharacter))
					{
						if (currentCharacter == '/' && currentToken.back() == '*')
							break;
						else currentToken.push_back(currentCharacter);
					}
					if (infile.eof())
						error(3); //unterminated comment
				}
				else
				{
					toFind = string(1, currentToken[0]);
					it = searchMap.find(toFind);
					outfile << it->second.type << " " << it->second.value << endl;

					toFind = string(1, currentToken[1]);
					it = searchMap.find(toFind);
					outfile << it->second.type << " " << it->second.value << endl;
				}
			}
			else
			{
				toFind = string(1, currentToken[0]);
				it = searchMap.find(toFind);
				outfile << it->second.type << " " << it->second.value << endl;
			}
			currentToken.clear();
			break;
		}
		case ';': case '(': case ')': case '[': case ']': case ',': case '.': case '{': case '}': case '#': //punctuation
		{
			if ((int)currentToken.size() > 0)
				processCurrentToken(&outfile);
			toFind = string(1, currentCharacter);
			it = searchMap.find(toFind);
			outfile << it->second.type << " " << it->second.value << endl;
			break;
		}
		case '\'': //character
		{
			if ((int)currentToken.size() > 0)
				processCurrentToken(&outfile);
			infile.get(currentCharacter);
			if (currentCharacter == '\'') //check for matching quotation i.e. char = ''
				outfile << CHARACTER << " " << 0 << endl;
			else
			{
				currentToken.push_back(currentCharacter); //store what is presumably the character between quotations
				infile.get(currentCharacter);
				if (currentCharacter == '\'') //if matching quotation i.e. properly formatted char
				{
					outfile << CHARACTER << " " << (int)currentToken[0] << endl;
					currentToken.clear();
				}
				else
					error(4); //else bomb out because unterminated character
			}
			break;
		}
		case '\"':
		{
			if ((int)currentToken.size() > 0)
				processCurrentToken(&outfile);
			infile.get(currentCharacter);
			if (currentCharacter == '\"') //if empty string
			{
				newEntry = Entry("", STRING, 0);
				if (!(symbolTable.findEntry(newEntry.name, newEntry.type, newEntry.value)))
					symbolTable.add(&newEntry);
				outfile << STRING << " " << 0 << endl;
			}
			else
			{
				currentToken.push_back(currentCharacter);
				while (infile.get(currentCharacter))
					if (currentCharacter == '\"')
						break;
					else currentToken.push_back(currentCharacter);
					if (infile.eof())
						error(2); //unterminated string
					else
					{
						newEntry = Entry(string(currentToken.data(), currentToken.size()), STRING, 0);
						currentToken.clear();
						if (!(symbolTable.findEntry(newEntry.name, newEntry.type, newEntry.value)))
							symbolTable.add(&newEntry);
						outfile << newEntry.type << " " << newEntry.value << endl;
					}
			}
			break;
		}
		default: currentToken.push_back(currentCharacter); break;
		}
	}
	outfile.close();
	return &symbolTable;
}
void LexicalAnalyzer::processCurrentToken(ofstream *outfile)
{
	if (isalpha(currentToken[0]) || currentToken[0] == '_')
		idOrReserved(outfile);
	else
		integerToken(outfile);
	currentToken.clear();
}
void LexicalAnalyzer::idOrReserved(ofstream *outfile)
{
	string newName;
	Entry identifier; //creates entry object for identifiers (required to hold onto pointer)
	unordered_map<string, Entry>::iterator it;
	it = searchMap.find(string(currentToken.data(), currentToken.size()));
	if (it != searchMap.end()) //if match is found in reserved word table
	{
		if (symbolTable.findEntry(it->second.name, it->second.type, it->second.value)) //if word is in symbol table
			*outfile << it->second.type << " " << it->second.value << endl; //just write to main output
		else
		{
			symbolTable.add(&(it->second)); //add to symbol table
			*outfile << it->second.type << " " << it->second.value << endl; //write to main output
		}
	}
	else
	{
		newName = string(currentToken.data(), currentToken.size());
		identifier = Entry("", IDENTIFIER, -1);
		identifier.name = newName;
		symbolTable.add(&identifier);
		*outfile << identifier.type << " " << identifier.value << endl;
	}
}
void LexicalAnalyzer::integerToken(ofstream *outfile)
{
	string outNumber(currentToken.begin(), currentToken.end());
	*outfile << INTEGER << " " << outNumber << endl;
}
void LexicalAnalyzer::generateMap()
{
	searchMap.insert(pair<string, Entry>("+", Entry("+", OPERATOR, 43)));
	searchMap.insert(pair<string, Entry>("++", Entry("++", OPERATOR, 86)));
	searchMap.insert(pair<string, Entry>("-", Entry("-", OPERATOR, 45)));
	searchMap.insert(pair<string, Entry>("--", Entry("--", OPERATOR, 90)));
	searchMap.insert(pair<string, Entry>("*", Entry("*", OPERATOR, 42)));
	searchMap.insert(pair<string, Entry>("/", Entry("/", OPERATOR, 47)));
	searchMap.insert(pair<string, Entry>("%", Entry("%", OPERATOR, 37)));
	searchMap.insert(pair<string, Entry>(">", Entry(">", OPERATOR, 62)));
	searchMap.insert(pair<string, Entry>("<", Entry("<", OPERATOR, 60)));
	searchMap.insert(pair<string, Entry>("=", Entry("=", OPERATOR, 61)));
	searchMap.insert(pair<string, Entry>("==", Entry("==", OPERATOR, 122)));
	searchMap.insert(pair<string, Entry>("!=", Entry("!=", OPERATOR, 94)));
	searchMap.insert(pair<string, Entry>("<=", Entry("<=", OPERATOR, 121)));
	searchMap.insert(pair<string, Entry>(">=", Entry(">=", OPERATOR, 123)));
	searchMap.insert(pair<string, Entry>("!", Entry("!", OPERATOR, 33)));
	searchMap.insert(pair<string, Entry>("&&", Entry("&&", OPERATOR, 76)));
	searchMap.insert(pair<string, Entry>("||", Entry("||", OPERATOR, 124)));
	searchMap.insert(pair<string, Entry>(">>", Entry(">>", OPERATOR, 135)));
	searchMap.insert(pair<string, Entry>("<<", Entry("<<", OPERATOR, 136)));
	searchMap.insert(pair<string, Entry>(";", Entry(";", PUNCTUATION, 59)));
	searchMap.insert(pair<string, Entry>("(", Entry("(", PUNCTUATION, 40)));
	searchMap.insert(pair<string, Entry>(")", Entry(")", PUNCTUATION, 41)));
	searchMap.insert(pair<string, Entry>("[", Entry("[", PUNCTUATION, 91)));
	searchMap.insert(pair<string, Entry>("]", Entry("]", PUNCTUATION, 93)));
	searchMap.insert(pair<string, Entry>(",", Entry(",", PUNCTUATION, 44)));
	searchMap.insert(pair<string, Entry>(".", Entry(".", PUNCTUATION, 46)));
	searchMap.insert(pair<string, Entry>("{", Entry("{", PUNCTUATION, 130)));
	searchMap.insert(pair<string, Entry>("}", Entry("}", PUNCTUATION, 131)));
	searchMap.insert(pair<string, Entry>("#", Entry("#", PUNCTUATION, 35)));
	searchMap.insert(pair<string, Entry>("and", Entry("and", RESERVED, 200)));
	searchMap.insert(pair<string, Entry>("and_eq", Entry("and_eq", RESERVED, 201)));
	searchMap.insert(pair<string, Entry>("asm", Entry("asm", RESERVED, 202)));
	searchMap.insert(pair<string, Entry>("auto", Entry("auto", RESERVED, 203)));
	searchMap.insert(pair<string, Entry>("bitand", Entry("bitand", RESERVED, 204)));
	searchMap.insert(pair<string, Entry>("bitor", Entry("bitor", RESERVED, 205)));
	searchMap.insert(pair<string, Entry>("bool", Entry("bool", RESERVED, 206)));
	searchMap.insert(pair<string, Entry>("break", Entry("break", RESERVED, 207)));
	searchMap.insert(pair<string, Entry>("case", Entry("case", RESERVED, 208)));
	searchMap.insert(pair<string, Entry>("char", Entry("char", RESERVED, 209)));
	searchMap.insert(pair<string, Entry>("const", Entry("const", RESERVED, 210)));
	searchMap.insert(pair<string, Entry>("continue", Entry("continue", RESERVED, 211)));
	searchMap.insert(pair<string, Entry>("catch", Entry("catch", RESERVED, 212)));
	searchMap.insert(pair<string, Entry>("class", Entry("class", RESERVED, 213)));
	searchMap.insert(pair<string, Entry>("const_cast", Entry("const_cast", RESERVED, 214)));
	searchMap.insert(pair<string, Entry>("compl", Entry("compl", RESERVED, 215)));
	searchMap.insert(pair<string, Entry>("default", Entry("default", RESERVED, 216)));
	searchMap.insert(pair<string, Entry>("do", Entry("do", RESERVED, 217)));
	searchMap.insert(pair<string, Entry>("double", Entry("double", RESERVED, 218)));
	searchMap.insert(pair<string, Entry>("delete", Entry("delete", RESERVED, 219)));
	searchMap.insert(pair<string, Entry>("dynamic_cast", Entry("dynamic_cast", RESERVED, 220)));
	searchMap.insert(pair<string, Entry>("else", Entry("else", RESERVED, 221)));
	searchMap.insert(pair<string, Entry>("enum", Entry("enum", RESERVED, 222)));
	searchMap.insert(pair<string, Entry>("extern", Entry("extern", RESERVED, 223)));
	searchMap.insert(pair<string, Entry>("explicit", Entry("explicit", RESERVED, 224)));
	searchMap.insert(pair<string, Entry>("float", Entry("float", RESERVED, 225)));
	searchMap.insert(pair<string, Entry>("for", Entry("for", RESERVED, 226)));
	searchMap.insert(pair<string, Entry>("false", Entry("false", RESERVED, 227)));
	searchMap.insert(pair<string, Entry>("friend", Entry("friend", RESERVED, 228)));
	searchMap.insert(pair<string, Entry>("goto", Entry("goto", RESERVED, 229)));
	searchMap.insert(pair<string, Entry>("if", Entry("if", RESERVED, 230)));
	searchMap.insert(pair<string, Entry>("inline", Entry("inline", RESERVED, 231)));
	searchMap.insert(pair<string, Entry>("int", Entry("int", RESERVED, 232)));
	searchMap.insert(pair<string, Entry>("long", Entry("long", RESERVED, 233)));
	searchMap.insert(pair<string, Entry>("mutable", Entry("mutable", RESERVED, 234)));
	searchMap.insert(pair<string, Entry>("namespace", Entry("namespace", RESERVED, 235)));
	searchMap.insert(pair<string, Entry>("new", Entry("new", RESERVED, 236)));
	searchMap.insert(pair<string, Entry>("not", Entry("not", RESERVED, 237)));
	searchMap.insert(pair<string, Entry>("not_eq", Entry("not_eq", RESERVED, 238)));
	searchMap.insert(pair<string, Entry>("operator", Entry("operator", RESERVED, 239)));
	searchMap.insert(pair<string, Entry>("or", Entry("or", RESERVED, 240)));
	searchMap.insert(pair<string, Entry>("or_eq", Entry("or_eq", RESERVED, 241)));
	searchMap.insert(pair<string, Entry>("private", Entry("private", RESERVED, 242)));
	searchMap.insert(pair<string, Entry>("public", Entry("public", RESERVED, 243)));
	searchMap.insert(pair<string, Entry>("protected", Entry("protected", RESERVED, 244)));
	searchMap.insert(pair<string, Entry>("register", Entry("register", RESERVED, 245)));
	searchMap.insert(pair<string, Entry>("return", Entry("return", RESERVED, 246)));
	searchMap.insert(pair<string, Entry>("reinterpret_cast", Entry("reinterpret_cast", RESERVED, 247)));
	searchMap.insert(pair<string, Entry>("short", Entry("short", RESERVED, 248)));
	searchMap.insert(pair<string, Entry>("signed", Entry("signed", RESERVED, 249)));
	searchMap.insert(pair<string, Entry>("sizeof", Entry("sizeof", RESERVED, 250)));
	searchMap.insert(pair<string, Entry>("static", Entry("static", RESERVED, 251)));
	searchMap.insert(pair<string, Entry>("struct", Entry("struct", RESERVED, 252)));
	searchMap.insert(pair<string, Entry>("switch", Entry("switch", RESERVED, 253)));
	searchMap.insert(pair<string, Entry>("static_cast", Entry("static_cast", RESERVED, 254)));
	searchMap.insert(pair<string, Entry>("typedef", Entry("typedef", RESERVED, 255)));
	searchMap.insert(pair<string, Entry>("template", Entry("template", RESERVED, 256)));
	searchMap.insert(pair<string, Entry>("this", Entry("this", RESERVED, 257)));
	searchMap.insert(pair<string, Entry>("throw", Entry("throw", RESERVED, 258)));
	searchMap.insert(pair<string, Entry>("true", Entry("true", RESERVED, 259)));
	searchMap.insert(pair<string, Entry>("try", Entry("try", RESERVED, 260)));
	searchMap.insert(pair<string, Entry>("typeid", Entry("typeid", RESERVED, 261)));
	searchMap.insert(pair<string, Entry>("typename", Entry("typename", RESERVED, 262)));
	searchMap.insert(pair<string, Entry>("union", Entry("union", RESERVED, 263)));
	searchMap.insert(pair<string, Entry>("unsigned", Entry("unsigned", RESERVED, 264)));
	searchMap.insert(pair<string, Entry>("using", Entry("using", RESERVED, 265)));
	searchMap.insert(pair<string, Entry>("void", Entry("void", RESERVED, 266)));
	searchMap.insert(pair<string, Entry>("volatile", Entry("volatile", RESERVED, 267)));
	searchMap.insert(pair<string, Entry>("virtual", Entry("virtual", RESERVED, 268)));
	searchMap.insert(pair<string, Entry>("while", Entry("while", RESERVED, 269)));
	searchMap.insert(pair<string, Entry>("wchar_t", Entry("wchar_t", RESERVED, 270)));
	searchMap.insert(pair<string, Entry>("xor", Entry("xor", RESERVED, 271)));
	searchMap.insert(pair<string, Entry>("xor_eq", Entry("xor_eq", RESERVED, 272)));
	searchMap.insert(pair<string, Entry>("cin", Entry("cin", RESERVED, 273)));
	searchMap.insert(pair<string, Entry>("cout", Entry("cout", RESERVED, 274)));
	searchMap.insert(pair<string, Entry>("endl", Entry("endl", RESERVED, 275)));
	searchMap.insert(pair<string, Entry>("include", Entry("include", RESERVED, 276)));
	searchMap.insert(pair<string, Entry>("INT_MIN", Entry("INT_MIN", RESERVED, 277)));
	searchMap.insert(pair<string, Entry>("INT_MAX", Entry("INT_MAX", RESERVED, 278)));
	searchMap.insert(pair<string, Entry>("iomanip", Entry("iomanip", RESERVED, 279)));
	searchMap.insert(pair<string, Entry>("iostream", Entry("iostream", RESERVED, 280)));
	searchMap.insert(pair<string, Entry>("main", Entry("main", RESERVED, 281)));
	searchMap.insert(pair<string, Entry>("MAX_RAND", Entry("MAX_RAND", RESERVED, 282)));
	searchMap.insert(pair<string, Entry>("npos", Entry("npos", RESERVED, 283)));
	searchMap.insert(pair<string, Entry>("NULL", Entry("NULL", RESERVED, 0)));
	searchMap.insert(pair<string, Entry>("std", Entry("std", RESERVED, 284)));
	searchMap.insert(pair<string, Entry>("override", Entry("override", RESERVED, 285)));
	searchMap.insert(pair<string, Entry>("final", Entry("final", RESERVED, 286)));
	searchMap.insert(pair<string, Entry>("requires", Entry("requires", RESERVED, 287)));
	searchMap.insert(pair<string, Entry>("isalnum", Entry("isalnum", RESERVED, 288)));
	searchMap.insert(pair<string, Entry>("isalpha", Entry("isalpha", RESERVED, 289)));
	searchMap.insert(pair<string, Entry>("malloc", Entry("malloc", RESERVED, 290)));
	searchMap.insert(pair<string, Entry>("strcat", Entry("strcat", RESERVED, 291)));
	searchMap.insert(pair<string, Entry>("strlen", Entry("strlen", RESERVED, 292)));
}
void LexicalAnalyzer::error(int errorCode)
{
	switch (errorCode)
	{
	case 1: cout << "Invalid character error"; break;
	case 2: cout << "Unterminated string error"; break;
	case 3: cout << "Unterminated comment error"; break;
	case 4: cout << "Unterminated character error"; break;
	}
	exit(0); //dirty exit
}
Entry * Parser::findEntry(int type, int value) //fixed findEntry from Lexer 
{
	Entry * ent;
	if (type == VARINT || type == VARCHAR || type == CONSTCHAR || type == CONSTINT || type == ARRAY) 
		ent = symbolTable->getUpdatedID(type, value);
	else
		ent = symbolTable->getEntry(type, value);

	if (ent == nullptr)
		throw 5;

	return ent;
}
void Parser::nextToken(ifstream *infile)
{
	if (!(*infile >> currentTokenType >> currentTokenValue))
		throw 11;
}
bool Parser::validType() //method returns true or throws illegal type error
{
	/* the first valid token in either a declaration or the beginning of the main method is
	going to be one of the valid type names: const, char, or int - no other analysis is done here
	besides that check */
	if (currentTokenType == RESERVED && (currentTokenValue == 209 || currentTokenValue == 210 || currentTokenValue == 232))
		return true;
	throw 7;
}
void Parser::error(int errorCode)
{
	switch (errorCode)
	{
	case 1: cout << "Invalid Library Name @ line " << lineNumber << endl; break;
	case 2: cout << "Invalid namespace @ line " << lineNumber << endl; break;
	case 3: cout << "Missing semicolons @ line " << lineNumber << endl; break;
	case 4: cout << "Reserved word being treated as a variable or constant @ line " << lineNumber << endl; break;
	case 5: cout << "Variable or constant declared twice/ Cannot find entry in symbol table @ line " << lineNumber << endl; break;
	case 6: cout << "Constant being declared without assignment @ line " << lineNumber << endl; break;
	case 7: cout << "Illegal Type Name for variable or constant @ line " << lineNumber << endl; break;
	case 8: cout << "Missing comma between variable or constant declarations @ line " << lineNumber << endl; break;
	case 9: cout << "Missing opening or closing bracket @ line " << lineNumber << endl; break;
	case 10: cout << "Missing or illegal array size indicator - must be a number of valid size @ line " << lineNumber << endl; break;
	case 11: symbolTable->printToFile(); break; //premature end of file
	case 12: cout << "Attempting to assign variable value of uninitialized variable @ line " << lineNumber << endl; break;
	case 13: cout << "Missing opening or closing parenthesis @ line " << lineNumber << endl; break;
	case 14: cout << "Midding opening or closing brace in method or decision structure @ line " << lineNumber << endl; break;
	case 15: cout << "Missing return - source of error @ line " << lineNumber << endl; break;
	case 16: cout << "Undeclared or uninitialized variable being used in expression @ line " << lineNumber << endl; break;
	case 17: cout << "Cannot assign value to a constant @ line " << lineNumber << endl; break;
	case 18: cout << "Missing assignment operator @ line " << lineNumber << endl; break;
	case 19: cout << "Cin can only assign values to declared variables @ line " << lineNumber << endl; break;
	case 20: cout << "Invalid argument for expression @ line " << lineNumber << endl; break;
	case 21: cout << "Boolean argument required for proper decision structure @ line " << lineNumber << endl; break;
	case 22: cout << "Missing relational operator in boolean expression @ line " << lineNumber << endl; break;
	default: cout << "Syntax error - token out of place @ line " << lineNumber << endl; break;
	}
	exit(0); //dirty exit
}
void Parser::includeStmt(ifstream *infile) //to make sure of no glaring errors in the include statements
{
	nextToken(infile);
	while (currentTokenType == PUNCTUATION && currentTokenValue == 35) //#
	{
		nextToken(infile);
		if (currentTokenType != RESERVED && currentTokenValue != 276) //include
			throw 0; //syntax error

		nextToken(infile);
		if (currentTokenType == OPERATOR && currentTokenValue == 60) //<
		{
			nextToken(infile);
			if ((currentTokenType != RESERVED && currentTokenValue != 279) || (currentTokenType != RESERVED && currentTokenValue != 280)) //iomanip or iostream
				throw 1; //invalid library

			nextToken(infile);
			if (currentTokenType != OPERATOR && currentTokenValue != 62) //>
				throw 0; //syntax error

			lineNumber++;
		}
		else throw 0; //syntax error in include

		nextToken(infile);
	}

	if (currentTokenType == RESERVED && currentTokenValue == 265) //using
	{
		nextToken(infile);
		if (currentTokenType == RESERVED && currentTokenValue == 235) //namespace
		{
			nextToken(infile);
			if (currentTokenType != RESERVED && currentTokenValue != 284) //std
				throw 2;

			nextToken(infile);
			if (currentTokenType != PUNCTUATION && currentTokenValue != 59) //;
				throw 3;

			lineNumber++;
		}
		else throw 0; //syntax error in using

		nextToken(infile);
	}
}
void Parser::declarations(ifstream *infile)
{
	bool methodCall;

	while (validType())
	{
		methodCall = processVariables(infile, typeEval(infile));

		if (methodCall)
			break;

		if (currentTokenType != PUNCTUATION && currentTokenValue != 59) //;
			throw 3;

		lineNumber++;
		nextToken(infile); //new token
	}
}
int Parser::typeEval(ifstream *infile)
{
	if (currentTokenValue == 209)
		return VARCHAR;
	else if (currentTokenValue == 232)
		return VARINT;
	else
	{
		nextToken(infile);
		if (validType()) {
			if (currentTokenValue == 209)
				return CONSTCHAR;
			else if (currentTokenValue == 232)
				return CONSTINT;
			else
				throw 7; //user declared a const const
		}
	}
}
bool Parser::identifierAssignment(ifstream *infile, int varType) // I -> id | id[size] | id = (id | number | character)
{
	Entry * id;
	nextToken(infile);

	if (currentTokenType == RESERVED)
	{
		if (currentTokenValue == 281 && varType == VARINT) //int main()
		{
			symbolTable->printToFile();
			nextToken(infile);
			if (currentTokenType == PUNCTUATION && currentTokenValue == 40) // (
			{
				nextToken(infile);
				if (currentTokenType != PUNCTUATION || currentTokenValue != 41) // )
					throw 13;
			}
			else throw 13;
			return true;
		}
		else throw 4;
	}
	else if (currentTokenType == IDENTIFIER)
	{
		id = findEntry(currentTokenType, currentTokenValue); //find id in symbol table
		nextToken(infile);

		if (currentTokenType == PUNCTUATION && currentTokenValue == 91) //[
			arrayHandler(infile, varType, id);
		else if (currentTokenType == OPERATOR && currentTokenValue == 61) //=
			assignmentHandler(infile, varType, id);
		else if (varType == CONSTCHAR || varType == CONSTINT)
			throw 6; //missing assignment for constant	
		else
		{
			id->type = varType;
			idTypes.insert(pair<int, int>(id->value, id->type));
		}
	}
	else throw 6; //trying to declare a string, character, or number as an identifer

	return false;
}
void Parser::arrayHandler(ifstream *infile, int varType, Entry *id)
{
	int arrSize;
	int arrElType = varType;
	nextToken(infile);

	if (currentTokenType == INTEGER)
	{
arrSize = currentTokenValue;

nextToken(infile);
if (currentTokenType == PUNCTUATION && currentTokenValue == 93)
{
	id->type = ARRAY;
	id->size = arrSize;
	id->elType = arrElType;
	idVals.insert(pair<int, Entry>(id->value, *id));
}
else throw 9;
	}
	else throw 10;

	nextToken(infile);
}
void Parser::assignmentHandler(ifstream *infile, int varType, Entry *id)
{
	int oldValue;
	unordered_map<int, Entry>::iterator it;
	nextToken(infile);

	if (currentTokenType == INTEGER || currentTokenType == CHARACTER)
	{
		oldValue = id->value;
		id->type = varType;
		id->value = currentTokenValue;
		idVals.insert(pair<int, Entry>(oldValue, *id));
	}
	else if (currentTokenType == IDENTIFIER)
	{
		oldValue = id->value;
		it = idVals.find(currentTokenValue); //checking for already entered id
		if (it != idVals.end())
		{
			id->type = varType;
			id->value = it->second.value;
			idVals.insert(pair<int, Entry>(oldValue, *id));
		}
		else throw 12;
	}
	else throw 7;

	nextToken(infile);
}
bool Parser::processVariables(ifstream *infile, int varType) //L -> I | I,L
{
	bool methodCall;
	methodCall = identifierAssignment(infile, varType);

	while (currentTokenType == PUNCTUATION && currentTokenValue == 44) //comma
	{
		methodCall = identifierAssignment(infile, varType);
		if (methodCall)
			break;
	}

	if (currentTokenType == IDENTIFIER) //check for missing comma
		throw 8;

	return methodCall;
}
void Parser::methodBody(ifstream * infile, ofstream * outfile)
{
	nextToken(infile);
	if (currentTokenType != PUNCTUATION || currentTokenValue != 130) // {
	{
		outfile->close();
		throw 14;
	}
	lineNumber++;

	nextToken(infile);
	while (currentTokenType != RESERVED || currentTokenValue != 246) //while token is not return
	{
		if (tempCount > highestTempCount)
			highestTempCount = tempCount;
		tempCount = 0;
		if (currentTokenType == RESERVED || currentTokenType == IDENTIFIER)
			statementType(infile, outfile);
		else if (currentTokenType == PUNCTUATION && currentTokenValue == 131) // }
		{
			outfile->close();
			throw 15;
		}
		else
		{
			outfile->close();
			throw 100; //syntax error 
		}

		if (!elseClauseCheck)
			nextToken(infile);
		else
			elseClauseCheck = false;
	}

	outfile->close();

	nextToken(infile);
	if (currentTokenType != INTEGER || currentTokenValue != 0) // 0
	{
		outfile->close();
		throw 15;
	}

	nextToken(infile);
	if (currentTokenType != PUNCTUATION || currentTokenValue != 59) // ;
	{
		outfile->close();
		throw 3;
	}
	lineNumber++;

	nextToken(infile);
	if (currentTokenType != PUNCTUATION || currentTokenValue != 131) // }
	{
		outfile->close();
		throw 14;
	}
}
void Parser::statementType(ifstream * infile, ofstream * outfile) //NEW METHOD
{
	/* This method takes the place of the majority of the main loop in methodBody, for the purpose
	of allowing decision structures to parse their internal statements and the like separate from 
	the rest of the main method body. */
	if (currentTokenType == IDENTIFIER) {
		unordered_map<int, Entry>::iterator it;
		it = idVals.find(currentTokenValue);

		if (it != idVals.end()) //if constant, array, or initialized variable
		{
			if (it->second.type == VARINT || it->second.type == VARCHAR)
				variableAssignment(infile, outfile, it->second.name);
			else if (it->second.type == ARRAY)
				arrayAssignment(infile, outfile, it->second.name);
			else //trying to assign value to a constant
			{
				outfile->close();
				throw 17;
			}
		}
		else
		{
			unordered_map<int, int>::iterator it2;
			it2 = idTypes.find(currentTokenValue);

			if (it2 != idTypes.end())
			{
				Entry *p = findEntry(it2->second, it2->first);
				variableAssignment(infile, outfile, p->name);
				idVals.insert(pair<int, Entry>(p->value, *p));
			}
			else
			{
				outfile->close();
				throw 16;
			}
		}
	}
	else if (currentTokenType == RESERVED && currentTokenValue == 274) //cout
		printStatement(infile, outfile);
	else if (currentTokenType == RESERVED && currentTokenValue == 273) //cin
		inputStatement(infile, outfile);
	else if (currentTokenType == RESERVED && currentTokenValue == 230) //if
		ifStatement(infile, outfile);
	else if (currentTokenType == RESERVED && currentTokenValue == 269) //while
		whileStatement(infile, outfile);
}
void Parser::variableAssignment(ifstream * infile, ofstream * outfile, string varName) //id = E;
{
	string arg = "";
	nextToken(infile);
	if (currentTokenType != OPERATOR || currentTokenValue != 61) // =
	{
		outfile->close();
		throw 18;
	}
	
	nextToken(infile); 
	exp(infile, outfile, &arg);

	if (currentTokenType != PUNCTUATION || currentTokenValue != 59) // ;
	{
		outfile->close();
		throw 3;
	}

	lineNumber++;
	*outfile << 61 << "\t" << arg << "\t\t" << varName << endl;
}
void Parser::arrayAssignment(ifstream * infile, ofstream * outfile, string varName) //id[E] = E;
{
	string arg = "";
	string index = "";
	string indexArg = "";
	nextToken(infile);
	if (currentTokenType != PUNCTUATION || currentTokenValue != 91) // [
	{
		outfile->close();
		throw 9;
	}

	nextToken(infile);
	exp(infile, outfile, &index);
	newTemp(&indexArg);
	*outfile << 61 << "\t" << index << "\t\t" << indexArg << endl;

	if (currentTokenType != PUNCTUATION || currentTokenValue != 93) // ]
	{
		outfile->close();
		throw 9;
	}

	nextToken(infile);
	if (currentTokenType != OPERATOR || currentTokenValue != 61) // =
	{
		outfile->close();
		throw 18;
	}

	nextToken(infile);
	exp(infile, outfile, &arg);

	if (currentTokenType != PUNCTUATION || currentTokenValue != 59) // ;
	{
		outfile->close();
		throw 3;
	}

	lineNumber++;
	*outfile << ARRAYASSIGNMENT << "\t" << arg << "\t" << index << "\t" << varName << endl;
}
void Parser::printStatement(ifstream * infile, ofstream * outfile) 
{
	nextToken(infile);

	while (currentTokenType == OPERATOR && currentTokenValue == 136) // <<
	{
		nextToken(infile);
		if (currentTokenType == STRING) //lexer modified to make string type-value pairs distinct
		{
			Entry * p = findEntry(currentTokenType, currentTokenValue);
			*outfile << "cout" << "\t" << "\"" << p->name << "\"" << endl;
			nextToken(infile);
		}
		else if (currentTokenType == RESERVED && currentTokenValue == 275) //endl
		{
			*outfile << "cout" << "\t" << "'" << "\\" << "n" << "'" << endl;
			nextToken(infile);
		}
		else //expression - will evaluate whether or not input is valid here
		{
			string arg = "";
			exp(infile, outfile, &arg);
			*outfile << "cout" << "\t" << arg << endl;
		}
	}
	if (currentTokenType != PUNCTUATION || currentTokenValue != 59) // ;
	{
		outfile->close();
		throw 3;
	}

	lineNumber++;
}
void Parser::inputStatement(ifstream * infile, ofstream * outfile) //cin >> id >> id >> ...... >> id;
{
	int i = 10;
	nextToken(infile);

	while (currentTokenType == OPERATOR && currentTokenValue == 135) // >>
	{
		nextToken(infile);
		if (currentTokenType == IDENTIFIER)
		{
			unordered_map<int, Entry>::iterator it;
			it = idVals.find(currentTokenValue);

			if (it != idVals.end())
				*outfile << "cin" << "\t" << it->second.name << endl;
			else
			{
				outfile->close();
				throw 19; //undeclared variable
			}
		}
		else
		{
			outfile->close();
			throw 19; //invalid target for cin
		}

		nextToken(infile);
	}

	if (currentTokenType != PUNCTUATION || currentTokenValue != 59) // ;
	{
		outfile->close();
		throw 3;
	}
	lineNumber++;
}
//Decision structures strictly enforce brackets regardless of amount of lines inside it
void Parser::ifStatement(ifstream *infile, ofstream *outfile) //NEW METHOD
{
	/* My thinking here is that if there is no else clause, two labels one line after the other affects nothing
	in practice, as */
	string result = "";
	string ifFalse = "";
	string end = "";

	nextToken(infile);
	if (currentTokenType != PUNCTUATION || currentTokenValue != 40) //(
	{
		outfile->close();
		throw 13;
	}
	
	nextToken(infile);
	orExp(infile, outfile, &result);

	if (currentTokenType != PUNCTUATION || currentTokenValue != 41) // )
	{
		outfile->close();
		throw 13;
	}
	
	nextToken(infile);
	if (currentTokenType != PUNCTUATION || currentTokenValue != 130) // {
	{
		outfile->close();
		throw 14;
	}

	lineNumber++;
	newLabel(&ifFalse);
	newLabel(&end);
	*outfile << 122 << "\t" << result << "\t" << 0 << "\t" << "goto" << "\t" << ifFalse << endl;
	
	nextToken(infile);
	while (currentTokenType != PUNCTUATION && currentTokenValue != 131) // }
	{
		statementType(infile, outfile);
		nextToken(infile);
	}

	lineNumber++;
	*outfile << "goto" << "\t" << end << endl;
	*outfile << ifFalse << endl;

	elseClauseCheck = true;
	nextToken(infile);

	if (currentTokenType == RESERVED && currentTokenValue == 221) // else
	{
		nextToken(infile);
		if (currentTokenType != PUNCTUATION || currentTokenValue != 130) // {
		{
			outfile->close();
			throw 14;
		}

		lineNumber++;
		nextToken(infile);
		while (currentTokenType != PUNCTUATION && currentTokenValue != 131) // }
		{
			statementType(infile, outfile);
			nextToken(infile);
		}
	}
	
	lineNumber++;
	nextToken(infile);
	*outfile << end << endl;
}
void Parser::whileStatement(ifstream *infile, ofstream *outfile) //NEW METHOD
{
	string begin = "";
	string ifFalse = "";
	string result = "";
	nextToken(infile);
	if (currentTokenType != PUNCTUATION || currentTokenValue != 40) //(
	{
		outfile->close();
		throw 13;
	}
	
	newLabel(&begin);
	newLabel(&ifFalse);
	
	*outfile << begin << endl;
	nextToken(infile);
	orExp(infile, outfile, &result);

	if (currentTokenType != PUNCTUATION || currentTokenValue != 41) //)
	{
		outfile->close();
		throw 13;
	}

	*outfile << 122 << "\t" << result << "\t" << 0 << "\t" << "goto" << "\t" << ifFalse << endl;

	nextToken(infile);
	if (currentTokenType != PUNCTUATION || currentTokenValue != 130) // {
	{
		outfile->close();
		throw 14;
	}

	lineNumber++;
	nextToken(infile);
	while (currentTokenType != PUNCTUATION && currentTokenValue != 131) // }
	{
		statementType(infile, outfile);
		nextToken(infile);
	}

	lineNumber++;
	nextToken(infile);
	*outfile << "goto" << "\t" << begin << endl;
	*outfile << ifFalse << endl;
}
void Parser::orExp(ifstream *infile, ofstream *outfile, string * result)
{
	string arg1 = "";
	string arg2 = "";

	andExp(infile, outfile, &arg1);

	while (currentTokenType == OPERATOR && currentTokenValue == 124)
	{
		nextToken(infile);
		andExp(infile, outfile, &arg2);
		newTemp(result);
		*outfile << 124 << "\t" << arg1 << "\t" << arg2 << "\t" << *result << endl;
		arg1 = *result;
 	}
	
	*result = arg1;
}
void Parser::andExp(ifstream * infile, ofstream * outfile, string * result)
{
	string arg1 = "";
	string arg2 = "";

	notExp(infile, outfile, &arg1);

	while (currentTokenType == OPERATOR && currentTokenValue == 76)
	{
		nextToken(infile);
		notExp(infile, outfile, &arg2);
		newTemp(result);
		*outfile << 76 << "\t" << arg1 << "\t" << arg2 << "\t" << *result << endl;
		arg1 = *result;
	}

	*result = arg1;
}
void Parser::notExp(ifstream * infile, ofstream * outfile, string * result) //notExp->!(B) | E relop E
{
	string arg1 = "";
	string arg2 = "";
	int op = -1;
	string ifTrue = "";
	string end = "";
	if (currentTokenType == OPERATOR && currentTokenValue == 33) // !
	{
		nextToken(infile);
		if (currentTokenType != PUNCTUATION || currentTokenValue != 40) // (
		{
			outfile->close();
			throw 13;
		}

		nextToken(infile);
		orExp(infile, outfile, result);
		*outfile << 33 << "\t" << "1" << "\t" << *result << "\t" << *result << endl; //33 is opcode for !

		if (currentTokenType != PUNCTUATION || currentTokenValue != 41) // )
		{
			outfile->close();
			throw 13;
		}
		nextToken(infile);
	}
	else //E relop E
	{
		newTemp(result);
		newLabel(&ifTrue);
		newLabel(&end);
		exp(infile, outfile, &arg1); //E
		
		if (currentTokenType == OPERATOR)
			op = currentTokenValue;
		else throw 22;

		nextToken(infile);
		exp(infile, outfile, &arg2); //E

		*outfile << op << "\t" << arg1 << "\t" << arg2 << "\t" << "goto" << "\t" << ifTrue << endl;
		*outfile << 61 << "\t" << "0" << "\t" << *result << endl; //61 is =
		*outfile << "goto" << "\t" << end << endl;
		*outfile << ifTrue << endl;
		*outfile << 61 << "\t" << "1" << "\t" << *result << endl;
		*outfile << end << endl;
	}
}
void Parser::exp(ifstream * infile, ofstream * outfile, string * result) 
{
	string arg1 = "";
	string arg2 = "";
	int op;

	term(infile, outfile, &arg1);

	while (currentTokenType == OPERATOR && (currentTokenValue == 43 || currentTokenValue == 45)) // + or -
	{
		op = currentTokenValue;
		nextToken(infile);
		term(infile, outfile, &arg2);
		newTemp(result);
		*outfile << op << "\t" << arg1 << "\t" << arg2 << "\t" << *result << endl;
		arg1 = *result;
	}

	*result = arg1;
}
void Parser::term(ifstream * infile, ofstream * outfile, string * result)
{
	string arg1 = "";
	string arg2 = "";
	int op;

	factor(infile, outfile, &arg1);

	while (currentTokenType == OPERATOR && (currentTokenValue == 42 || currentTokenValue == 47))
	{
		op = currentTokenValue;
		nextToken(infile);
		factor(infile, outfile, &arg2);
		newTemp(result);
		*outfile << op << "\t" << arg1 << "\t" << arg2 << "\t" << *result << endl;
		arg1 = *result;
	}

	*result = arg1;
}
void Parser::factor(ifstream * infile, ofstream * outfile, string * result)
{
	bool skipRead = false; 

	if (currentTokenType == PUNCTUATION && currentTokenValue == 40) // ( 
	{
		nextToken(infile);
		exp(infile, outfile, result); // E

		if (currentTokenType != PUNCTUATION || currentTokenValue != 41) // )
		{
			outfile->close();
			throw 13;
		}
	}
	else if (currentTokenType == INTEGER || currentTokenType == CHARACTER)
		*result = to_string(currentTokenValue);
	else if (currentTokenType == IDENTIFIER)
	{
		unordered_map<int, Entry>::iterator it;
		it = idVals.find(currentTokenValue);

		if (it != idVals.end())
		{
			if (it->second.type == ARRAY)
			{
				nextToken(infile);
				if (currentTokenType != PUNCTUATION || currentTokenValue != 91) // [
				{
					outfile->close();
					throw 9;
				}

				string index = "";
				nextToken(infile);
				exp(infile, outfile, &index);

				if (currentTokenType != PUNCTUATION || currentTokenValue != 93) // ]
				{
					outfile->close();
					throw 9;
				}

				newTemp(result);
				*outfile << ARRAYINDEXREFERENCE << "\t" << it->second.name << "\t" << index << "\t" << *result << endl;
			}
			else
				*result = it->second.name;
		}
		else
		{
			outfile->close();
			throw 16;
		}
	}
	else if (currentTokenType == OPERATOR && currentTokenValue == 45) //minus sign being used as a unary operator
	{ 
		string arg = ""; 
		newTemp(result);

		nextToken(infile);
		factor(infile, outfile, &arg);

		skipRead = true; 
		*outfile << 45 << "\t" << arg << "\t\t" << *result << endl;
	}
	if(!skipRead)
		nextToken(infile);
}
void Parser::newTemp(string * result)
{
	*result = "_t_" + to_string(tempCount);
	tempCount++;
}
void Parser::newLabel(string * result) 
{
	*result = "L_" + to_string(labelCount);
	labelCount++;
}
int Parser::parse(string parseFile, SymbolTable * st)
{
	symbolTable = st;
	ifstream infile;

	try
	{
		infile.open(parseFile);
		includeStmt(&infile);
		declarations(&infile);

		ofstream outfile;
		outfile.open("intermediateCode.txt");

		methodBody(&infile, &outfile);
		infile.close();
		return highestTempCount;
	}
	catch (int errorCode)
	{
		infile.close();
		error(errorCode);
	}
}
void CodeGen::generate(int numTemps, SymbolTable * st)
{
	sym = st;
	ofstream outfile;
	ifstream infile;
	string tempName;
	string a;
	string b;
	string target;

	infile.open("intermediateCode.txt");
	outfile.open("outputCode.txt");

	//variable declarations
	sym->generateDeclarations(&outfile);

	//temp declarations
	outfile << "int ";
	for (int i = 0; i < numTemps; i++)
	{
		tempName = "_t_" + i;
		if (i == (numTemps - 1))
			outfile << tempName << ";" << endl;
		else
			outfile << tempName << ", ";
		sym->add(new Entry(tempName, VARINT, 0));
	}

	outfile << "int main() {" << endl;
	//read intermediate code
	for (string in; infile >> in;) {
		if (in == "goto") //goto statement
		{
			infile >> in;
			outfile << "\tgoto " << in << ";" << endl;
		}
		else if (in == "cout") 
		{
			getline(infile, in);
			in.erase(in.begin());
			outfile << "\tcout << " << in << ";" << endl;
		}
		else if(in.at(0) == 'L') //label
			outfile << in << ":" << endl;
		else if (in == "61") //=
		{
			infile >> a >> target;
			outfile << "\t" << target << " = " << a << ";" << endl;
		}
		else if (in == "33") //!
		{
			infile >> a >> b >> target;
			outfile << "\t" << target << " = " << a << " ^ " << b << ";" << endl;
		}
		else if (in == "76" || in == "124") // || or &&
		{
			infile >> a >> b >> target;
			if (in == "76") outfile << "\t" << target << " = " << a << " & " << b << ";" << endl;
			else outfile << "\t" << target << " = " << a << " | " << b << ";" << endl;
		}
		else if (in == "10") //array assignment statement
		{
			infile >> a >> b >> target;
			outfile << "\t" << target << "[" << b << "] = " << a << ";" << endl;
		}
		else if (in == "20") //reference to an array
		{
			infile >> a >> b >> target;
			outfile << "\t" << target << " = " << a << "[" << b << "];" << endl;
		}
		else if (in == "62" || in == "60" || in == "122" || in == "94" || in == "121" || in == "123") //relational operators
		{
			infile >> a >> b;
			if (in == "62") outfile << "\tif(" << a << " > " << b << ") ";
			else if (in == "60") outfile << "\tif(" << a << " < " << b << ") ";
			else if (in == "122") outfile << "\tif(" << a << " == " << b << ") ";
			else if (in == "94") outfile << "\tif(" << a << " != " << b << ") ";
			else if (in == "121") outfile << "\tif(" << a << " <= " << b << ") ";
			else if (in == "123") outfile << "\tif(" << a << " >= " << b << ") ";
			infile >> a >> b; //goto labelName - the first is just to get the unncecessary part of the statement out of the way
			outfile << "goto " << b << ";" << endl;
		}
		else //some math operator
		{
			infile >> a >> b >> target;
			if (in == "43") outfile << "\t" << target << " = " << a << " + " << b << ";" << endl;
			else if (in == "45") outfile << "\t" << target << " = " << a << " - " << b << ";" << endl;
			else if (in == "42") outfile << "\t" << target << " = " << a << " * " << b << ";" << endl;
			else if (in == "47") outfile << "\t" << target << " = " << a << " / " << b << ";" << endl;
			else if (in == "37") outfile << "\t" << target << " = " << a << " % " << b << ";" << endl;
			else cout << "Something has gone horribly wrong" << endl;
		}
	}
	outfile << "}" << endl;

	infile.close();
	outfile.close();
}
int main()
{
	SymbolTable * sym;
	LexicalAnalyzer lex;
	Parser p;
	CodeGen cg;
	string s = "test.txt";
	string parseFile = "token.txt";
	int numTemps;

	sym = lex.analyze(s);
	numTemps = p.parse(parseFile, sym);
	cg.generate(numTemps, sym);
	return 0;
}