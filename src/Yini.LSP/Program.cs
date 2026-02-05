using System;
using System.IO;
using System.Text;
using System.Text.Json;
using System.Text.Json.Serialization;
using Yini;
using Yini.Model;

namespace Yini.LSP
{
    // Minimal LSP types
    public class LspMessage
    {
        [JsonPropertyName("jsonrpc")] public string JsonRpc { get; set; } = "2.0";
        [JsonPropertyName("id")] public int? Id { get; set; }
        [JsonPropertyName("method")] public string Method { get; set; }
        [JsonPropertyName("params")] public JsonElement Params { get; set; }
    }

    public class InitializeResult
    {
        [JsonPropertyName("capabilities")] public ServerCapabilities Capabilities { get; set; }
    }

    public class ServerCapabilities
    {
        [JsonPropertyName("textDocumentSync")] public int TextDocumentSync { get; set; } = 1; // Full sync
        [JsonPropertyName("hoverProvider")] public bool HoverProvider { get; set; } = true;
        [JsonPropertyName("completionProvider")] public CompletionOptions CompletionProvider { get; set; } = new CompletionOptions();
    }

    public class CompletionOptions
    {
        [JsonPropertyName("resolveProvider")] public bool ResolveProvider { get; set; } = false;
    }

    public class PublishDiagnosticsParams
    {
        [JsonPropertyName("uri")] public string Uri { get; set; }
        [JsonPropertyName("diagnostics")] public Diagnostic[] Diagnostics { get; set; }
    }

    public class Diagnostic
    {
        [JsonPropertyName("range")] public Range Range { get; set; }
        [JsonPropertyName("message")] public string Message { get; set; }
        [JsonPropertyName("severity")] public int Severity { get; set; } = 1; // Error
    }

    public class Range
    {
        [JsonPropertyName("start")] public Position Start { get; set; }
        [JsonPropertyName("end")] public Position End { get; set; }
    }

    public class Position
    {
        [JsonPropertyName("line")] public int Line { get; set; }
        [JsonPropertyName("character")] public int Character { get; set; }
    }

    class Program
    {
        static void Main(string[] args)
        {
            var server = new LanguageServer(Console.OpenStandardInput(), Console.OpenStandardOutput());
            server.Run();
        }
    }

    public class LanguageServer
    {
        private readonly Stream _input;
        private readonly Stream _output;
        private bool _running = true;

        public LanguageServer(Stream input, Stream output)
        {
            _input = input;
            _output = output;
        }

        public void Run()
        {
            while (_running)
            {
                try
                {
                    var message = ReadMessage();
                    if (message == null) break;
                    HandleMessage(message);
                }
                catch (Exception)
                {
                    // Ignore errors for robustness
                }
            }
        }

        private LspMessage ReadMessage()
        {
            // Read headers
            int contentLength = 0;
            while (true)
            {
                string line = ReadLine();
                if (string.IsNullOrEmpty(line)) break;
                if (line.StartsWith("Content-Length: "))
                {
                    contentLength = int.Parse(line.Substring(16));
                }
            }

            if (contentLength == 0) return null;

            byte[] buffer = new byte[contentLength];
            int totalRead = 0;
            while (totalRead < contentLength)
            {
                int read = _input.Read(buffer, totalRead, contentLength - totalRead);
                if (read == 0) return null;
                totalRead += read;
            }

            string json = Encoding.UTF8.GetString(buffer);
            return JsonSerializer.Deserialize<LspMessage>(json);
        }

        private string ReadLine()
        {
            var sb = new StringBuilder();
            int b;
            while ((b = _input.ReadByte()) != -1)
            {
                char c = (char)b;
                if (c == '\n') return sb.ToString().TrimEnd('\r');
                sb.Append(c);
            }
            return sb.ToString();
        }

        private void WriteMessage(object result, int? id)
        {
            var response = new { jsonrpc = "2.0", id = id, result = result };
            string json = JsonSerializer.Serialize(response);
            byte[] bytes = Encoding.UTF8.GetBytes(json);

            string header = $"Content-Length: {bytes.Length}\r\n\r\n";
            byte[] headerBytes = Encoding.ASCII.GetBytes(header);

            _output.Write(headerBytes, 0, headerBytes.Length);
            _output.Write(bytes, 0, bytes.Length);
            _output.Flush();
        }

