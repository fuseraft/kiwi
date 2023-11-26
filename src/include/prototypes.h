#ifndef PROTOTYPES_H
#define PROTOTYPES_H

void handleFailedIfStatement();
void handlePublicDecl();
void handlePrivateDecl();
void handleEnd();
void handleExit();
void handleCaught();
void handleInlineScriptDecl(std::string &arg1);
void handleDirPop(std::string &arg1);
void handleDirPush(std::string &arg1);
void handleFilePop(std::string &arg1);
void handleFilePush(std::string &arg1);
void handleUnlockAssignment(std::string &arg1);
void handleLockAssignment(std::string &arg1);
void handleTemplateDecl(std::string &arg1);
void handleStringInspect(std::string &before, std::string &after, std::string &arg1);
void handleNumberInspect(std::string &before, std::string &after, std::string &arg1);
void handleCollectInspect(std::string &arg1);
void handleFileInspect(std::string &before, std::string &after, std::string &arg1);
void handleDirectoryInspect(std::string &before, std::string &after, std::string &arg1);
void handleListInspect(std::string &arg1);
void handleVariableInspect(std::string &before, std::string &after, std::string &arg1);
void handleClassInspect(std::string &arg1);
void handleMethodInspect(std::string &before, std::string &after, std::string &arg1);
void handleInitialDir(std::string &arg1);
void handleInlineShellExec(std::string &arg1, std::vector<std::string> &command);
void handleInlineParse(std::string &arg1);
void handleListDecl(std::string &arg1);
void handleChangeDir(std::string &arg1);
void handleLoad(std::string &arg1);
void handleRemove(std::string &arg1);
void handleDelay(std::string &arg1);
void handleErr(std::string &arg1);
void handlePrompt(std::string &arg1);
void handleIfStatement(std::string &arg1);
void handleGoto(std::string &arg1);
void handleSwitch(std::string &arg1);
void handleClear(std::string &arg);
void handleError(int errorType, const string variableName, bool isMethod);
void handleIfStatementDecl_Generic(std::string first, std::string second, std::string oper);
void handleIfStatementDecl_Method(std::string arg1, std::string arg1Result, std::string arg3, std::string arg3Result);
void handleClassDecl(std::string arg1, std::string arg3, std::string arg2);

void checkGenericCondition(const string arg1, const string arg3, const string arg2);
void checkMethodCondition(const string arg1, const string arg3, const string arg2);
void checkParamsCondition(const string arg1, const string arg2, const string arg3);
void checkVariableCondition(const string arg1, const string arg2, const string arg3);
void checkListContainsCondition(const string listName, const string condition, const string testValue);
void checkListInCondition(const string listName, const string condition, const string testValue);
void checkCondition(const string arg1, const string arg2, const string arg3);
bool checkListContains(const string listName, const string testString);
bool checkListForElement(const string listName, const string testString, const string conditionType);
void checkNumericStringFileDirCondition(string arg1, string arg2, string arg3);

string getParsedOutput(string cmd);
string getTestString(bool variableExists, const string variableName);

void parse(string s);
void zeroSpace(string arg0, vector<string> command);
void oneSpace(string arg0, string arg1, vector<string> command);
void twoSpace(string arg0, string arg1, string arg2, vector<string> command);
void threeSpace(string arg0, string arg1, string arg2, string arg3, vector<string> command);

void handleLoopInit_For(std::string &arg1, std::string &arg2, std::string &arg3, std::string &arg0);

void handleLoopInit_ForIn(std::string &arg1, std::string &arg3, std::string &arg0, bool &retFlag);

void handleLoopInit_Environment_BuiltIns();

void handleLoopInit_Brackets(std::string &arg3, std::string &arg1, bool &retFlag);

void handleLoopInit_Params(std::string &arg3, std::string &arg1);

void handleLoopInit_Variable_FileRead(std::string &before);

void handleLoopInit_Variable_Files(std::string &before);

void handleLoopInit_Variable_Directories(std::string &before);

void handleLoopInit_Variable_Length(std::string &before);

void handleLoopInit_ClassMembers_Variables(std::string &before);

void handleLoopInit_ClassMembers_Methods(std::string &before);

void handleLoopInit_CommandLineArgs();

void handleLoopInit_While(std::string &arg1, std::string &arg3, std::string &arg2, std::string &arg0);

int startREPL();
string getPrompt();

void internal_encode_decode(string arg0, string arg1);
void internal_env_builtins(string arg1, string after, int mode);
void internal_puts(string arg0, string arg1);

string cleanString(string st);

List getDirectoryList(string before, bool filesOnly);

void show_version();
void help(string app);
void initialize_state(string uslang);
void write(string st);
void writeline(string st);

void setList(string listName, string methodName, vector<string> params);

void copyClass(string arg0, string arg1, string arg2, vector<string> command);
void initializeVariable(string arg0, string arg1, string arg2, vector<string> command);
void initializeListValues(string arg0, string arg1, string arg2, vector<string> command);
void initializeGlobalVariable(string arg0, string arg1, string arg2, vector<string> command);
void initializeClassVariable(string arg0, string arg1, string arg2, vector<string> command);
void initializeConstant(string arg0, string arg1, string arg2);

double get_filesize(string path);

void parseAssignment(string arg0, string arg1, string arg2);
void parseNumberStack(vector<string> &contents, vector<string> vars, string &temporaryBuild, char currentChar);
void parseStringStack(vector<string> &contents, vector<string> vars, string &temporaryBuild, char currentChar);
string getStackValue(string value);
double getStack(string arg2);
bool isStringStack(string arg2);
bool stackReady(string arg2);
string getStringStack(string arg2);

void error(int errorType, string errorInfo, bool quit);

#endif
