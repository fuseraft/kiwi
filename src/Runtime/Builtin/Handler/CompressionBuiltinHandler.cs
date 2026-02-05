using System.IO.Compression;
using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Runtime.Builtin.Operation;
using kiwi.Tracing.Error;
using kiwi.Typing;
using ZstdSharp;

namespace kiwi.Runtime.Builtin.Handler;

public static class CompressionBuiltinHandler
{
    public static Value Execute(Token token, TokenName builtin, List<Value> args)
    {
        return builtin switch
        {
            TokenName.Builtin_Compression_CompressBrotli => CompressBrotli(token, args),
            TokenName.Builtin_Compression_CompressDeflate => CompressDeflate(token, args),
            TokenName.Builtin_Compression_CompressGzip => CompressGzip(token, args),
            TokenName.Builtin_Compression_CompressZstd => CompressZstd(token, args),
            TokenName.Builtin_Compression_DecompressBrotli => DecompressBrotli(token, args),
            TokenName.Builtin_Compression_DecompressDeflate => DecompressDeflate(token, args),
            TokenName.Builtin_Compression_DecompressGzip => DecompressGzip(token, args),
            TokenName.Builtin_Compression_DecompressZstd => DecompressZstd(token, args),
            TokenName.Builtin_Compression_ZipCreate => ZipCreate(token, args),
            TokenName.Builtin_Compression_ZipExtract => ZipExtract(token, args),
            _ => throw new InvalidOperationError(token, $"Unhandled compression builtin: {builtin}"),
        };
    }

