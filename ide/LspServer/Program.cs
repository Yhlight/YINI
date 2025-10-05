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
using System.Collections.Generic;
using MediatR;
using OmniSharp.Extensions.LanguageServer.Protocol.Client.Capabilities;
using OmniSharp.Extensions.LanguageServer.Protocol.Server.Capabilities;
using OmniSharp.Extensions.LanguageServer.Server; // This was the missing using directive

namespace Yini.Lsp
{
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
                        services.AddSingleton<TextDocumentHandler>();
                    })
                    .WithHandler<TextDocumentHandler>()
            );

            await server.WaitForExit;
        }
    }

    class TextDocumentHandler : ITextDocumentSyncHandler, ICompletionHandler
    {
        private readonly ILanguageServerFacade _router;
        private readonly ILogger<TextDocumentHandler> _logger;
        private readonly YiniManager _yiniManager;

        public TextDocumentHandler(ILanguageServerFacade router, ILogger<TextDocumentHandler> logger)
        {
            _router = router;
            _logger = logger;
            _yiniManager = new YiniManager();
        }

        public TextDocumentSyncKind Change => TextDocumentSyncKind.Full;

        private void ValidateAndPublishDiagnostics(DocumentUri uri, string text)
        {
            try
            {
                _yiniManager.LoadFromString(text, uri.ToString());
                _router.TextDocument.PublishDiagnostics(new PublishDiagnosticsParams
                {
                    Uri = uri,
                    Diagnostics = new Container<Diagnostic>()
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
                        new Position(ex.Line > 0 ? ex.Line - 1 : 0, ex.Column > 0 ? ex.Column - 1 : 0)
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
            if (request.Context.TriggerCharacter == "@")
            {
                var macroNames = _yiniManager.GetMacroNames();
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