        private void WriteNotification(string method, object paramsObj)
        {
            var response = new { jsonrpc = "2.0", method = method, @params = paramsObj };
            string json = JsonSerializer.Serialize(response);
            byte[] bytes = Encoding.UTF8.GetBytes(json);

            string header = $"Content-Length: {bytes.Length}\r\n\r\n";
            byte[] headerBytes = Encoding.ASCII.GetBytes(header);

            _output.Write(headerBytes, 0, headerBytes.Length);
            _output.Write(bytes, 0, bytes.Length);
            _output.Flush();
        }

        private void HandleMessage(LspMessage message)
        {
            if (message.Method == "initialize")
            {
                WriteMessage(new InitializeResult { Capabilities = new ServerCapabilities() }, message.Id);
            }
            else if (message.Method == "shutdown")
            {
                WriteMessage(null, message.Id);
            }
            else if (message.Method == "exit")
            {
                _running = false;
            }
            else if (message.Method == "textDocument/didOpen" || message.Method == "textDocument/didChange")
            {
                // Validate document
                ValidateDocument(message.Params);
            }
            else if (message.Method == "textDocument/hover")
            {
                HandleHover(message);
            }
            else if (message.Method == "textDocument/completion")
            {
                HandleCompletion(message);
            }
        }

        // Cache the last valid doc for intelligence
        private YiniDocument _lastDoc;

        private void HandleCompletion(LspMessage message)
        {
            if (_lastDoc == null || !message.Params.TryGetProperty("position", out var pos))
            {
                WriteMessage(new System.Collections.Generic.List<object>(), message.Id);
                return;
            }

            int line = pos.GetProperty("line").GetInt32() + 1;
            int col = pos.GetProperty("character").GetInt32() + 1;

            var suggestions = LspHelper.GetCompletion(_lastDoc, line, col);
            var items = new System.Collections.Generic.List<object>();
            foreach(var s in suggestions)
            {
                items.Add(new { label = s, kind = 10 }); // Property
            }

            if (items.Count == 0)
            {
                items.Add(new { label = "true", kind = 21 });
                items.Add(new { label = "false", kind = 21 });
            }

            WriteMessage(items, message.Id);
        }

        private void HandleHover(LspMessage message)
        {
            if (_lastDoc != null && message.Params.TryGetProperty("position", out var pos))
            {
                int line = pos.GetProperty("line").GetInt32() + 1;
                int col = pos.GetProperty("character").GetInt32() + 1;

                string text = LspHelper.GetHover(_lastDoc, line, col);
                if (text != null)
                {
                    var result = new
                    {
                        contents = new { kind = "markdown", value = text }
                    };
                    WriteMessage(result, message.Id);
                    return;
                }
            }
            WriteMessage(null, message.Id);
        }

        private void ValidateDocument(JsonElement paramsEl)
        {
            string uri = "";
            string text = "";

            if (paramsEl.TryGetProperty("textDocument", out var doc))
            {
                if (doc.TryGetProperty("uri", out var u)) uri = u.GetString();

                // For didChange, it might be contentChanges array. For didOpen it's text.
                if (doc.TryGetProperty("text", out var t))
                {
                    text = t.GetString();
                }
            }

            if (paramsEl.TryGetProperty("contentChanges", out var changes))
            {
                // Assume full sync, take last change text
                foreach (var change in changes.EnumerateArray())
                {
                    if (change.TryGetProperty("text", out var t)) text = t.GetString();
                }
            }

            if (string.IsNullOrEmpty(uri) || text == null) return;

            var diagnostics = new System.Collections.Generic.List<Diagnostic>();
            try
            {
                var compiler = new Compiler();
                var res = compiler.Compile(text);
                _lastDoc = res; // Cache successful parse

                var validator = new Validator();
                validator.Validate(res);
            }
            catch (YiniException ex)
            {
                diagnostics.Add(new Diagnostic
                {
                    Message = ex.Message,
                    Range = new Range
                    {
                        Start = new Position { Line = ex.Span.Line - 1, Character = ex.Span.Column - 1 },
                        End = new Position { Line = ex.Span.Line - 1, Character = ex.Span.Column }
                    }
                });
            }
            catch (Exception ex)
            {
                 // Generic error
                 diagnostics.Add(new Diagnostic
                {
                    Message = ex.Message,
                    Range = new Range
                    {
                        Start = new Position { Line = 0, Character = 0 },
                        End = new Position { Line = 0, Character = 1 }
                    }
                });
            }

            WriteNotification("textDocument/publishDiagnostics", new PublishDiagnosticsParams
            {
                Uri = uri,
                Diagnostics = diagnostics.ToArray()
            });
        }
    }
}
