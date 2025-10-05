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
using System.Text; // Required for StringBuilder

namespace Yini.Lsp
{
    internal class DocumentManager : IDisposable
    {
        private readonly ConcurrentDictionary<DocumentUri, YiniManager> _managers = new ConcurrentDictionary<DocumentUri, YiniManager>();

        public YiniManager GetOrAdd(DocumentUri uri)
        {
            return _managers.GetOrAdd(uri, _ => new YiniManager());
        }

        public void Remove(DocumentUri uri)
        {
            if (_managers.TryRemove(uri, out var manager))
            {
                manager.Dispose();
            }
        }

        public void Dispose()
        {
            foreach (var manager in _managers.Values)
            {
                manager.Dispose();
            }
            _managers.Clear();
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
                    .WithHandler<HoverHandler>() // Register the new HoverHandler
            );

            await server.WaitForExit;
        }
    }

    class HoverHandler : IHoverHandler
    {
        private readonly DocumentManager _documentManager;
        private readonly ILogger<HoverHandler> _logger;

        public HoverHandler(DocumentManager documentManager, ILogger<HoverHandler> logger)
        {
            _documentManager = documentManager;
            _logger = logger;
        }

        public HoverRegistrationOptions GetRegistrationOptions(HoverCapability capability, ClientCapabilities clientCapabilities)
        {
            return new HoverRegistrationOptions
            {
                DocumentSelector = DocumentSelector.ForLanguage("yini")
            };
        }

        public Task<Hover?> Handle(HoverParams request, CancellationToken cancellationToken)
        {
            var manager = _documentManager.GetOrAdd(request.TextDocument.Uri);
            var result = manager.FindKeyAtPos(request.Position.Line + 1, request.Position.Character + 1);

            if (result.HasValue)
            {
                (string section, string key) = result.Value;
                using var yiniValue = manager.GetValue(section, key);

                if (yiniValue != null)
                {
                    var sb = new StringBuilder();
                    sb.AppendLine($"**{key}**");
                    sb.AppendLine("---");
                    sb.AppendLine($"*({yiniValue.Type.ToString().ToLower()})*");

                    // For complex types, just show the type name. For simple types, show the value.
                    if (yiniValue.Type != YiniValueType.Array && yiniValue.Type != YiniValueType.Map)
                    {
                         sb.AppendLine($"```\n{yiniValue.AsString()}\n```");
                    }

                    return Task.FromResult<Hover?>(new Hover
                    {
                        Contents = new MarkedStringsOrMarkupContent(new MarkupContent
                        {
                            Kind = MarkupKind.Markdown,
                            Value = sb.ToString()
                        })
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

        private void ValidateAndPublishDiagnostics(DocumentUri uri, string text)
        {
            var yiniManager = _documentManager.GetOrAdd(uri);
            try
            {
                yiniManager.LoadFromString(text, uri.ToString());
                // After successful load, perform schema validation if a schema is present
                var validationErrors = yiniManager.Validate();
                var diagnostics = validationErrors.Select(err => new Diagnostic {
                    Message = err,
                    Severity = DiagnosticSeverity.Warning,
                    // We don't have line/col info for schema errors yet, so we mark the whole doc
                    Range = new OmniSharp.Extensions.LanguageServer.Protocol.Models.Range(0,0,0,0)
                }).ToList();

                _router.TextDocument.PublishDiagnostics(new PublishDiagnosticsParams
                {
                    Uri = uri,
                    Diagnostics = new Container<Diagnostic>(diagnostics)
                });
            }
            catch (YiniException ex)
            {
                var diagnostic = new Diagnostic
                {
                    Message = ex.Message,
                    Severity = DiagnosticSeverity.Error,
                    Range = new OmniSharp.Extensions.LanguageServer.Protocol.Models.Range(
                        new Position(ex.Line > 0 ? ex.Line - 1 : 0, ex.Column > 0 ? ex.Column - 1 : 0),
                        new Position(ex.Line > 0 ? ex.Line - 1 : 0, ex.Column > 0 ? ex.Column + 10 : 10) // Give a bit of range
                    )
                };

                _router.TextDocument.PublishDiagnostics(new PublishDiagnosticsParams
                {
                    Uri = uri,
                    Diagnostics = new Container<Diagnostic>(diagnostic)
                });
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "An unexpected error occurred during validation.");
            }
        }

        public Task<Unit> Handle(DidChangeTextDocumentParams request, CancellationToken cancellationToken)
        {
            ValidateAndPublishDiagnostics(request.TextDocument.Uri, request.ContentChanges.First().Text);
            return Unit.Task;
        }

        public Task<Unit> Handle(DidOpenTextDocumentParams request, CancellationToken cancellationToken)
        {
            ValidateAndPublishDiagnostics(request.TextDocument.Uri, request.TextDocument.Text);
            return Unit.Task;
        }

        public Task<Unit> Handle(DidCloseTextDocumentParams request, CancellationToken cancellationToken)
        {
            _documentManager.Remove(request.TextDocument.Uri);
            _router.TextDocument.PublishDiagnostics(new PublishDiagnosticsParams
            {
                Uri = request.TextDocument.Uri,
                Diagnostics = new Container<Diagnostic>()
            });
            return Unit.Task;
        }

        public Task<Unit> Handle(DidSaveTextDocumentParams request, CancellationToken cancellationToken)
        {
            return Unit.Task;
        }

        public TextDocumentAttributes GetTextDocumentAttributes(DocumentUri uri)
        {
            return new TextDocumentAttributes(uri, "yini");
        }

        public CompletionRegistrationOptions GetRegistrationOptions(CompletionCapability capability, ClientCapabilities clientCapabilities)
        {
            return new CompletionRegistrationOptions
            {
                DocumentSelector = DocumentSelector.ForLanguage("yini"),
                TriggerCharacters = new Container<string>("@")
            };
        }

        public Task<CompletionList> Handle(CompletionParams request, CancellationToken cancellationToken)
        {
            if (request.Context?.TriggerCharacter == "@")
            {
                var yiniManager = _documentManager.GetOrAdd(request.TextDocument.Uri);
                var macroNames = yiniManager.GetMacroNames();
                var items = macroNames.Select(name => new CompletionItem
                {
                    Label = name,
                    Kind = CompletionItemKind.Variable,
                    InsertText = name
                }).ToArray();

                return Task.FromResult(new CompletionList(items));
            }

            return Task.FromResult(new CompletionList());
        }

        TextDocumentChangeRegistrationOptions IRegistration<TextDocumentChangeRegistrationOptions, SynchronizationCapability>.GetRegistrationOptions(SynchronizationCapability capability, ClientCapabilities clientCapabilities)
        {
             return new TextDocumentChangeRegistrationOptions()
            {
                DocumentSelector = DocumentSelector.ForLanguage("yini"),
                SyncKind = Change
            };
        }

        TextDocumentOpenRegistrationOptions IRegistration<TextDocumentOpenRegistrationOptions, SynchronizationCapability>.GetRegistrationOptions(SynchronizationCapability capability, ClientCapabilities clientCapabilities)
        {
            return new TextDocumentOpenRegistrationOptions()
            {
                DocumentSelector = DocumentSelector.ForLanguage("yini")
            };
        }

        TextDocumentCloseRegistrationOptions IRegistration<TextDocumentCloseRegistrationOptions, SynchronizationCapability>.GetRegistrationOptions(SynchronizationCapability capability, ClientCapabilities clientCapabilities)
        {
            return new TextDocumentCloseRegistrationOptions()
            {
                DocumentSelector = DocumentSelector.ForLanguage("yini")
            };
        }

        TextDocumentSaveRegistrationOptions IRegistration<TextDocumentSaveRegistrationOptions, SynchronizationCapability>.GetRegistrationOptions(SynchronizationCapability capability, ClientCapabilities clientCapabilities)
        {
            return new TextDocumentSaveRegistrationOptions()
            {
                DocumentSelector = DocumentSelector.ForLanguage("yini"),
                IncludeText = true
            };
        }
    }
}