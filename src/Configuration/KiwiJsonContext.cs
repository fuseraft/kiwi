using System.Text.Json;
using System.Text.Json.Serialization;

namespace kiwi.Settings;

[JsonSourceGenerationOptions(
    WriteIndented = true,
    PropertyNameCaseInsensitive = true
)]
[JsonSerializable(typeof(KiwiSettings))]
[JsonSerializable(typeof(FileExtensions))]
[JsonSerializable(typeof(DebugSettings))]
[JsonSerializable(typeof(StandardLibraryPath))]
[JsonSerializable(typeof(List<StandardLibraryPath>))]
[JsonSerializable(typeof(Dictionary<string, string>))]
[JsonSerializable(typeof(List<string>))]
[JsonSerializable(typeof(Config))]
partial class KiwiJsonContext : JsonSerializerContext { }