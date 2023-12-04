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
void parse_clear(std::string &arg);
void handleError(ErrorCode errorType, const std::string variableName, bool isMethod);
void handleIfStatementDecl_Generic(std::string first, std::string second, std::string oper);
void handleIfStatementDecl_Method(std::string arg1, std::string arg1Result, std::string arg3, std::string arg3Result);
void handleClassDecl(std::string arg1, std::string arg3, std::string arg2);

void checkGenericCondition(const std::string arg1, const std::string arg3, const std::string arg2);
void checkMethodCondition(const std::string arg1, const std::string arg3, const std::string arg2);
void checkParamsCondition(const std::string& arg1, const std::string& arg2, const std::string& arg3);
void checkVariableCondition(const std::string arg1, const std::string arg2, const std::string arg3);
void checkListContainsCondition(const std::string listName, const std::string condition, const std::string testValue);
void checkListInCondition(const std::string listName, const std::string condition, const std::string testValue);
void checkCondition(const std::string arg1, const std::string arg2, const std::string arg3);
bool checkListContains(const std::string listName, const std::string testString);
bool checkListForElement(const std::string listName, const std::string testString, const std::string conditionType);
void checkNumericStringFileDirCondition(std::string arg1, std::string arg2, std::string arg3);

std::string get_parsed_stdout(std::string cmd);
std::string getTestString(bool variableExists, const std::string variableName);

void parse(std::string s);
void parse_StringList(StringList &StringList);
void preparse_stripcomment(std::string &bigString, std::string &commentString);
void parse_method_def(std::string &s);
void parse_ifstatement_def(std::vector<std::string> &command, std::string &s, int size);
void parse_nestedif_def(std::vector<std::string> &command, std::string &s);
void parse_whileloop_def(std::vector<std::string> &command, std::string &s);
void parse_forloop_def(std::vector<std::string> &command, std::string &s);
void parse_default(int size, std::vector<std::string> &command, std::string &s);
void parse_4space(std::vector<std::string> &command, std::string &s);
void parse_3space(std::vector<std::string> &command);
void parse_2space(std::vector<std::string> &command, std::string &s);
void parse_1space(std::vector<std::string> &command, std::string &s);
void parse_0space(std::vector<std::string> &command, std::string &s);
void parse_ifstatement();
void preparse_line_classdef(std::string &s, std::string &freshLine);
void parse_classmethod_decl(std::string &s);
void parse_class_decl(std::string &s);
void parse_scriptdefinition(std::string &s);
void parse_moduledefinition(std::string &s);
void parse_switchstatement(std::string &s, std::vector<std::string> &command);
void parse_args(int size, std::vector<std::string> &command);
void parse_forloop();
void tokenize(int length, std::string &s, bool &parenthesis, bool &quoted, std::vector<std::string> &command, int &count, char &prevChar, std::string &bigString, bool &uncomment, bool &broken, StringList &StringList);
void zeroSpace(std::string arg0, std::vector<std::string> command);
void oneSpace(std::string arg0, std::string arg1, std::vector<std::string> command);
void twoSpace(std::string arg0, std::string arg1, std::string arg2, std::vector<std::string> command);
void threeSpace(std::string arg0, std::string arg1, std::string arg2, std::string arg3, std::vector<std::string> command);

template<typename condition>
void parse_whileloop(std::string v1, std::string v2, condition cond);
void parse_whileloops();

void parse_forloopmethod(Method &m, int iterVal);
void parse_forloopmethod(Method &m, std::string iterVal);
void preparse_methodline(std::string &tmp, Method &m, std::string &cleanString, std::string iterVal);    

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

int load_repl();
std::string get_prompt();

void internal_env_builtins(std::string arg1, std::string after, int mode);
void internal_puts(std::string arg0, std::string arg1, bool newline);

std::string pre_parse(std::string st);

List getDirectoryList(std::string before, bool filesOnly);

void show_version();
void help(std::string app);
void initialize_state(std::string uslang);
void write(std::string st);
void writeline(std::string st);

void setList(std::string listName, std::string methodName, std::vector<std::string> params);

void copy_class(std::string arg0, std::string arg1, std::string arg2, std::vector<std::string> command);
void initializeVariable(std::string arg0, std::string arg1, std::string arg2, std::vector<std::string> command);
void init_listvalues(std::string arg0, std::string arg1, std::string arg2, std::vector<std::string> command);
void init_globalvar(std::string arg0, std::string arg1, std::string arg2, std::vector<std::string> command);
void init_classvar(std::string arg0, std::string arg1, std::string arg2, std::vector<std::string> command);
void init_const(std::string arg0, std::string arg1, std::string arg2);
void parse_mathfunc(std::string arg0, std::string before, std::string after);
void parse_mathfunc_assignfromvar(std::string arg0, std::string before, std::string after);
void parse_targetandtext(const std::string &arg1, const std::string &arg2, std::string &target, std::string &text);
void parse_assign(std::string arg0, std::string arg1, std::string arg2);
void parseNumberStack(std::vector<std::string> &contents, std::vector<std::string> vars, std::string &temporaryBuild, char currentChar);
void parseStringStack(std::vector<std::string> &contents, std::vector<std::string> vars, std::string &temporaryBuild, char currentChar);
std::string getStackValue(std::string value);
double getStack(std::string arg2);
bool isStringStack(std::string arg2);
bool is_stackable(std::string &temporaryBuild);
bool stackReady(std::string arg2);
std::string getStringStack(std::string arg2);

void error(ErrorCode errorType, std::string errorInfo, bool quit);

#endif
