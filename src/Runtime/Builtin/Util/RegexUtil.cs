using System.Text.RegularExpressions;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Util;

public static class RegexUtil
{
    /// <summary>
    /// Searches for the first occurrence of a pattern described by a regex and returns the substring.
    /// </summary>
    /// <param name="text">The string to check.</param>
    /// <param name="pattern">The regular expression.</param>
    /// <returns>The matched substring, or an empty string if no match is found.</returns>
    public static string Find(string text, string pattern)
    {
        try
        {
            Regex regex = new(pattern);
            Match match = regex.Match(text);

            if (match.Success)
            {
                return match.Value;
            }
        }
        catch
        {
            throw;
        }

        return string.Empty;
    }

    /// <summary>
    /// Returns a list of capture groups from the first match of the regex pattern.
    /// </summary>
    /// <param name="text">The string to check.</param>
    /// <param name="pattern">The regular expression.</param>
    /// <returns>A list of capture group strings, excluding the full match.</returns>
    public static List<Value> Match(string text, string pattern)
    {
        List<Value> results = [];
        try
        {
            Regex regex = new(pattern);
            Match match = regex.Match(text);

            if (match.Success)
            {
                // Start from index 1 to skip the full match (group 0)
                for (int i = 1; i < match.Groups.Count; i++)
                {
                    results.Add(Value.CreateString(match.Groups[i].Value));
                }
            }
        }
        catch
        {
            throw;
        }

        return results;
    }

    /// <summary>
    /// Tests whether the entire string conforms to a regular expression pattern.
    /// </summary>
    /// <param name="text">The string to check.</param>
    /// <param name="pattern">The regular expression.</param>
    /// <returns>True if the entire string matches the pattern, false otherwise.</returns>
    public static bool Matches(string text, string pattern)
    {
        try
        {
            Regex regex = new(pattern);
            return regex.IsMatch(text) && regex.Match(text).Length == text.Length;
        }
        catch
        {
            throw;
        }
    }

    /// <summary>
    /// Tests whether the entire string is composed of non-overlapping matches of the regex pattern.
    /// </summary>
    /// <param name="text">The string to check.</param>
    /// <param name="pattern">The regular expression.</param>
    /// <returns>True if the entire string is covered by matches, false otherwise.</returns>
    public static bool MatchesAll(string text, string pattern)
    {
        try
        {
            Regex regex = new(pattern);
            MatchCollection matches = regex.Matches(text);

            int matchesLength = 0;
            foreach (Match match in matches)
            {
                matchesLength += match.Length;
            }

            return matchesLength == text.Length;
        }
        catch
        {
            throw;
        }
    }

    /// <summary>
    /// Splits the string around matches of the given regex. An optional limit can be set for the number of splits.
    /// </summary>
    /// <param name="text">The string to check.</param>
    /// <param name="pattern">The regular expression.</param>
    /// <param name="limit">The maximum number of splits (default -1 for no limit).</param>
    /// <returns>A list of substrings split around regex matches.</returns>
    public static List<Value> RSplit(string text, string pattern, long limit = -1)
    {
        List<Value> result = [];
        try
        {
            Regex regex = new(pattern);

            string[] splits = limit < 0 ? regex.Split(text) : regex.Split(text, (int)limit);
            foreach (var split in splits)
            {
                result.Add(Value.CreateString(split));
            }

            if (limit < 0)
            {
                result.RemoveAll(s => string.IsNullOrEmpty(s.GetString()));
            }
        }
        catch
        {
            throw;
        }

        return result;
    }

    /// <summary>
    /// Replaces every part of the string that matches the regex with the replacement string.
    /// </summary>
    /// <param name="text">The string to check.</param>
    /// <param name="pattern">The regular expression.</param>
    /// <param name="replacement">The replacement text.</param>
    /// <returns>The string with all matches replaced.</returns>
    public static string RReplace(string text, string pattern, string replacement)
    {
        if (string.IsNullOrEmpty(text))
        {
            return text;
        }

        try
        {
            Regex regex = new(pattern);
            return regex.Replace(text, replacement);
        }
        catch
        {
            throw;
        }
    }

    /// <summary>
    /// Finds every occurrence of the regex in the string and returns a list of matches.
    /// </summary>
    /// <param name="text">The string to check.</param>
    /// <param name="pattern">The regular expression.</param>
    /// <returns>A list of all matched substrings.</returns>
    public static List<Value> Scan(string text, string pattern)
    {
        List<Value> matches = [];
        try
        {
            Regex regex = new(pattern);
            MatchCollection matchCollection = regex.Matches(text);

            foreach (Match match in matchCollection)
            {
                matches.Add(Value.CreateString(match.Value));
            }
        }
        catch
        {
            throw;
        }

        return matches;
    }
}