using OmniSharp.Extensions.LanguageServer.Protocol.Models;
using OmniSharp.Extensions.LanguageServer.Protocol.Server;
using System.Threading;
using System.Threading.Tasks;
using Microsoft.Extensions.Logging;
using System;
using System.Linq;
using OmniSharp.Extensions.LanguageServer.Protocol.Document;
using OmniSharp.Extensions.LanguageServer.Protocol;
using Microsoft.Extensions.DependencyInjection;
using System.Collections.Concurrent;
using System.Collections.Generic;
using MediatR;
using OmniSharp.Extensions.LanguageServer.Protocol.Client.Capabilities;
using OmniSharp.Extensions.LanguageServer.Protocol.Server.Capabilities;
using OmniSharp.Extensions.LanguageServer.Server;
using System.Text;

namespace Yini.Lsp
{
    /// <summary>
    /// Holds the state for a single document, including its text content and the YiniManager instance.
    /// </summary>
    internal class DocumentState : IDisposable
    {
        public YiniManager Manager { get; } = new YiniManager();
        public string Text { get; set; } = "";

        public void Dispose()
        {
            Manager.Dispose();
        }
    }

    /// <summary>
    /// Manages the state for all open documents.
    /// </summary>
    internal class DocumentManager : IDisposable
    {
        private readonly ConcurrentDictionary<DocumentUri, DocumentState> _documents = new ConcurrentDictionary<DocumentUri, DocumentState>();

        public DocumentState? Get(DocumentUri uri)
        {
            _documents.TryGetValue(uri, out var state);
            return state;
        }

        public DocumentState GetOrAdd(DocumentUri uri, string text)
        {
            var state = _documents.GetOrAdd(uri, _ => new DocumentState());
            state.Text = text;
            return state;
        }

        public void Remove(DocumentUri uri)
        {
            if (_documents.TryRemove(uri, out var state))
            {
                state.Dispose();
            }
        }

        public void Dispose()
        {
            foreach (var state in _documents.Values)
            {
                state.Dispose();
            }
            _documents.Clear();
        }
    }

    internal class Program
    {
        static async Task Main(string[] args)
        {
            var server = await LanguageServer.From(options =>
                options
                    .WithInput(Console.OpenStandardInput())
                    .WithOutput(Console.OpenStandardOutput())
                    .WithLoggerFactory(new LoggerFactory())
                    .AddDefaultLoggingProvider()
                    .WithServices(services =>
                    {
                        services.AddSingleton<DocumentManager>();
                    })
                    .WithHandler<TextDocumentHandler>()
                    .WithHandler<HoverHandler>()
                    .WithHandler<DefinitionHandler>()
            );

            await server.WaitForExit;
        }
    }

    class DefinitionHandler : IDefinitionHandler
    {
        private readonly DocumentManager _documentManager;

        public DefinitionHandler(DocumentManager documentManager)
        {
            _documentManager = documentManager;
        }

        public DefinitionRegistrationOptions GetRegistrationOptions(DefinitionCapability capability, ClientCapabilities clientCapabilities)
        {
            return new DefinitionRegistrationOptions { DocumentSelector = DocumentSelector.ForLanguage("yini") };
        }

        public Task<LocationOrLocationLinks> Handle(DefinitionParams request, CancellationToken cancellationToken)
        {
            var state = _documentManager.Get(request.TextDocument.Uri);
            if (state == null) return Task.FromResult(new LocationOrLocationLinks());

            var manager = state.Manager;
            var symbol = manager.FindKeyAtPos(request.Position.Line + 1, request.Position.Character + 1);
            if (symbol.HasValue)
            {
                var (section, key) = symbol.Value;
                var location = manager.GetDefinitionLocation(section, key);
                if (location.HasValue)
                {
                    return Task.FromResult(new LocationOrLocationLinks(new Location
                    {
                        Uri = DocumentUri.FromFileSystemPath(location.Value.FilePath),
                        Range = new OmniSharp.Extensions.LanguageServer.Protocol.Models.Range(
                            new Position(location.Value.Line - 1, location.Value.Column - 1),
                            new Position(location.Value.Line - 1, location.Value.Column)
                        )
                    }));
                }
            }
            return Task.FromResult(new LocationOrLocationLinks());
        }
    }

    class HoverHandler : IHoverHandler
    {
        private readonly DocumentManager _documentManager;

        public HoverHandler(DocumentManager documentManager)
        {
            _documentManager = documentManager;
        }

        public HoverRegistrationOptions GetRegistrationOptions(HoverCapability capability, ClientCapabilities clientCapabilities)
        {
            return new HoverRegistrationOptions { DocumentSelector = DocumentSelector.ForLanguage("yini") };
        }

