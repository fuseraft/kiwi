; Unshipped analyzer release
; https://github.com/dotnet/roslyn-analyzers/blob/main/src/Microsoft.CodeAnalysis.Analyzers/ReleaseTrackingAnalyzers.Help.md

### New Rules

Rule ID | Category | Severity | Notes
--------|----------|----------|-------
KIWI001 | Kiwi.Native | Error | Unsupported parameter type
KIWI002 | Kiwi.Native | Error | Unsupported return type
KIWI003 | Kiwi.Native | Error | Kiwi functions must be static
KIWI004 | Kiwi.Native | Error | Kiwi functions must be accessible
KIWI005 | Kiwi.Native | Error | Kiwi functions can't be generic
KIWI006 | Kiwi.Native | Error | Kiwi functions can't take ref, out or in parameters
KIWI007 | Kiwi.Native | Error | KiwiContext must be the first parameter
KIWI008 | Kiwi.Native | Error | Duplicate Kiwi function name
KIWI009 | Kiwi.Native | Error | Invalid Kiwi function name
KIWI010 | Kiwi.Native | Error | Kiwi extensions need unsafe code
KIWI011 | Kiwi.Native | Error | Unsupported default value
