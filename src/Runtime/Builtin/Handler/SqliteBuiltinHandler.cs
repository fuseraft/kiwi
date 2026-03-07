using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Tracing.Error;
using kiwi.Typing;
using Microsoft.Data.Sqlite;

namespace kiwi.Runtime.Builtin.Handler;

public static class SqliteBuiltinHandler
{
    private static int _nextId = 1;
    private static readonly Dictionary<int, SqliteConnection> _connections = new();
    private static readonly object _lock = new();

    public static Value Execute(Token token, TokenName builtin, List<Value> args)
    {
        return builtin switch
        {
            TokenName.Builtin_Sqlite_Open         => Open(token, args),
            TokenName.Builtin_Sqlite_Close        => Close(token, args),
            TokenName.Builtin_Sqlite_Exec         => Exec(token, args),
            TokenName.Builtin_Sqlite_ExecParams   => ExecParams(token, args),
            TokenName.Builtin_Sqlite_Query        => Query(token, args),
            TokenName.Builtin_Sqlite_QueryParams  => QueryParams(token, args),
            TokenName.Builtin_Sqlite_LastInsertId => LastInsertId(token, args),
            TokenName.Builtin_Sqlite_Changes      => Changes(token, args),
            _ => throw new FunctionUndefinedError(token, token.Text),
        };
    }

    private static SqliteConnection GetConnection(Token token, string funcName, Value arg)
    {
        ParameterTypeMismatchError.ExpectInteger(token, funcName, 0, arg);
        var id = (int)arg.GetInteger();
        lock (_lock)
        {
            if (!_connections.TryGetValue(id, out var conn))
            {
                throw new KiwiError(token, "SqliteError", $"No open connection with id {id}");
            }
            return conn;
        }
    }

    private static Value Open(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, SqliteBuiltin.Open, 1, args.Count);
        ParameterTypeMismatchError.ExpectString(token, SqliteBuiltin.Open, 0, args[0]);

        var path = args[0].GetString();

