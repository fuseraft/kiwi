/**
 * 	noctis: a hybrid-typed, object-oriented, interpreted, programmable command line shell.
 *
 *		scstauf@gmail.com
 **/

#ifndef PROTOTYPES_H
#define PROTOTYPES_H

string getParsedOutput(string cmd);
void parse(string s);
void zeroSpace(string arg0, string s, vector<string> command);
void oneSpace(string arg0, string arg1, string s, vector<string> command);
void twoSpace(string arg0, string arg1, string arg2, string s, vector<string> command);
void threeSpace(string arg0, string arg1, string arg2, string arg3, string s, vector<string> command);

string getPrompt();

int random(int low, int high);
double random(double min, double max);
string random(string start, string sc);

vector<Method>      methods;
vector<Object>      objects;
vector<Variable>    variables;
vector<List>        lists;
vector<Constant>    constants;
vector<Method>      ifStatements;
vector<Method>      forLoops;
vector<Method>      whileLoops;
vector<string>      args;
vector<Module>      modules;
vector<Script>      scripts;

void InternalCallMethod(string arg0, string arg1, string before, string after);
void InternalCreateModule(string s);
void InternalCreateObject(string arg0);
void InternalEncryptDecrypt(string arg0, string arg1);
void InternalForget(string arg0, string arg1);
void InternalInspect(string arg0, string arg1, string before, string after);
void InternalGetEnv(string arg1, string after, int mode);
void InternalOutput(string arg0, string arg1);
void InternalRemember(string arg0, string arg1);
bool InternalReturn(string arg0, string arg1, string before, string after);

string cleanString(string st);

List getDirectoryList(string before, bool filesOnly);

void delay(int seconds);
void displayVersion();
void executeMethod(Method m);
void executeNest(Container n);
void setFalseIf();
void forLoop(Method m);
void help(string app);
void loadSavedVars(Crypt c, string &bs);
void runScript();
void loop(bool skip);
void saveVariable(string var);
void writeline(string st);
void setup();
void setTrueIf();
void uninstall();
void whileLoop(Method m);
void write(string st);
void __true();
void __false();

void executeTemplate(Method m, vector<string> vs);
void executeSimpleStatement(string arg0, string arg1, string arg2, string s, vector<string> command);

void copyObject(string arg0, string arg1, string arg2, string s, vector<string> command);
void initializeVariable(string arg0, string arg1, string arg2, string s, vector<string> command);
void initializeListValues(string arg0, string arg1, string arg2, string s, vector<string> command);
void initializeGlobalVariable(string arg0, string arg1, string arg2, string s, vector<string> command);
void initializeObjectVariable(string arg0, string arg1, string arg2, string s, vector<string> command);
void initializeConstant(string arg0, string arg1, string arg2, string s, vector<string> command);

void appendText(string arg1, string arg2, bool newLine);
void __fwrite(string file, string contents);

bool is(string s, string si);
bool isScript(string path);

bool notStandardZeroSpace(string s);
bool notStandardOneSpace(string arg1);
bool notStandardTwoSpace(string arg1);

double getBytes(string path);
double getStack(string arg2);

bool isStringStack(string arg2);
bool stackReady(string arg2);
string getStringStack(string arg2);

void error(int errorType, string errorInfo, bool quit);

#endif
