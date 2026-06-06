# Kiwi Standard Library Catalog

One row per package. Import with `import "name"` (or `import name` for most). All entries are drawn from `docs/lib/*.md`.

| Import | Description | Key Functions / Objects |
|--------|-------------|--------------------------|
| `bench` | Benchmarking framework for measuring and reporting code performance. | `bench::run`, `Benchmark`, `BenchmarkResult` |
| `cli` | Output helpers, tables, progress bars, prompts, and argument parsing for CLI apps. | `cli::table`, `cli::progress`, `cli::prompt`, `cli::parse_args` |
| `collections` | Specialized collection types, including `Heap` and `Set`. | `Heap`, `Set`, `collections::new_set`, `collections::new_heap` |
| `compress` | Functions for compression and decompression. | `compress::gzip`, `compress::gunzip`, `compress::deflate` |
| `console` | An interface that wraps core I/O operations. | `console::print`, `console::readline`, `console::clear` |
| `crypto` | Cryptographic functions like MD5 and SHA-2. | `crypto::md5`, `crypto::sha256`, `crypto::sha512`, `crypto::hmac` |
| `csv` | Functions for parsing CSV data. | `csv::parse`, `csv::stringify`, `CsvReader`, `CsvWriter` |
| `curl` | Thin wrapper around the system `curl` command for HTTP requests. | `curl::get`, `curl::post`, `curl::request` |
| `date` | Extends the built-in `date` type with arithmetic and component accessor methods. | `date::add_days`, `date::diff`, `date::format`, `date::parse` |
| `dotenv` | Load `.env` files into the process environment. | `dotenv::load`, `dotenv::load_file` |
| `encode` | Encoding and decoding utilities: Base64, hex, and URL. | `encode::base64_encode`, `encode::base64_decode`, `encode::hex_encode` |
| `env` | For interacting with environment variables. | `env::get`, `env::set`, `env::has_key`, `env::keys` |
| `error` | Helpers for constructing, throwing, and inspecting structured errors. | `error::new`, `error::raise`, `error::wrap`, `Error` |
| `faker` | Realistic fake data for testing and development: names, addresses, emails, lorem, and more. | `faker::name`, `faker::email`, `faker::address`, `faker::lorem` |
| `fio` | Functions for file system operations. | `fio::read`, `fio::write`, `fio::exists`, `fio::mkdirp`, `fio::glob`, `fio::listdir`, `fio::remove` |
| `fmt` | `sprintf`-style string formatting with `%` specifiers. | `fmt::sprintf`, `fmt::printf` |
| `hashable` | The `Hashable` mixin struct for automatic hashmap serialization of instance variables. | `Hashable` mixin |
| `http` | Functions for HTTP requests (GET, POST, etc), including `HttpResponse`. | `http::get`, `http::post`, `http::put`, `http::delete`, `HttpResponse` |
| `httpserver` | Production-grade HTTP server with routing, middleware, static files, CORS, cookies, and Tomcat-style configuration. | `HttpServer`, `Router`, `httpserver::new` |
| `ipc` | Interprocess communication via named pipes with length-prefixed message framing. | `ipc::connect`, `ipc::listen`, `IpcChannel` |
| `iter` | An explicit iterator pattern for safely traversing lists. | `ListIterator`, `iter::new` |
| `json` | Functions for parsing and serializing JSON data. | `json::parse`, `json::stringify`, `json::pipeline`, `JsonPipeline` |
| `list` | Higher-order utility functions for working with lists. | `list::all`, `list::any`, `list::find`, `list::reject`, `list::iterator` |
| `log` | Structured logging with severity levels and optional file output. | `log::info`, `log::warn`, `log::error`, `log::debug`, `Logger` |
| `math` | Common mathematical functions and utilities. | `math::sin`, `math::cos`, `math::sqrt`, `math::pow`, `math::floor`, `math::ceil`, `math::random`, `math::eps`, `math::round` |
| `path` | Contains the definition for the `Path` struct. | `Path`, `path::new`, `Path::is_file`, `Path::is_dir`, `Path::join` |
| `pipeline` | Named, step-based data pipelines with before/after hooks and error recovery. | `Pipeline`, `pipeline::new` |
| `protobuf` | Encode and decode data in Protocol Buffer binary wire format (proto3). | `protobuf::encode`, `protobuf::decode`, `ProtobufMessage` |
| `reflector` | Reflection tools, including `Callable`. | `Callable`, `reflector::typeof`, `reflector::methods` |
| `regex` | Regular expression matching, searching, splitting, and replacement. | `regex::match`, `regex::find_all`, `regex::replace`, `regex::split`, `Regex` |
| `signal` | OS signal handling for `SIGINT`, `SIGTERM`, and `SIGHUP`. | `signal::trap`, `signal::ignore` |
| `smtp` | Send email via SMTP with a fluent `Message` builder. | `smtp::send`, `Message`, `smtp::new_message` |
| `socket` / `tls` | Low-level asynchronous TCP and TLS networking. | `Socket`, `TlsSocket`, `socket::connect`, `socket::listen` |
| `sqlite` | SQLite database access via the `SqliteDb` struct. | `SqliteDb`, `sqlite::open`, `SqliteDb::query`, `SqliteDb::exec` |
| `stat` | Descriptive statistics: mean, median, variance, correlation, and more. | `stat::mean`, `stat::median`, `stat::stddev`, `stat::correlation` |
| `std` | General-purpose utility functions for introspection and common operations. | `std::inspect`, `std::deep_copy`, `std::is_nil` |
| `stdin` | Functions for reading from standard input. | `stdin::readline`, `stdin::read_all` |
| `stopwatch` | A simple elapsed-time stopwatch with lap support. | `Stopwatch`, `stopwatch::new`, `Stopwatch::start`, `Stopwatch::lap` |
| `string` | String manipulation and transformation utilities. | `string::capitalize`, `string::slug`, `string::squish`, `string::titleize`, `string::base64_encode`, `string::urlencode`, `string::dedent` |
| `sys` | For executing shell commands. | `sys::exec`, `sys::run`, `sys::which`, `sys::getenv` |
| `table` | In-memory tabular data pipeline: filter, join, group, aggregate, and display lists of hashmaps. | `Table`, `table::new`, `Table::filter`, `Table::group_by`, `Table::join` |
| `task` | Asynchronous task management, including `Channel`. | `Task`, `Channel`, `task::spawn`, `task::await` |
| `template` | Lightweight `{{key}}` string template engine. | `template::render`, `Template` |
| `tester` | A simple unit testing framework. | `tester::test`, `tester::assert`, `TestSuite`, `tester::run` |
| `text` | Fluent line-oriented text processing pipeline with filter, map, match, and split steps. | `TextPipeline`, `text::pipeline` |
| `time` | Time and date utilities. | `time::now`, `time::sleep`, `time::parse_duration` |
| `udp` | UDP socket communication for sending and receiving datagrams. | `UdpSocket`, `udp::bind`, `udp::send`, `udp::recv` |
| `xml` | XML parser and serializer using a tree of `XmlNode` structs. | `XmlNode`, `xml::parse`, `xml::stringify` |
| `yaml` | YAML parser and serializer. | `yaml::parse`, `yaml::stringify` |

