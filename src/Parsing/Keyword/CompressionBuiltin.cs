namespace kiwi.Parsing.Keyword;

public static class CompressionBuiltin
{
    public const string CompressBrotli    = "__compress_brotli__";
    public const string CompressDeflate   = "__compress_deflate__";
    public const string CompressGzip      = "__compress_gzip__";
    public const string CompressZstd      = "__compress_zstd__";
    public const string DecompressBrotli  = "__decompress_brotli__";
    public const string DecompressDeflate = "__decompress_deflate__";
    public const string DecompressGzip    = "__decompress_gzip__";
    public const string DecompressZstd    = "__decompress_zstd__";
    public const string ZipCreate         = "__zip_create__";
    public const string ZipExtract        = "__zip_extract__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map 
        = new Dictionary<string, TokenName>
        {
            { CompressBrotli,    TokenName.Builtin_Compression_CompressBrotli    },
            { CompressDeflate,   TokenName.Builtin_Compression_CompressDeflate   },
            { CompressGzip,      TokenName.Builtin_Compression_CompressGzip      },
            { CompressZstd,      TokenName.Builtin_Compression_CompressZstd      },
            { DecompressBrotli,  TokenName.Builtin_Compression_DecompressBrotli  },
            { DecompressDeflate, TokenName.Builtin_Compression_DecompressDeflate },
            { DecompressGzip,    TokenName.Builtin_Compression_DecompressGzip    },
            { DecompressZstd,    TokenName.Builtin_Compression_DecompressZstd    },
            { ZipCreate,         TokenName.Builtin_Compression_ZipCreate         },
            { ZipExtract,        TokenName.Builtin_Compression_ZipExtract        },
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;
    
    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}