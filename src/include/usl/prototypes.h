#ifndef PROTOTYPES_H
#define PROTOTYPES_H

void handleFailedIfStatement();
void handlePublicDecl();
void handlePrivateDecl();
void handleProtectedDecl();
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
void handleStringInspect(std::string &before, std::string &after,
                         std::string &arg1);
void handleNumberInspect(std::string &before, std::string &after,
                         std::string &arg1);
void handleCollectInspect(std::string &arg1);
void handleFileInspect(std::string &before, std::string &after,
                       std::string &arg1);
void handleDirectoryInspect(std::string &before, std::string &after,
                            std::string &arg1);
void handleListInspect(std::string &arg1);
void handleVariableInspect(std::string &before, std::string &after,
                           std::string &arg1);
void handleClassInspect(std::string &arg1);
void handleMethodInspect(std::string &before, std::string &after,
                         std::string &arg1);
void handleInitialDir(std::string &arg1);
void handleInlineShellExec(std::string &arg1);
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
void interp_clear_command(std::string &arg);
void handleError(ErrorCode errorType, const std::string variableName);
void handleIfStatementDecl_Generic(std::string first, std::string second,
                                   std::string oper);
void handleIfStatementDecl_Method(std::string arg1, std::string arg1Result,
                                  std::string arg3, std::string arg3Result);
void handleClassDecl(std::string arg1, std::string arg3, std::string arg2);

void checkGenericCondition(const std::string arg1, const std::string arg3,
                           const std::string arg2);
void checkMethodCondition(const std::string arg1, const std::string arg3,
                          const std::string arg2);
void checkParamsCondition(const std::string &arg1, const std::string &arg2,
                          const std::string &arg3);
void checkVariableCondition(const std::string arg1, const std::string arg2,
                            const std::string arg3);
void checkListContainsCondition(const std::string listName,
                                const std::string condition,
                                const std::string testValue);
void checkListInCondition(const std::string listName,
                          const std::string condition,
                          const std::string testValue);
void checkCondition(const std::string arg1, const std::string arg2,
                    const std::string arg3);
bool checkListContains(const std::string listName,
                       const std::string testString);
bool checkListForElement(const std::string listName,
                         const std::string testString,
                         const std::string conditionType);
void checkNumericStringFileDirCondition(std::string arg1, std::string arg2,
                                        std::string arg3);

std::string get_parsed_stdout(std::string cmd);
std::string getTestString(bool variableExists, const std::string variableName);

void parse(std::string s);
void interp_StringList(StringList &StringList);
std::string preparse_stripcomment(std::string &inputString);
void interp_method_def(std::string &s);
void interp_ifstatement_def(std::vector<std::string> &command, std::string &s,
                            int size);
void interp_nestedif_def(std::vector<std::string> &command, std::string &s);
void interp_whileloop_def(std::vector<std::string> &command, std::string &s);
void interp_forloop_def(std::vector<std::string> &command, std::string &s);
void interp_default(int size, std::vector<std::string> &command,
                    std::string &s);
void interp_4space(std::vector<std::string> &command, std::string &s);
void interp_3space(std::vector<std::string> &command);
void interp_2space(std::vector<std::string> &command, std::string &s);
void interp_1space(std::vector<std::string> &command, std::string &s);
void interp_0space(std::vector<std::string> &command, std::string &s);
void interp_list_rhs(std::string &after, std::string &before);
void interp_ifstatement();
void preparse_line_classdef(std::string &s, std::string &freshLine);
void interp_classmethod_decl(std::string &s);
void interp_class_decl(std::string &s);
void interp_scriptdefinition(std::string &s);
void interp_moduledefinition(std::string &s);
void interp_switchstatement(std::string &s, std::vector<std::string> &command);
void interp_args(int size, std::vector<std::string> &command);
void interp_forloop();
void tokenize(std::string &s, std::vector<std::string> &tokens,
              std::string &bigString, bool &endOfCommand, StringList &stringList);
void legacy_interpreter(std::string s);
void zeroSpace(std::string arg0);
void oneSpace(std::string arg0, std::string arg1);
void twoSpace(std::string arg0, std::string arg1, std::string arg2,
              std::vector<std::string> command);
void threeSpace(std::string arg0, std::string arg1, std::string arg2,
                std::string arg3);

template <typename condition>
void interp_whileloop(std::string v1, std::string v2, condition cond);
void interp_whileloops();

void interp_forloopmethod(Method &m, int iterVal);
void interp_forloopmethod(Method &m, std::string iterVal);
void preparse_methodline(std::string &tmp, Method &m, std::string &cleanString,
                         std::string iterVal);

void handleLoopInit_For(std::string &arg1, std::string &arg3,
                        std::string &arg0);

void handleLoopInit_ForIn(std::string &arg1, std::string &arg3,
                          std::string &arg0, bool &retFlag);

void handleLoopInit_Environment_BuiltIns();

void handleLoopInit_Brackets(std::string &arg3, std::string &arg1,
                             bool &retFlag);

void handleLoopInit_Params(std::string &arg3, std::string &arg1);

void handleLoopInit_Variable_FileRead(std::string &before);

void handleLoopInit_Variable_Files(std::string &before);

void handleLoopInit_Variable_Directories(std::string &before);

void handleLoopInit_Variable_Length(std::string &before);

void handleLoopInit_ClassMembers_Variables(std::string &before);

void handleLoopInit_ClassMembers_Methods(std::string &before);

void handleLoopInit_CommandLineArgs();

void handleLoopInit_While(std::string &arg1, std::string &arg3,
                          std::string &arg2, std::string &arg0);

int load_repl();
std::string get_prompt();

void interp_env_rhs(std::string arg1, std::string after, int mode);
void interp_internal_puts(std::string arg1, bool newline);

std::string pre_parse(std::string st);

List getDirectoryList(std::string before, bool filesOnly);

void show_version();
void help(std::string app);
void initialize_state(std::string uslang);
void write(std::string st);
void writeline(std::string st);

void setList(std::string listName, std::string methodName,
             std::vector<std::string> params);

void interp_class_clone(std::string arg0, std::string arg1, std::string arg2);
void initializeVariable(std::string arg0, std::string arg1, std::string arg2,
                        std::vector<std::string> command);
void init_listvalues(std::string arg0, std::string arg1, std::string arg2);
void init_globalvar(std::string arg0, std::string arg1, std::string arg2,
                    std::vector<std::string> command);
void interp_init_classvar(std::string arg0, std::string arg1, std::string arg2);
void interp_init_const(std::string arg0, std::string arg1, std::string arg2);
void interp_mathfunc(std::string arg0, std::string before, std::string after);
void interp_mathfunc_assignfromvar(std::string arg0, std::string before,
                                   std::string after);
void interp_targetandtext(const std::string &arg1, const std::string &arg2,
                          std::string &target, std::string &text);
void interp_assign(std::string arg0, std::string arg1, std::string arg2);
void parseNumberStack(std::vector<std::string> &contents,
                      std::vector<std::string> vars,
                      std::string &temporaryBuild, char currentChar);
void parseStringStack(std::vector<std::string> &contents,
                      std::vector<std::string> vars,
                      std::string &temporaryBuild, char currentChar);
std::string getStackValue(std::string value);
double getStack(std::string arg2);
bool isStringStack(std::string arg2);
bool is_stackable(std::string &temporaryBuild);
bool stackReady(std::string arg2);
std::string getStringStack(std::string arg2);

void error(ErrorCode errorType, std::string errorInfo);

#endif