        public Task<Hover?> Handle(HoverParams request, CancellationToken cancellationToken)
        {
            var state = _documentManager.Get(request.TextDocument.Uri);
            if (state == null) return Task.FromResult<Hover?>(null);

            var manager = state.Manager;
            var result = manager.FindKeyAtPos(request.Position.Line + 1, request.Position.Character + 1);

            if (result.HasValue)
            {
                (string section, string key) = result.Value;
                using var yiniValue = manager.GetValue(section, key);

                if (yiniValue != null)
                {
                    var sb = new StringBuilder();
                    sb.AppendLine($"**{section}.{key}**");

                    var schemaKeyInfo = manager.GetSchemaKeysForSection(section).FirstOrDefault(k => k.Name == key);
                    if (schemaKeyInfo != null)
                        sb.AppendLine($"**Schema:** `{schemaKeyInfo.TypeName}`" + (schemaKeyInfo.IsRequired ? " (required)" : ""));

                    sb.AppendLine("---");

                    var displayType = yiniValue.Type;
                    if (displayType == YiniValueType.Dyna) {
                        using (var inner = yiniValue.AsDynaValue()) sb.AppendLine($"**Value:** *(dynamic: {inner.Type.ToString().ToLower()})*");
                    } else {
                        sb.AppendLine($"**Value:** *({displayType.ToString().ToLower()})*");
                    }

                    sb.AppendLine("```yini");
                    sb.AppendLine(manager.StringifyValue(yiniValue));
                    sb.AppendLine("```");

                    return Task.FromResult<Hover?>(new Hover {
                        Contents = new MarkedStringsOrMarkupContent(new MarkupContent { Kind = MarkupKind.Markdown, Value = sb.ToString() })
                    });
                }
            }
            return Task.FromResult<Hover?>(null);
        }
    }

    class TextDocumentHandler : ITextDocumentSyncHandler, ICompletionHandler
    {
        private readonly ILanguageServerFacade _router;
        private readonly ILogger<TextDocumentHandler> _logger;
        private readonly DocumentManager _documentManager;

        public TextDocumentHandler(ILanguageServerFacade router, ILogger<TextDocumentHandler> logger, DocumentManager documentManager)
        {
            _router = router;
            _logger = logger;
            _documentManager = documentManager;
        }

        public TextDocumentSyncKind Change => TextDocumentSyncKind.Full;

        private void ValidateAndPublishDiagnostics(DocumentUri uri, DocumentState state)
        {
            try
            {
                state.Manager.LoadFromString(state.Text, uri.ToString());
                var diagnostics = new List<Diagnostic>();

                var validationErrors = state.Manager.Validate();
                diagnostics.AddRange(validationErrors.Select(err => new Diagnostic {
                    Message = err,
                    Severity = DiagnosticSeverity.Warning,
                    Range = new OmniSharp.Extensions.LanguageServer.Protocol.Models.Range(0,0,0,0) // TODO: Get range from error
                }));

                _router.TextDocument.PublishDiagnostics(new PublishDiagnosticsParams { Uri = uri, Diagnostics = new Container<Diagnostic>(diagnostics) });
            }
            catch (YiniException ex)
            {
                _router.TextDocument.PublishDiagnostics(new PublishDiagnosticsParams {
                    Uri = uri,
                    Diagnostics = new Container<Diagnostic>(new Diagnostic {
                        Message = ex.Message,
                        Severity = DiagnosticSeverity.Error,
                        Range = new OmniSharp.Extensions.LanguageServer.Protocol.Models.Range(
                            new Position(ex.Line > 0 ? ex.Line - 1 : 0, ex.Column > 0 ? ex.Column - 1 : 0),
                            new Position(ex.Line > 0 ? ex.Line - 1 : 0, ex.Column > 0 ? ex.Column + 5 : 5)
                        )
                    })
                });
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "An unexpected error occurred during validation.");
            }
        }

        public Task<Unit> Handle(DidChangeTextDocumentParams request, CancellationToken cancellationToken)
        {
            var state = _documentManager.GetOrAdd(request.TextDocument.Uri, request.ContentChanges.First().Text);
            ValidateAndPublishDiagnostics(request.TextDocument.Uri, state);
            return Unit.Task;
        }

        public Task<Unit> Handle(DidOpenTextDocumentParams request, CancellationToken cancellationToken)
        {
            var state = _documentManager.GetOrAdd(request.TextDocument.Uri, request.TextDocument.Text);
            ValidateAndPublishDiagnostics(request.TextDocument.Uri, state);
            return Unit.Task;
        }

        public Task<Unit> Handle(DidCloseTextDocumentParams request, CancellationToken cancellationToken)
        {
            _documentManager.Remove(request.TextDocument.Uri);
            _router.TextDocument.PublishDiagnostics(new PublishDiagnosticsParams { Uri = request.TextDocument.Uri, Diagnostics = new Container<Diagnostic>() });
            return Unit.Task;
        }

        public Task<Unit> Handle(DidSaveTextDocumentParams request, CancellationToken cancellationToken) => Unit.Task;
        public TextDocumentAttributes GetTextDocumentAttributes(DocumentUri uri) => new TextDocumentAttributes(uri, "yini");

