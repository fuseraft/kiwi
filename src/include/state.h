#ifndef NOCTIS_STATE_H
#define NOCTIS_STATE_H

struct {
    bool CaptureParse;
    string ParsedOutput;
    string CurrentLine;
    string CurrentMethodClass;
    string CurrentModule;
    string CurrentClass;
    string CurrentScript;
    string CurrentScriptName;
    string GoTo;
    string LastValue;
    string LogFile;
    string Noctis;
    string PreviousScript;
    string PromptStyle;
    string SwitchVarName;
    string DefaultLoopSymbol;
    string Null;
    double NullNum;
    int ArgumentCount;
    int BadMethodCount;
    int BadClassCount;
    int BadVarCount;
    int CurrentLineNumber;
    int IfStatementCount;
    int ForLoopCount;
    int ParamVarCount;
    int WhileLoopCount;

    bool IsCommented;
    bool IsMultilineComment;

    bool Breaking;
    bool DefiningIfStatement;
    bool DefiningForLoop;
    bool DefiningLocalForLoop;
    bool DefiningLocalSwitchBlock;
    bool DefiningLocalWhileLoop;
    bool DefiningMethod;
    bool DefiningModule;
    bool DefiningNest;
    bool DefiningClass;
    bool DefiningClassMethod;
    bool DefiningParameterizedMethod;
    bool DefiningPrivateCode;
    bool DefiningPublicCode;
    bool DefiningScript;
    bool DefiningSwitchBlock;
    bool DefiningWhileLoop;
    bool DontCollectMethodVars;
    bool ExecutedIfStatement;
    bool ExecutedMethod;
    bool ExecutedTemplate;
    bool FailedIfStatement;
    bool FailedNest;
    bool GoToLabel;
    bool InDefaultCase;
    bool Returning;
    bool UseCustomPrompt;

    string ErrorVarName;
    string LastError;
    bool ExecutedTryBlock;
    bool RaiseCatchBlock;
    bool Negligence;
    bool SkipCatchBlock;
    bool SuccessFlag;
} State;

#endif

