using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Runtime.Builtin.Util;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;

public static class FileIOBuiltinHandler
{

    public static Value Execute(Token token, TokenName builtin, List<Value> args)
    {
        return builtin switch
        {
            TokenName.Builtin_FileIO_AppendText => AppendText(token, args),
            TokenName.Builtin_FileIO_ChangeDirectory => ChangeDirectory(token, args),
            TokenName.Builtin_FileIO_Combine => Combine(token, args),
            TokenName.Builtin_FileIO_CopyFile => CopyFile(token, args),
            TokenName.Builtin_FileIO_CopyR => CopyR(token, args),
            TokenName.Builtin_FileIO_CreateFile => CreateFile(token, args),
            TokenName.Builtin_FileIO_DeleteFile => RemovePath(token, args),
            TokenName.Builtin_FileIO_FileExists => FileExists(token, args),
            TokenName.Builtin_FileIO_FileName => GetFileName(token, args),
            TokenName.Builtin_FileIO_FileSize => GetFileSize(token, args),
            TokenName.Builtin_FileIO_GetCurrentDirectory => GetCurrentDirectory(token, args),
            TokenName.Builtin_FileIO_GetFileAbsolutePath => GetFileAbsolutePath(token, args),
            TokenName.Builtin_FileIO_GetFileExtension => GetFileExtension(token, args),
            TokenName.Builtin_FileIO_GetFileInfo => GetFileInfo(token, args),
            TokenName.Builtin_FileIO_GetFilePath => GetFilePath(token, args),
            TokenName.Builtin_FileIO_Glob => Glob(token, args),
            TokenName.Builtin_FileIO_IsDirectory => IsDirectory(token, args),
            TokenName.Builtin_FileIO_ListDirectory => ListDirectory(token, args),
            TokenName.Builtin_FileIO_MakeDirectory => MakeDirectory(token, args),
            TokenName.Builtin_FileIO_MakeDirectoryP => MakeDirectoryP(token, args),
            TokenName.Builtin_FileIO_MoveFile => MoveFile(token, args),
            TokenName.Builtin_FileIO_PathParts => PathParts(token, args),
            TokenName.Builtin_FileIO_ReadBytes => ReadBytes(token, args),
            TokenName.Builtin_FileIO_ReadFile => ReadFile(token, args),
            TokenName.Builtin_FileIO_ReadLines => ReadLines(token, args),
            TokenName.Builtin_FileIO_ReadSlice => ReadSlice(token, args),
            TokenName.Builtin_FileIO_RemoveDirectory => RemovePath(token, args),
            TokenName.Builtin_FileIO_RemoveDirectoryF => RemovePathF(token, args),
            TokenName.Builtin_FileIO_TempDir => GetTempDirectory(token, args),
            TokenName.Builtin_FileIO_WriteBytes => WriteBytes(token, args),
            TokenName.Builtin_FileIO_WriteLine => WriteLine(token, args),
            TokenName.Builtin_FileIO_WriteSlice => WriteSlice(token, args),
            TokenName.Builtin_FileIO_WriteText => WriteText(token, args),
            _ => throw new FunctionUndefinedError(token, token.Text),
        };
    }

    private static Value GetFileInfo(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FileIOBuiltin.GetFileInfo, 1, args.Count);
        ParameterTypeMismatchError.ExpectString(token, FileIOBuiltin.GetFileInfo, 0, args[0]);