**Notes**
- Most packages are imported with `import "name"` (string form) or `import name`.
- `json` is pre-loaded by the runtime; explicit import is optional.
- Key functions are shown in `package::function` form where applicable.
- Structs (e.g., `Path`, `HttpResponse`, `SqliteDb`) are listed when they are the primary API surface.

## Quick Patterns (Session 2 Priority Packages)

### fio — file read/write/walk
```kiwi
fio = import "fio"
text = fio::read("input.txt")
fio::write("output.txt", "hello\n")
fio::append("log.txt", "entry")
for p in fio::glob("*.kiwi") do println(p) end
```

### json — serialize/deserialize with edge cases
```kiwi
json = import "json"
obj = json::parse('{"x":1,"arr":[true,null]}')
json_str = json::stringify(obj, true)  # pretty
# edge: empty, nested, unicode handled by runtime
```

### regex — match, findall, replace
```kiwi
regex = import "regex"
m = regex::match("a(b)c", "abc")  # groups
all = regex::find_all("\\d+", "a1b22")
replaced = regex::replace("hello", "l", "L")
```

### http — GET/POST, headers, response object
```kiwi
http = import "http"
resp = http::get("https://example.com", {"User-Agent":"kiwi"})
println(resp.status, resp.body)
post_resp = http::post("https://api", {"q":"test"}, {"Content-Type":"application/json"})
```

### math — numeric utilities
```kiwi
math = import "math"
println(math::sqrt(16), math::pow(2, 8), math::random())
println(math::floor(3.9), math::round(3.5))
```

### date — formatting and arithmetic
```kiwi
date = import "date"
d = date::parse("2024-01-01")
d2 = date::add_days(d, 30)
println(date::format(d2, "%Y-%m-%d"))
println(date::diff(d, d2))
```

### tester — test authoring pattern
```kiwi
tester = import "tester"
suite = tester::new_suite("math")
tester::test(suite, "add", fn() tester::assert(1+1==2) end)
tester::run(suite)
```

### task — async task creation and awaiting
```kiwi
task = import "task"
t = task::spawn(fn() 42 end)
result = task::await(t)
ch = Channel::new()
# use ch::send / ch::recv in tasks
```