        try
        {
            var conn = new SqliteConnection($"Data Source={path}");
            conn.Open();

            int id;
            lock (_lock)
            {
                id = _nextId++;
                _connections[id] = conn;
            }

            return Value.CreateInteger(id);
        }
        catch (Exception ex)
        {
            throw new KiwiError(token, "SqliteError", ex.Message);
        }
    }

    private static Value Close(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, SqliteBuiltin.Close, 1, args.Count);
        var conn = GetConnection(token, SqliteBuiltin.Close, args[0]);
        var id = (int)args[0].GetInteger();

        conn.Close();
        conn.Dispose();
        lock (_lock) { _connections.Remove(id); }

        return Value.CreateNull();
    }

    private static Value Exec(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, SqliteBuiltin.Exec, 2, args.Count);
        var conn = GetConnection(token, SqliteBuiltin.Exec, args[0]);
        ParameterTypeMismatchError.ExpectString(token, SqliteBuiltin.Exec, 1, args[1]);

        try
        {
            using var cmd = conn.CreateCommand();
            cmd.CommandText = args[1].GetString();
            return Value.CreateInteger(cmd.ExecuteNonQuery());
        }
        catch (Exception ex)
        {
            throw new KiwiError(token, "SqliteError", ex.Message);
        }
    }

    private static Value ExecParams(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, SqliteBuiltin.ExecParams, 3, args.Count);
        var conn = GetConnection(token, SqliteBuiltin.ExecParams, args[0]);
        ParameterTypeMismatchError.ExpectString(token, SqliteBuiltin.ExecParams, 1, args[1]);

        try
        {
            using var cmd = conn.CreateCommand();
            cmd.CommandText = args[1].GetString();
            BindParams(cmd, args[2]);
            return Value.CreateInteger(cmd.ExecuteNonQuery());
        }
        catch (Exception ex)
        {
            throw new KiwiError(token, "SqliteError", ex.Message);
        }
    }

    private static Value Query(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, SqliteBuiltin.Query, 2, args.Count);
        var conn = GetConnection(token, SqliteBuiltin.Query, args[0]);
        ParameterTypeMismatchError.ExpectString(token, SqliteBuiltin.Query, 1, args[1]);

        try
        {
            using var cmd = conn.CreateCommand();
            cmd.CommandText = args[1].GetString();
            return ExecuteQuery(cmd);
        }
        catch (Exception ex)
        {
            throw new KiwiError(token, "SqliteError", ex.Message);
        }
    }

    private static Value QueryParams(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, SqliteBuiltin.QueryParams, 3, args.Count);
        var conn = GetConnection(token, SqliteBuiltin.QueryParams, args[0]);
        ParameterTypeMismatchError.ExpectString(token, SqliteBuiltin.QueryParams, 1, args[1]);

        try
        {
            using var cmd = conn.CreateCommand();
            cmd.CommandText = args[1].GetString();
            BindParams(cmd, args[2]);
            return ExecuteQuery(cmd);
        }
        catch (Exception ex)
        {
            throw new KiwiError(token, "SqliteError", ex.Message);
        }
    }

    private static Value LastInsertId(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, SqliteBuiltin.LastInsertId, 1, args.Count);
        var conn = GetConnection(token, SqliteBuiltin.LastInsertId, args[0]);

        try
        {
            using var cmd = conn.CreateCommand();
            cmd.CommandText = "SELECT last_insert_rowid()";
            return Value.CreateInteger((long)(cmd.ExecuteScalar() ?? 0L));
        }
        catch (Exception ex)
        {
            throw new KiwiError(token, "SqliteError", ex.Message);
        }
    }

    private static Value Changes(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, SqliteBuiltin.Changes, 1, args.Count);
        var conn = GetConnection(token, SqliteBuiltin.Changes, args[0]);

        try
        {
            using var cmd = conn.CreateCommand();
            cmd.CommandText = "SELECT changes()";
            return Value.CreateInteger((long)(cmd.ExecuteScalar() ?? 0L));
        }
        catch (Exception ex)
        {
            throw new KiwiError(token, "SqliteError", ex.Message);
        }
    }

    private static void BindParams(SqliteCommand cmd, Value paramList)
    {
        if (paramList.IsNull()) 
        {
            return;
        }

        var items = paramList.GetList();
        for (var i = 0; i < items.Count; i++)
        {
            var p = cmd.CreateParameter();
            p.ParameterName = $"@p{i + 1}";
            p.Value = KiwiToSqlite(items[i]);
            cmd.Parameters.Add(p);
        }
    }

    private static object KiwiToSqlite(Value v)
    {
        if (v.IsNull())    return DBNull.Value;
        if (v.IsInteger()) return v.GetInteger();
        if (v.IsFloat())   return v.GetFloat();
        if (v.IsBoolean()) return v.GetBoolean() ? 1L : 0L;
        return v.GetString();
    }

    private static Value ExecuteQuery(SqliteCommand cmd)
    {
        using var reader = cmd.ExecuteReader();
        var rows = new List<Value>();
        var fieldCount = reader.FieldCount;
        var columnNames = new string[fieldCount];

        for (var i = 0; i < fieldCount; i++)
        {
            columnNames[i] = reader.GetName(i);
        }

        while (reader.Read())
        {
            var row = new Dictionary<Value, Value>(fieldCount);
            for (var i = 0; i < fieldCount; i++)
            {
                var key = Value.CreateString(columnNames[i]);
                var val = reader.IsDBNull(i) ? Value.CreateNull() : SqliteToKiwi(reader, i);
                row[key] = val;
            }
            rows.Add(Value.CreateHashmap(row));
        }

        return Value.CreateList(rows);
    }

    private static Value SqliteToKiwi(SqliteDataReader reader, int i)
    {
        var type = reader.GetFieldType(i);
        if (type == typeof(long))   return Value.CreateInteger(reader.GetInt64(i));
        if (type == typeof(double)) return Value.CreateFloat(reader.GetDouble(i));
        if (type == typeof(bool))   return Value.CreateBoolean(reader.GetBoolean(i));
        return Value.CreateString(reader.GetString(i));
    }
}