        return FileUtil.GetFileInfo(token, args[0].GetString());
    }

    private static Value Combine(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FileIOBuiltin.Combine, 1, args.Count);
        ParameterTypeMismatchError.ExpectList(token, FileIOBuiltin.Combine, 0, args[0]);

        List<string> paths = [];

        foreach (var item in args[0].GetList())
        {
            if (!item.IsString())
            {
                throw new InvalidOperationError(token, "Expected a list of strings.");
            }

            paths.Add(item.GetString());
        }

        var path = FileUtil.CombinePath(token, paths);
        return Value.CreateString(path);
    }

    private static Value GetFileSize(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FileIOBuiltin.FileSize, 1, args.Count);
        ParameterTypeMismatchError.ExpectString(token, FileIOBuiltin.FileSize, 0, args[0]);

        var path = args[0].GetString();

        return Value.CreateInteger(FileUtil.GetFileSize(token, path));
    }

    private static Value ListDirectory(Token token, List<Value> args)
    {
        if (args.Count != 1 && args.Count != 2)
        {
            throw new ParameterCountMismatchError(token, FileIOBuiltin.ListDirectory);
        }

        ParameterTypeMismatchError.ExpectString(token, FileIOBuiltin.ListDirectory, 0, args[0]);

        var path = args[0].GetString();
        var recursive = false;

        if (args.Count == 2)
        {
            ParameterTypeMismatchError.ExpectBoolean(token, FileIOBuiltin.ListDirectory, 1, args[1]);
            recursive = args[1].GetBoolean();
        }

        var contents = FileUtil.ListDirectory(token, path, recursive);
        return Value.CreateList(contents);
    }

    private static Value Glob(Token token, List<Value> args)
    {
        if (args.Count != 2 && args.Count != 3)
        {
            throw new ParameterCountMismatchError(token, FileIOBuiltin.Glob);
        }

        ParameterTypeMismatchError.ExpectString(token, FileIOBuiltin.Glob, 0, args[0]);

        if (!args[1].IsList() || args[1].GetList().Count == 0)
        {
            throw new TypeError(token, "Expected a non-empty list.");
        }

        if (args.Count == 3)
        {
            ParameterTypeMismatchError.ExpectList(token, FileIOBuiltin.Glob, 2, args[2]);
        }

        List<string> includePatterns = [];
        List<string> excludePatterns = [];
        var path = args[0].GetString();

        foreach (var item in args[1].GetList())
        {
            if (!item.IsString())
            {
                throw new TypeError(token, "Expected a list of strings.");
            }

            includePatterns.Add(item.GetString());
        }

        if (args.Count == 3)
        {
            foreach (var item in args[2].GetList())
            {
                if (!item.IsString())
                {
                    throw new TypeError(token, "Expected a list of strings.");
                }

                excludePatterns.Add(item.GetString());
            }
        }

        var files = FileUtil.Glob(token, path, includePatterns, excludePatterns);
        return Value.CreateList(files);
    }

    private static Value AppendText(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FileIOBuiltin.AppendText, 2, args.Count);
        ParameterTypeMismatchError.ExpectString(token, FileIOBuiltin.AppendText, 0, args[0]);
        ParameterTypeMismatchError.ExpectString(token, FileIOBuiltin.AppendText, 1, args[1]);

        var path = args[0].GetString();
        var text = args[1].GetString();

        return Value.CreateBoolean(FileUtil.AppendText(token, path, text));
    }

    private static Value WriteBytes(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FileIOBuiltin.WriteBytes, 2, args.Count);

        ParameterTypeMismatchError.ExpectString(token, FileIOBuiltin.WriteBytes, 0, args[0]);
        var path = args[0].GetString();

        if (args[1].IsList())
        {
            ParameterTypeMismatchError.ExpectList(token, FileIOBuiltin.WriteBytes, 1, args[1]);
            var bytes = args[1].GetList();
            return Value.CreateInteger(FileUtil.WriteBytes(token, path, bytes));
        }
        else if (args[1].IsBytes())
        {
            return Value.CreateInteger(FileUtil.WriteBytes(token, path, args[1].GetBytes()));        
        }

        throw new InvalidOperationError(token, "Expected a list or bytes.");
    }

    private static Value WriteText(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FileIOBuiltin.WriteText, 2, args.Count);

        ParameterTypeMismatchError.ExpectString(token, FileIOBuiltin.WriteText, 0, args[0]);
        ParameterTypeMismatchError.ExpectString(token, FileIOBuiltin.WriteText, 1, args[1]);

        var path = args[0].GetString();
        var text = args[1].GetString();

        return Value.CreateBoolean(FileUtil.WriteText(token, path, text));
    }

    private static Value WriteLine(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FileIOBuiltin.WriteLine, 2, args.Count);

        ParameterTypeMismatchError.ExpectString(token, FileIOBuiltin.WriteLine, 0, args[0]);

        var path = args[0].GetString();

        if (args[1].IsString())
        {
            var text = args[1].GetString();
            return Value.CreateBoolean(FileUtil.WriteLine(token, path, text));
        }
        else if (args[1].IsList())
        {
            List<string> text = [];
            foreach (var item in args[1].GetList())
            {
                text.Add(Serializer.Serialize(item));
            }
            return Value.CreateBoolean(FileUtil.WriteLines(token, path, text));
        }

        throw new InvalidOperationError(token, "Expected a string or a list.");
    }

    private static Value WriteSlice(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FileIOBuiltin.WriteSlice, 3, args.Count);
        ParameterTypeMismatchError.ExpectString(token, FileIOBuiltin.WriteSlice, 0, args[0]);
        ParameterTypeMismatchError.ExpectInteger(token, FileIOBuiltin.WriteSlice, 1, args[1]);
        ParameterTypeMismatchError.ExpectBytes(token, FileIOBuiltin.WriteSlice, 2, args[2]);

        string filePath = args[0].GetString();
        long offset = args[1].GetInteger();
        byte[] data = args[2].GetBytes();

        return Value.CreateInteger(FileUtil.WriteSlice(token, filePath, offset, data));
    }

    private static Value ReadSlice(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FileIOBuiltin.ReadSlice, 3, args.Count);
        ParameterTypeMismatchError.ExpectString(token, FileIOBuiltin.ReadSlice, 0, args[0]);
        ParameterTypeMismatchError.ExpectInteger(token, FileIOBuiltin.ReadSlice, 1, args[1]);
        ParameterTypeMismatchError.ExpectInteger(token, FileIOBuiltin.ReadSlice, 2, args[2]);

        string filePath = args[0].GetString();
        long offset = args[1].GetInteger();
        long length = args[2].GetInteger();

        return Value.CreateBytes(FileUtil.ReadSlice(token, filePath, offset, length));
    }

    private static Value ReadBytes(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FileIOBuiltin.ReadBytes, 1, args.Count);

        ParameterTypeMismatchError.ExpectString(token, FileIOBuiltin.ReadBytes, 0, args[0]);

        var path = args[0].GetString();

        return Value.CreateBytes(FileUtil.ReadBytes(token, path));
    }

    private static Value ReadFile(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FileIOBuiltin.ReadFile, 1, args.Count);

        ParameterTypeMismatchError.ExpectString(token, FileIOBuiltin.ReadFile, 0, args[0]);

        var path = args[0].GetString();

        return Value.CreateString(FileUtil.ReadFile(token, path));
    }

    private static Value ReadLines(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FileIOBuiltin.ReadLines, 1, args.Count);

        ParameterTypeMismatchError.ExpectString(token, FileIOBuiltin.ReadLines, 0, args[0]);

        var path = args[0].GetString();
        return Value.CreateList(FileUtil.ReadLines(token, path));
    }

    private static Value MoveFile(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FileIOBuiltin.MoveFile, 2, args.Count);

        ParameterTypeMismatchError.ExpectString(token, FileIOBuiltin.MoveFile, 0, args[0]);
        ParameterTypeMismatchError.ExpectString(token, FileIOBuiltin.MoveFile, 1, args[1]);

        var src = args[0].GetString();
        var dst = args[1].GetString();

        return Value.CreateBoolean(FileUtil.MoveFile(token, src, dst));
    }

    private static Value CopyFile(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FileIOBuiltin.CopyFile, 2, args.Count);

        ParameterTypeMismatchError.ExpectString(token, FileIOBuiltin.CopyFile, 0, args[0]);
        ParameterTypeMismatchError.ExpectString(token, FileIOBuiltin.CopyFile, 1, args[1]);

        var src = args[0].GetString();
        var dst = args[1].GetString();

        return Value.CreateBoolean(FileUtil.CopyFile(token, src, dst));
    }

    private static Value CopyR(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FileIOBuiltin.CopyR, 2, args.Count);

        ParameterTypeMismatchError.ExpectString(token, FileIOBuiltin.CopyR, 0, args[0]);
        ParameterTypeMismatchError.ExpectString(token, FileIOBuiltin.CopyR, 1, args[1]);

        var src = args[0].GetString();
        var dst = args[1].GetString();

        return Value.CreateBoolean(FileUtil.CopyDirectoryRecursive(token, src, dst));
    }

    private static Value ChangeDirectory(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FileIOBuiltin.ChangeDirectory, 1, args.Count);

        ParameterTypeMismatchError.ExpectString(token, FileIOBuiltin.ChangeDirectory, 0, args[0]);
        
        var path = args[0].GetString();
        return Value.CreateBoolean(FileUtil.ChangeDirectory(token, path));
    }

    private static Value GetTempDirectory(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FileIOBuiltin.TempDir, 0, args.Count);

        return Value.CreateString(FileUtil.GetTempDirectory());
    }

    private static Value MakeDirectory(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FileIOBuiltin.MakeDirectory, 1, args.Count);

        ParameterTypeMismatchError.ExpectString(token, FileIOBuiltin.MakeDirectory, 0, args[0]);
        
        var fileName = args[0].GetString();
        return Value.CreateBoolean(FileUtil.MakeDirectory(token, fileName));
    }

    private static Value MakeDirectoryP(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FileIOBuiltin.MakeDirectoryP, 1, args.Count);

        ParameterTypeMismatchError.ExpectString(token, FileIOBuiltin.MakeDirectoryP, 0, args[0]);
        
        var fileName = args[0].GetString();
        return Value.CreateBoolean(FileUtil.MakeDirectory(token, fileName));
    }

    private static Value RemovePath(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FileIOBuiltin.RemoveDirectory, 1, args.Count);

        ParameterTypeMismatchError.ExpectString(token, FileIOBuiltin.RemoveDirectory, 0, args[0]);
        
        var fileName = args[0].GetString();
        return Value.CreateBoolean(FileUtil.RemovePath(token, fileName));
    }

    private static Value RemovePathF(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FileIOBuiltin.RemoveDirectoryF, 1, args.Count);

        ParameterTypeMismatchError.ExpectString(token, FileIOBuiltin.RemoveDirectoryF, 0, args[0]);

        var fileName = args[0].GetString();
        return Value.CreateInteger(FileUtil.RemovePathRecursive(token, fileName));
    }

    private static Value GetFileExtension(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FileIOBuiltin.GetFileExtension, 1, args.Count);

        ParameterTypeMismatchError.ExpectString(token, FileIOBuiltin.GetFileExtension, 0, args[0]);

        var fileName = args[0].GetString();
        return Value.CreateString(FileUtil.GetFileExtension(token, fileName));
    }

    private static Value GetCurrentDirectory(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FileIOBuiltin.CreateFile, 0, args.Count);

        return Value.CreateString(FileUtil.GetCurrentDirectory());
    }

    private static Value GetFileName(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FileIOBuiltin.FileName, 1, args.Count);

        ParameterTypeMismatchError.ExpectString(token, FileIOBuiltin.FileName, 0, args[0]);

        var fileName = args[0].GetString();
        return Value.CreateString(FileUtil.GetFileName(token, fileName));
    }

    private static Value GetFilePath(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FileIOBuiltin.GetFilePath, 1, args.Count);

        ParameterTypeMismatchError.ExpectString(token, FileIOBuiltin.GetFilePath, 0, args[0]);

        var fileName = args[0].GetString();
        return Value.CreateString(FileUtil.GetParentPath(token, fileName));
    }

    private static Value GetFileAbsolutePath(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FileIOBuiltin.GetFileAbsolutePath, 1, args.Count);

        ParameterTypeMismatchError.ExpectString(token, FileIOBuiltin.GetFileAbsolutePath, 0, args[0]);

        var fileName = args[0].GetString();
        return Value.CreateString(FileUtil.GetAbsolutePath(token, fileName));
    }

    private static Value PathParts(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FileIOBuiltin.PathParts, 1, args.Count);
        ParameterTypeMismatchError.ExpectString(token, FileIOBuiltin.PathParts, 0, args[0]);

        var path = args[0].GetString();
        var parts = FileUtil.GetPathParts(token, path).Select(p => Value.CreateString(p)).ToList();
        return Value.CreateList(parts);
    }

    private static Value CreateFile(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FileIOBuiltin.CreateFile, 1, args.Count);

        ParameterTypeMismatchError.ExpectString(token, FileIOBuiltin.CreateFile, 0, args[0]);

        var fileName = args[0].GetString();
        return Value.CreateBoolean(FileUtil.CreateFile(token, fileName));
    }

    private static Value FileExists(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FileIOBuiltin.FileExists, 1, args.Count);

        ParameterTypeMismatchError.ExpectString(token, FileIOBuiltin.FileExists, 0, args[0]);

        var fileName = args[0].GetString();
        return Value.CreateBoolean(FileUtil.FileExists(token, fileName));
    }

    private static Value IsDirectory(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FileIOBuiltin.IsDirectory, 1, args.Count);

        ParameterTypeMismatchError.ExpectString(token, FileIOBuiltin.IsDirectory, 0, args[0]);

        var fileName = args[0].GetString();
        return Value.CreateBoolean(FileUtil.DirectoryExists(token, fileName));
    }
}