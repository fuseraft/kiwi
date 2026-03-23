using System.Net;
using System.Net.Mail;
using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Settings;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;

public static class SmtpBuiltinHandler
{
    public static Value Execute(Token token, TokenName builtin, List<Value> args)
    {
        if (Kiwi.Settings.SafeMode) throw new SafeModeError(token, "smtp");
        return builtin switch
        {
            TokenName.Builtin_Smtp_Send => Send(token, args),
            _ => throw new FunctionUndefinedError(token, token.Text),
        };
    }

    // args: host, port, from, to_list, subject, body, username, password, use_tls
    private static Value Send(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, SmtpBuiltin.Send, 9, args.Count);
        ParameterTypeMismatchError.ExpectString(token, SmtpBuiltin.Send, 0, args[0]);
        ParameterTypeMismatchError.ExpectInteger(token, SmtpBuiltin.Send, 1, args[1]);
        ParameterTypeMismatchError.ExpectString(token, SmtpBuiltin.Send, 2, args[2]);
        ParameterTypeMismatchError.ExpectList(token, SmtpBuiltin.Send, 3, args[3]);
        ParameterTypeMismatchError.ExpectString(token, SmtpBuiltin.Send, 4, args[4]);
        ParameterTypeMismatchError.ExpectString(token, SmtpBuiltin.Send, 5, args[5]);
        ParameterTypeMismatchError.ExpectString(token, SmtpBuiltin.Send, 6, args[6]);
        ParameterTypeMismatchError.ExpectString(token, SmtpBuiltin.Send, 7, args[7]);
        ParameterTypeMismatchError.ExpectBoolean(token, SmtpBuiltin.Send, 8, args[8]);

        var host     = args[0].GetString();
        var port     = (int)args[1].GetInteger();
        var from     = args[2].GetString();
        var toList   = args[3].GetList();
        var subject  = args[4].GetString();
        var body     = args[5].GetString();
        var username = args[6].GetString();
        var password = args[7].GetString();
        var useTls   = args[8].GetBoolean();

        try
        {
#pragma warning disable SYSLIB0021 // SmtpClient is deprecated but still functional
            using var client = new SmtpClient(host, port)
            {
                EnableSsl        = useTls,
                DeliveryMethod   = SmtpDeliveryMethod.Network,
                UseDefaultCredentials = false,
            };

            if (!string.IsNullOrEmpty(username))
                client.Credentials = new NetworkCredential(username, password);

            using var message = new MailMessage { From = new MailAddress(from), Subject = subject, Body = body };

            foreach (var to in toList)
                message.To.Add(new MailAddress(to.GetString()));

            client.Send(message);
#pragma warning restore SYSLIB0021
            return Value.CreateBoolean(true);
        }
        catch (Exception ex)
        {
            throw new SystemError(token, $"SMTP error: {ex.Message}");
        }
    }
}