    private static Value CompressDeflate(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CompressionBuiltin.CompressDeflate, 1, args.Count);
        var input = GetInputBytes(token, args[0]);
        using var output = new MemoryStream();
        using (var deflate = new DeflateStream(output, CompressionMode.Compress, true))
        {
            deflate.Write(input, 0, input.Length);
        }
        return Value.CreateBytes(output.ToArray());
    }

    private static Value DecompressDeflate(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CompressionBuiltin.DecompressDeflate, 1, args.Count);
        var input = GetInputBytes(token, args[0]);
        using var inputStream = new MemoryStream(input);
        using var output = new MemoryStream();
        using (var deflate = new DeflateStream(inputStream, CompressionMode.Decompress, true))
        {
            deflate.CopyTo(output);
        }
        return Value.CreateBytes(output.ToArray());
    }

    private static Value CompressGzip(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CompressionBuiltin.CompressGzip, 1, args.Count);
        var input = GetInputBytes(token, args[0]);
        using var output = new MemoryStream();
        using (var gzip = new GZipStream(output, CompressionMode.Compress, true))
        {
            gzip.Write(input, 0, input.Length);
        }
        return Value.CreateBytes(output.ToArray());
    }

    private static Value DecompressGzip(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CompressionBuiltin.DecompressGzip, 1, args.Count);
        var input = GetInputBytes(token, args[0]);
        using var inputStream = new MemoryStream(input);
        using var output = new MemoryStream();
        using (var gzip = new GZipStream(inputStream, CompressionMode.Decompress, true))
        {
            gzip.CopyTo(output);
        }
        return Value.CreateBytes(output.ToArray());
    }

    private static Value CompressBrotli(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CompressionBuiltin.CompressBrotli, 1, args.Count);
        var input = GetInputBytes(token, args[0]);
        using var output = new MemoryStream();
        using (var brotli = new BrotliStream(output, CompressionMode.Compress, true))
        {
            brotli.Write(input, 0, input.Length);
        }
        return Value.CreateBytes(output.ToArray());
    }

    private static Value DecompressBrotli(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CompressionBuiltin.DecompressBrotli, 1, args.Count);
        var input = GetInputBytes(token, args[0]);
        using var inputStream = new MemoryStream(input);
        using var output = new MemoryStream();
        using (var brotli = new BrotliStream(inputStream, CompressionMode.Decompress, true))
        {
            brotli.CopyTo(output);
        }
        return Value.CreateBytes(output.ToArray());
    }

    // Zstandard (single buffer, with optional level)
    private static Value CompressZstd(Token token, List<Value> args)
    {
        // compress_zstd(bytes_or_string, level = 3)
        int argCount = args.Count;
        ParameterCountMismatchError.CheckRange(token, CompressionBuiltin.CompressZstd, 1, 2, argCount);

        byte[] input = GetInputBytes(token, args[0]);
        int level = argCount == 2 ? (int)ConversionOp.GetInteger(token, args[1]) : 3;

        // Level clamping (zstd allows -131072 to 22; we keep it sane)
        level = Math.Clamp(level, 1, 22);

        using var compressor = new Compressor(level);
        byte[] compressed = compressor.Wrap(input).ToArray();

        return Value.CreateBytes(compressed);
    }

    private static Value DecompressZstd(Token token, List<Value> args)
    {
        // decompress_zstd(compressed_bytes)
        ParameterCountMismatchError.Check(token, CompressionBuiltin.DecompressZstd, 1, args.Count);

        byte[] input = GetInputBytes(token, args[0]);

        using var decompressor = new Decompressor();
        byte[] decompressed = decompressor.Unwrap(input).ToArray();

        return Value.CreateBytes(decompressed);
    }

    // ZIP archive (create from list of {name, content}, extract to list)
    private static Value ZipCreate(Token token, List<Value> args)
    {
        // __zip_create__(list_of_hashmaps [{name:"file.txt", content: bytes_or_string}, ...], compressionLevel = CompressionLevel.Optimal)
        ParameterCountMismatchError.CheckRange(token, CompressionBuiltin.ZipCreate, 1, 2, args.Count);
        ParameterTypeMismatchError.ExpectList(token, "", 0, args[0]);

        var entries = args[0].GetList();
        CompressionLevel level = args.Count == 2 
            ? (CompressionLevel)(int)ConversionOp.GetInteger(token, args[1]) 
            : CompressionLevel.Optimal;

        using var ms = new MemoryStream();
        using (var archive = new ZipArchive(ms, ZipArchiveMode.Create, true))
        {
            foreach (var entryVal in entries)
            {
                TypeError.ExpectHashmap(token, entryVal);
                var entryMap = entryVal.GetHashmap();

                if (!entryMap.TryGetValue(Value.CreateString("name"), out var nameVal) || !nameVal.IsString())
                {
                    throw new InvalidOperationError(token, "Each zip entry must have a 'name' string key");
                }

                string entryName = nameVal.GetString();

                if (!entryMap.TryGetValue(Value.CreateString("content"), out var contentVal))
                {
                    throw new InvalidOperationError(token, "Each zip entry must have a 'content' key");
                }

                byte[] contentBytes = GetInputBytes(token, contentVal);

                var entry = archive.CreateEntry(entryName);
                entry.ExternalAttributes = (int)FileAttributes.Normal;

                using var entryStream = entry.Open();
                entryStream.Write(contentBytes, 0, contentBytes.Length);
            }
        }

        return Value.CreateBytes(ms.ToArray());
    }

    private static Value ZipExtract(Token token, List<Value> args)
    {
        // __zip_extract__(zip_bytes) returns list of hashmaps [{name, content: bytes}]
        ParameterCountMismatchError.Check(token, CompressionBuiltin.ZipExtract, 1, args.Count);

        byte[] zipBytes = GetInputBytes(token, args[0]);

        var resultList = new List<Value>();

        using var ms = new MemoryStream(zipBytes);
        using (var archive = new ZipArchive(ms, ZipArchiveMode.Read, true))
        {
            foreach (var entry in archive.Entries)
            {
                using var entryStream = entry.Open();
                using var copyMs = new MemoryStream();
                entryStream.CopyTo(copyMs);

                var entryDict = new Dictionary<Value, Value>
                {
                    [Value.CreateString("name")]    = Value.CreateString(entry.FullName),
                    [Value.CreateString("content")] = Value.CreateBytes(copyMs.ToArray())
                };

                resultList.Add(Value.CreateHashmap(entryDict));
            }
        }

        return Value.CreateList(resultList);
    }

    private static byte[] GetInputBytes(Token token, Value arg)
    {
        if (arg.IsBytes())
        {
            return arg.GetBytes();
        }

        if (arg.IsString())
        {
            return System.Text.Encoding.UTF8.GetBytes(arg.GetString());
        }

        throw new ParameterTypeMismatchError(token, "compression function", 0, arg.Type, [Typing.ValueType.Bytes, Typing.ValueType.String]);
    }
}