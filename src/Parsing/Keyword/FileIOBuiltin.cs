namespace kiwi.Parsing.Keyword;

public static class FileIOBuiltin
{
    // File operations
    public const string AppendText = "__fio_appendtext__";
    public const string CopyFile = "__fio_copyfile__";
    public const string CopyR = "__fio_copyr__";
    public const string Combine = "__fio_combine__";
    public const string CreateFile = "__fio_createfile__";
    public const string DeleteFile = "__fio_deletefile__";
    public const string MoveFile = "__fio_movefile__";
    public const string ReadFile = "__fio_readfile__";
    public const string ReadLines = "__fio_readlines__";
    public const string ReadBytes = "__fio_readbytes__";
    public const string ReadSlice = "__fio_readslice__";
    public const string WriteLine = "__fio_writeline__";
    public const string WriteText = "__fio_writetext__";
    public const string WriteBytes = "__fio_writebytes__";
    public const string WriteSlice = "__fio_writeslice__";
    public const string FileExists = "__fio_isfile__";
    public const string GetFileExtension = "__fio_fileext__";
    public const string FileName = "__fio_filename__";
    public const string FileSize = "__fio_filesize__";
    public const string GetFilePath = "__fio_filepath__";
    public const string GetFileAbsolutePath = "__fio_fileabspath__";
    public const string GetFileInfo = "__fio_fileinfo__";
    public const string Glob = "__fio_glob__";
    public const string PathParts = "__fio_pathparts__";

    // Directory operations
    public const string ListDirectory = "__fio_listdir__";
    public const string MakeDirectory = "__fio_mkdir__";
    public const string MakeDirectoryP = "__fio_mkdirp__";
    public const string RemoveDirectory = "__fio_rmdir__";
    public const string RemoveDirectoryF = "__fio_rmdirf__";
    public const string IsDirectory = "__fio_isdir__";
    public const string ChangeDirectory = "__fio_chdir__";
    public const string GetCurrentDirectory = "__fio_cwd__";
    public const string TempDir = "__fio_tmpdir__";
    public const string TempFile = "__fio_tmpfile__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { AppendText,          TokenName.Builtin_FileIO_AppendText },
            { ChangeDirectory,     TokenName.Builtin_FileIO_ChangeDirectory },
            { Combine,             TokenName.Builtin_FileIO_Combine },
            { CopyFile,            TokenName.Builtin_FileIO_CopyFile },
            { CopyR,               TokenName.Builtin_FileIO_CopyR },
            { CreateFile,          TokenName.Builtin_FileIO_CreateFile },
            { DeleteFile,          TokenName.Builtin_FileIO_DeleteFile },
            { FileExists,          TokenName.Builtin_FileIO_FileExists },
            { FileName,            TokenName.Builtin_FileIO_FileName },
            { FileSize,            TokenName.Builtin_FileIO_FileSize },
            { GetCurrentDirectory, TokenName.Builtin_FileIO_GetCurrentDirectory },
            { GetFileAbsolutePath, TokenName.Builtin_FileIO_GetFileAbsolutePath },
            { GetFileExtension,    TokenName.Builtin_FileIO_GetFileExtension },
            { GetFileInfo,         TokenName.Builtin_FileIO_GetFileInfo },
            { GetFilePath,         TokenName.Builtin_FileIO_GetFilePath },
            { Glob,                TokenName.Builtin_FileIO_Glob },
            { IsDirectory,         TokenName.Builtin_FileIO_IsDirectory },
            { ListDirectory,       TokenName.Builtin_FileIO_ListDirectory },
            { MakeDirectory,       TokenName.Builtin_FileIO_MakeDirectory },
            { MakeDirectoryP,      TokenName.Builtin_FileIO_MakeDirectoryP },
            { MoveFile,            TokenName.Builtin_FileIO_MoveFile },
            { PathParts,           TokenName.Builtin_FileIO_PathParts },
            { ReadBytes,           TokenName.Builtin_FileIO_ReadBytes },
            { ReadFile,            TokenName.Builtin_FileIO_ReadFile },
            { ReadLines,           TokenName.Builtin_FileIO_ReadLines },
            { ReadSlice,           TokenName.Builtin_FileIO_ReadSlice },
            { RemoveDirectory,     TokenName.Builtin_FileIO_RemoveDirectory },
            { RemoveDirectoryF,    TokenName.Builtin_FileIO_RemoveDirectoryF },
            { TempDir,             TokenName.Builtin_FileIO_TempDir },
            { TempFile,            TokenName.Builtin_FileIO_TempFile },
            { WriteBytes,          TokenName.Builtin_FileIO_WriteBytes },
            { WriteLine,           TokenName.Builtin_FileIO_WriteLine },
            { WriteSlice,          TokenName.Builtin_FileIO_WriteSlice },
            { WriteText,           TokenName.Builtin_FileIO_WriteText },
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
