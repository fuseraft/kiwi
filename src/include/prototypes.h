#ifndef PROTOTYPES_H
#define PROTOTYPES_H

string getParsedOutput(string cmd);
void parse(string s);
void zeroSpace(string arg0, string s, vector<string> command);
void oneSpace(string arg0, string arg1, string s, vector<string> command);
void twoSpace(string arg0, string arg1, string arg2, string s, vector<string> command);
void threeSpace(string arg0, string arg1, string arg2, string arg3, vector<string> command);

int startREPL();
string getPrompt();

void InternalEncryptDecrypt(string arg0, string arg1);
void InternalGetEnv(string arg1, string after, int mode);
void InternalOutput(string arg0, string arg1);

string cleanString(string st);

List getDirectoryList(string before, bool filesOnly);

void displayVersion();
void help(string app);
void setup();
void write(string st);
void writeline(string st);

void setList(string listName, string methodName, vector<string> params);

void copyClass(string arg0, string arg1, string arg2, string s, vector<string> command);
void initializeVariable(string arg0, string arg1, string arg2, string s, vector<string> command);
void initializeListValues(string arg0, string arg1, string arg2, string s, vector<string> command);
void initializeGlobalVariable(string arg0, string arg1, string arg2, string s, vector<string> command);
void initializeClassVariable(string arg0, string arg1, string arg2, string s, vector<string> command);
void initializeConstant(string arg0, string arg1, string arg2, string s);

double getBytes(string path);
double getStack(string arg2);

bool isStringStack(string arg2);
bool stackReady(string arg2);
string getStringStack(string arg2);

void error(int errorType, string errorInfo, bool quit);

#endif