        public CompletionRegistrationOptions GetRegistrationOptions(CompletionCapability capability, ClientCapabilities clientCapabilities)
        {
            return new CompletionRegistrationOptions {
                DocumentSelector = DocumentSelector.ForLanguage("yini"),
                TriggerCharacters = new Container<string>("@", " ", "=")
            };
        }

        public Task<CompletionList> Handle(CompletionParams request, CancellationToken cancellationToken)
        {
            var state = _documentManager.Get(request.TextDocument.Uri);
            if (state == null) return Task.FromResult(new CompletionList());

            var items = new List<CompletionItem>();
            var yiniManager = state.Manager;

            // Macro completion
            if (request.Context?.TriggerCharacter == "@")
            {
                items.AddRange(yiniManager.GetMacroNames().Select(name => new CompletionItem {
                    Label = name, Kind = CompletionItemKind.Variable, InsertText = name
                }));
            }

            // Schema-based key completion
            var currentSection = GetCurrentSection(state.Text, request.Position.Line);
            if (currentSection != null)
            {
                var schemaKeys = yiniManager.GetSchemaKeysForSection(currentSection);
                if (schemaKeys.Any())
                {
                    var existingKeys = GetExistingKeys(state.Text, currentSection);
                    var newKeys = schemaKeys.Where(sk => !existingKeys.Contains(sk.Name));
                    items.AddRange(newKeys.Select(k => new CompletionItem {
                        Label = k.Name,
                        Kind = CompletionItemKind.Property,
                        Detail = $"{k.TypeName}{(k.IsRequired ? " (required)" : "")}",
                        InsertText = $"{k.Name} = ",
                        Documentation = new MarkupContent { Kind = MarkupKind.Markdown, Value = $"Type: `{k.TypeName}`\n\nRequired: `{k.IsRequired}`" }
                    }));
                }
            }
            return Task.FromResult(new CompletionList(items));
        }

        private string? GetCurrentSection(string text, int line)
        {
            var lines = text.Split('\n');
            for (int i = line; i >= 0 && i < lines.Length; i--)
            {
                var trimmedLine = lines[i].Trim();
                if (trimmedLine.StartsWith("[") && trimmedLine.EndsWith("]"))
                {
                    var sectionName = trimmedLine.Substring(1, trimmedLine.Length - 2).Trim();
                    var colonPos = sectionName.IndexOf(':');
                    if (colonPos != -1) sectionName = sectionName.Substring(0, colonPos).Trim();
                    return sectionName;
                }
            }
            return null;
        }

        private HashSet<string> GetExistingKeys(string text, string sectionName)
        {
            var keys = new HashSet<string>();
            var lines = text.Split('\n');
            bool inSection = false;
            foreach (var line in lines)
            {
                var trimmedLine = line.Trim();
                if (trimmedLine.StartsWith("[") && trimmedLine.EndsWith("]"))
                {
                    var currentSection = trimmedLine.Substring(1, trimmedLine.Length - 2).Trim();
                    var colonPos = currentSection.IndexOf(':');
                    if (colonPos != -1) currentSection = currentSection.Substring(0, colonPos).Trim();
                    inSection = (currentSection == sectionName);
                }
                else if (inSection)
                {
                    var eqPos = trimmedLine.IndexOf('=');
                    if (eqPos > 0) keys.Add(trimmedLine.Substring(0, eqPos).Trim());
                }
            }
            return keys;
        }

        TextDocumentChangeRegistrationOptions IRegistration<TextDocumentChangeRegistrationOptions, SynchronizationCapability>.GetRegistrationOptions(SynchronizationCapability capability, ClientCapabilities clientCapabilities)
        {
             return new TextDocumentChangeRegistrationOptions() { DocumentSelector = DocumentSelector.ForLanguage("yini"), SyncKind = Change };
        }

        TextDocumentOpenRegistrationOptions IRegistration<TextDocumentOpenRegistrationOptions, SynchronizationCapability>.GetRegistrationOptions(SynchronizationCapability capability, ClientCapabilities clientCapabilities)
        {
            return new TextDocumentOpenRegistrationOptions() { DocumentSelector = DocumentSelector.ForLanguage("yini") };
        }

        TextDocumentCloseRegistrationOptions IRegistration<TextDocumentCloseRegistrationOptions, SynchronizationCapability>.GetRegistrationOptions(SynchronizationCapability capability, ClientCapabilities clientCapabilities)
        {
            return new TextDocumentCloseRegistrationOptions() { DocumentSelector = DocumentSelector.ForLanguage("yini") };
        }

        TextDocumentSaveRegistrationOptions IRegistration<TextDocumentSaveRegistrationOptions, SynchronizationCapability>.GetRegistrationOptions(SynchronizationCapability capability, ClientCapabilities clientCapabilities)
        {
            return new TextDocumentSaveRegistrationOptions() { DocumentSelector = DocumentSelector.ForLanguage("yini"), IncludeText = true };
        }
    }
}