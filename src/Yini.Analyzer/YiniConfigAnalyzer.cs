using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.IO;
using System.Linq;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.Diagnostics;
using Yini;

namespace Yini.Analyzer
{
    [DiagnosticAnalyzer(LanguageNames.CSharp)]
    public class YiniConfigAnalyzer : DiagnosticAnalyzer
    {
        public const string DiagnosticId = "YINI001";

        private static readonly DiagnosticDescriptor Rule = new DiagnosticDescriptor(
            DiagnosticId,
            "YINI Configuration Error",
            "{0}",
            "Configuration",
            DiagnosticSeverity.Error,
            isEnabledByDefault: true,
            description: "YINI configuration file validation failed.");

        public override ImmutableArray<DiagnosticDescriptor> SupportedDiagnostics => ImmutableArray.Create(Rule);

        public override void Initialize(AnalysisContext context)
        {
            context.ConfigureGeneratedCodeAnalysis(GeneratedCodeAnalysisFlags.None);
            context.EnableConcurrentExecution();

            context.RegisterCompilationAction(AnalyzeCompilation);
        }

        private void AnalyzeCompilation(CompilationAnalysisContext context)
        {
            var loader = new AnalyzerFileLoader(context.Options.AdditionalFiles);

            foreach (var file in context.Options.AdditionalFiles)
            {
                // Only analyze .yini files
                if (Path.GetExtension(file.Path).EndsWith("yini", StringComparison.OrdinalIgnoreCase))
                {
                    AnalyzeYiniFile(context, file, loader);
                }
            }
        }

        private void AnalyzeYiniFile(CompilationAnalysisContext context, AdditionalText file, IFileLoader loader)
        {
            var text = file.GetText(context.CancellationToken);
            if (text == null) return;

            string source = text.ToString();
            string path = file.Path;

            try
            {
                var compiler = new Compiler(loader);
                var doc = compiler.Compile(source, Path.GetDirectoryName(path));

                var validator = new Validator();
                validator.Validate(doc);
            }
            catch (YiniException ex)
            {
                var linePosition = new Microsoft.CodeAnalysis.Text.LinePosition(ex.Span.Line - 1, ex.Span.Column - 1);
                var linePosSpan = new Microsoft.CodeAnalysis.Text.LinePositionSpan(linePosition, linePosition);
                // We create a location. We assume the error is in the file we are analyzing.
                // If the error comes from an included file, YiniException.Span.File will tell us.
                // But for simplicity, if File matches, report there. Else report on file 0,0.

                Location location;
                if (ex.Span.File == null || path.EndsWith(ex.Span.File) || ex.Span.File == path)
                {
                     location = Location.Create(path, new Microsoft.CodeAnalysis.Text.TextSpan(0, 0), linePosSpan);
                }
                else
                {
                     // Report that error is in included file
                     location = Location.Create(path, new Microsoft.CodeAnalysis.Text.TextSpan(0, 0), new Microsoft.CodeAnalysis.Text.LinePositionSpan(new Microsoft.CodeAnalysis.Text.LinePosition(0,0), new Microsoft.CodeAnalysis.Text.LinePosition(0,0)));
                     context.ReportDiagnostic(Diagnostic.Create(Rule, location, $"Error in included file '{ex.Span.File}': {ex.Message}"));
                     return;
                }

                var diagnostic = Diagnostic.Create(Rule, location, ex.Message);
                context.ReportDiagnostic(diagnostic);
            }
            catch (Exception ex)
            {
                var diagnostic = Diagnostic.Create(Rule, Location.None, $"Internal Yini Analyzer Error: {ex.Message}");
                context.ReportDiagnostic(diagnostic);
            }
        }
    }

    public class AnalyzerFileLoader : IFileLoader
    {
        private readonly ImmutableArray<AdditionalText> _files;

        public AnalyzerFileLoader(ImmutableArray<AdditionalText> files)
        {
            _files = files;
        }

        public bool Exists(string path)
        {
            // Simple check: does any additional file end with this path?
            // This is loose, but Analyzers paths are absolute.
            // Yini compiler might resolve relative paths.
            // We need to match.
            return FindFile(path) != null;
        }

        public string LoadFile(string path)
        {
            var file = FindFile(path);
            if (file != null) return file.GetText()?.ToString() ?? "";
            throw new FileNotFoundException(path);
        }

        private AdditionalText FindFile(string path)
        {
            foreach(var f in _files)
            {
                // Exact match?
                if (f.Path == path) return f;
                // Or ends with (if relative resolution happened)
                if (f.Path.EndsWith(path, StringComparison.OrdinalIgnoreCase)) return f;
            }
            return null;
        }
    }
}
