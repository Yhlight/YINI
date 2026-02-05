using System;
using System.IO;
using System.Security.Cryptography;
using System.Text;
using Yini;

namespace Yini.CLI
{
    class Program
    {
        static int Main(string[] args)
        {
            if (args.Length == 0)
            {
                PrintHelp();
                return 1;
            }

            string command = args[0];
            try
            {
                switch (command)
                {
                    case "build":
                        return Build(args);
                    case "validate":
                        return Validate(args);
                    case "format":
                        return Format(args);
                    case "gen-meta":
                        return GenMeta(args);
                    case "gen-cs":
                        return GenCs(args);
                    case "precompile":
                        return Precompile(args);
                    case "doc":
                        return GenDoc(args);
                    default:
                        Console.WriteLine($"Unknown command: {command}");
                        PrintHelp();
                        return 1;
                }
            }
            catch (YiniException ex)
            {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine($"Error: {ex.Message}");
                Console.ResetColor();
                return 1;
            }
            catch (Exception ex)
            {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine($"Unexpected Error: {ex.Message}");
                Console.WriteLine(ex.StackTrace);
                Console.ResetColor();
                return 1;
            }
        }

        static void PrintHelp()
        {
            Console.WriteLine("YINI Compiler CLI");
            Console.WriteLine("Usage:");
            Console.WriteLine("  yini build <file>    Compile YINI file to JSON (debug output)");
            Console.WriteLine("  yini validate <file> Validate YINI file against schemas");
            Console.WriteLine("  yini format <file>   Format/Normalize YINI file");
            Console.WriteLine("  yini gen-meta <file> Generate .ymeta cache file");
            Console.WriteLine("  yini gen-cs <file> <ns> <class> [out.cs] Generate C# class");
            Console.WriteLine("  yini precompile <file> Compile Dyna expressions to bytecode (.ybc)");
            Console.WriteLine("  yini doc <file> [output.md] Generate Markdown documentation");
        }

        static int Build(string[] args)
        {
            if (args.Length < 2)
            {
                Console.WriteLine("Usage: yini build <file/dir> [output]");
                return 1;
            }

            string input = args[1];

            if (Directory.Exists(input))
            {
                return BuildDirectory(input, args.Length > 2 ? args[2] : null);
            }

            if (!File.Exists(input))
            {
                Console.WriteLine($"File not found: {input}");
                return 1;
            }

            return BuildFile(input, args.Length > 2 ? args[2] : null);
        }

        static int BuildDirectory(string inputDir, string outputDir)
        {
            var files = Directory.GetFiles(inputDir, "*.yini", SearchOption.AllDirectories);
            Console.WriteLine($"Found {files.Length} files in {inputDir}. Building parallel...");

            string cachePath = Path.Combine(inputDir, ".yini_cache");
            var cache = BuildCache.Load(cachePath);
            // Cache needs to be thread safe if updated in parallel, or we gather updates and save at end.
            var cacheUpdates = new System.Collections.Concurrent.ConcurrentDictionary<string, string>();

            var exceptions = new System.Collections.Concurrent.ConcurrentQueue<Exception>();
            int successCount = 0;
            int skippedCount = 0;

            System.Threading.Tasks.Parallel.ForEach(files, file =>
            {
                try
                {
                    string content = File.ReadAllText(file);

                    // Simple check: if source hasn't changed, and output exists, skip.
                    // But we don't know dependencies (includes) easily without parsing.
                    // For V1 incremental: Only check source file change.
                    // Users should run clean build if includes change fundamentally.
                    // Or we could parse imports.
                    // Let's rely on Hash check of the main file for now as "File Level Incremental".

                    bool skip = false;
                    if (cache.IsUpToDate(file, content))
                    {
                        // Check if output exists
                        if (outputDir != null)
                        {
                            string relPath = Path.GetRelativePath(inputDir, file);
                            string relDir = Path.GetDirectoryName(relPath);
                            string fileName = Path.GetFileNameWithoutExtension(file);
                            string outPath = Path.Combine(outputDir, relDir, fileName + ".ybin");
                            if (File.Exists(outPath)) skip = true;
                        }
                    }

                    if (skip)
                    {
                        System.Threading.Interlocked.Increment(ref skippedCount);
                        return; // Continue loop
                    }

                    // Proceed to build
                    string outFile = null;
                    if (outputDir != null)
                    {
                        // Mirror structure?
                        string relPath = Path.GetRelativePath(inputDir, file);
                        string relDir = Path.GetDirectoryName(relPath);
                        string fileName = Path.GetFileNameWithoutExtension(file);
                        string outPath = Path.Combine(outputDir, relDir, fileName + ".ybin");

                        Directory.CreateDirectory(Path.GetDirectoryName(outPath));
                        outFile = outPath;
                    }

                    // Logic similar to BuildFile but silent
                    var loader = new PhysicalFileLoader(Path.GetDirectoryName(Path.GetFullPath(file)));
                    var compiler = new Compiler(loader);
                    var doc = compiler.Compile(content, Path.GetDirectoryName(file));

                    if (outFile != null)
                    {
                        using(var fs = File.Create(outFile))
                        {
                            var writer = new YiniBinaryWriter();
                            writer.Write(doc, fs);
                        }
                    }

                    // Record successful hash
                    // Re-calculate hash or assume content didn't change?
                    // Use helper to update our local dict
                    // cache.Update(file, content); // Not thread safe
                    using (var sha = SHA256.Create())
                    {
                        var hash = Convert.ToBase64String(sha.ComputeHash(Encoding.UTF8.GetBytes(content)));
                        cacheUpdates[file] = hash;
                    }

                    System.Threading.Interlocked.Increment(ref successCount);
                }
                catch (Exception ex)
                {
                    exceptions.Enqueue(new Exception($"Error building {file}: {ex.Message}"));
                }
            });

            // Update cache
            foreach(var kv in cacheUpdates)
            {
                cache.FileHashes[kv.Key] = kv.Value;
            }
            if (successCount > 0) cache.Save(cachePath);

            Console.WriteLine($"Built {successCount}, Skipped {skippedCount}, Total {files.Length} files.");
            if (exceptions.Count > 0)
            {
                foreach(var ex in exceptions) Console.WriteLine(ex.Message);
                return 1;
            }
            return 0;
        }

        static int BuildFile(string file, string outFile)
        {
            var loader = new PhysicalFileLoader(Path.GetDirectoryName(Path.GetFullPath(file)));
            var compiler = new Compiler(loader);
            var doc = compiler.Compile(File.ReadAllText(file), Path.GetDirectoryName(file));

            Console.WriteLine($"Successfully compiled {file}");

            if (outFile != null)
            {
                if (outFile.EndsWith(".ybin", StringComparison.OrdinalIgnoreCase))
                {
                    using(var fs = File.Create(outFile))
                    {
                        var writer = new YiniBinaryWriter();
                        writer.Write(doc, fs);
                    }
                    Console.WriteLine($"Binary output written to {outFile}");
                }
                else
                {
                    // Text output
                    var serializer = new Serializer();
                    File.WriteAllText(outFile, serializer.Serialize(doc));
                    Console.WriteLine($"Text output written to {outFile}");
                }
            }
            else
            {
                 // Stdout text
                 var serializer = new Serializer();
                 Console.WriteLine(serializer.Serialize(doc));
            }
            return 0;
        }

        static int Validate(string[] args)
        {
            if (args.Length < 2)
            {
                Console.WriteLine("Usage: yini validate <file>");
                return 1;
            }

            string file = args[1];
            if (!File.Exists(file))
            {
                Console.WriteLine($"File not found: {file}");
                return 1;
            }

            var loader = new PhysicalFileLoader(Path.GetDirectoryName(Path.GetFullPath(file)));
            var compiler = new Compiler(loader);
            var doc = compiler.Compile(File.ReadAllText(file), Path.GetDirectoryName(file));

            var validator = new Validator();
            validator.Validate(doc);

            Console.ForegroundColor = ConsoleColor.Green;
            Console.WriteLine("Validation passed.");
            Console.ResetColor();
            return 0;
        }

        static int Format(string[] args)
        {
            if (args.Length < 2)
            {
                Console.WriteLine("Usage: yini format <file> [output]");
                return 1;
            }

            string file = args[1];
            if (!File.Exists(file))
            {
                Console.WriteLine($"File not found: {file}");
                return 1;
            }

            var loader = new PhysicalFileLoader(Path.GetDirectoryName(Path.GetFullPath(file)));
            var compiler = new Compiler(loader);
            // Compile usually resolves everything. Format might want to preserve structure?
            // The Serializer we have dumps the *resolved* state.
            // A true formatter parses but doesn't resolve includes/macros fully if we want to preserve source structure.
            // But our spec says "Serializer: Serialize(YiniDocument doc)".
            // Let's dump the resolved doc for now.

            var doc = compiler.Compile(File.ReadAllText(file), Path.GetDirectoryName(file));
            var serializer = new Serializer();
            string output = serializer.Serialize(doc);

            if (args.Length > 2)
            {
                File.WriteAllText(args[2], output);
                Console.WriteLine($"Formatted output written to {args[2]}");
            }
            else
            {
                Console.WriteLine(output);
            }
            return 0;
        }

        static int GenMeta(string[] args)
        {
            if (args.Length < 2)
            {
                Console.WriteLine("Usage: yini gen-meta <file>");
                return 1;
            }

            string file = args[1];
            if (!File.Exists(file))
            {
                Console.WriteLine($"File not found: {file}");
                return 1;
            }

            var loader = new PhysicalFileLoader(Path.GetDirectoryName(Path.GetFullPath(file)));
            var compiler = new Compiler(loader);
            var doc = compiler.Compile(File.ReadAllText(file), Path.GetDirectoryName(file));

            string metaFile = file + ".ymeta"; // e.g. config.yini.ymeta
            using (var fs = File.Create(metaFile))
            {
                var generator = new MetaGenerator();
                generator.Generate(doc, fs);
            }
            Console.WriteLine($"Generated {metaFile}");
            return 0;
        }

        static int GenCs(string[] args)
        {
            if (args.Length < 4)
            {
                Console.WriteLine("Usage: yini gen-cs <file> <namespace> <class> [output.cs]");
                return 1;
            }

            string file = args[1];
            string ns = args[2];
            string cls = args[3];
            string outPath = args.Length > 4 ? args[4] : cls + ".cs";

            if (!File.Exists(file))
            {
                Console.WriteLine($"File not found: {file}");
                return 1;
            }

            var loader = new PhysicalFileLoader(Path.GetDirectoryName(Path.GetFullPath(file)));
            var compiler = new Compiler(loader);
            var doc = compiler.Compile(File.ReadAllText(file), Path.GetDirectoryName(file));

            var generator = new CodeGenerator();
            string code = generator.GenerateCSharp(doc, ns, cls);

            File.WriteAllText(outPath, code);
            Console.WriteLine($"Generated C# code to {outPath}");
            return 0;
        }

        static int Precompile(string[] args)
        {
            if (args.Length < 2)
            {
                Console.WriteLine("Usage: yini precompile <file>");
                return 1;
            }

            string file = args[1];
            if (!File.Exists(file))
            {
                Console.WriteLine($"File not found: {file}");
                return 1;
            }

            var loader = new PhysicalFileLoader(Path.GetDirectoryName(Path.GetFullPath(file)));
            var compiler = new Compiler(loader);
            var doc = compiler.Compile(File.ReadAllText(file), Path.GetDirectoryName(file));

            // Find all Dyna values
            var bytecodeCompiler = new Yini.Bytecode.BytecodeCompiler();
            using(var ms = new MemoryStream())
            using(var writer = new BinaryWriter(ms))
            {
                // Write Header for YBC Container (Collection of expressions)
                writer.Write(new char[] { 'Y', 'B', 'C', 'C' }); // YBC Container

                int count = 0;
                long countPos = ms.Position;
                writer.Write(count); // Placeholder

                foreach(var section in doc.Sections.Values)
                {
                    foreach(var prop in section.Properties)
                    {
                        if (prop.Value is Yini.Model.YiniDyna dyna)
                        {
                            // Key: Section.Prop
                            string key = $"{section.Name}.{prop.Key}";
                            writer.Write(key);

                            // Compile expression string
                            var lexer = new Lexer(dyna.Expression);
                            var parser = new Parser(lexer.Tokenize());
                            var ast = parser.ParseExpression();
                            byte[] code = bytecodeCompiler.Compile(ast);

                            writer.Write(code.Length);
                            writer.Write(code);
                            count++;
                        }
                    }
                }

                // Update count
                ms.Position = countPos;
                writer.Write(count);

                string outFile = file + ".ybc";
                File.WriteAllBytes(outFile, ms.ToArray());
                Console.WriteLine($"Precompiled {count} expressions to {outFile}");
            }
            return 0;
        }

        static int GenDoc(string[] args)
        {
            if (args.Length < 2)
            {
                Console.WriteLine("Usage: yini doc <file> [output.md]");
                return 1;
            }

            string file = args[1];
            string outFile = args.Length > 2 ? args[2] : Path.GetFileNameWithoutExtension(file) + ".md";

            if (!File.Exists(file))
            {
                Console.WriteLine($"File not found: {file}");
                return 1;
            }

            var loader = new PhysicalFileLoader(Path.GetDirectoryName(Path.GetFullPath(file)));
            var compiler = new Compiler(loader);
            var doc = compiler.Compile(File.ReadAllText(file), Path.GetDirectoryName(file));

            var sb = new StringBuilder();
            sb.AppendLine($"# Configuration Documentation: {Path.GetFileName(file)}");
            sb.AppendLine();

            foreach(var schemaKv in doc.Schemas)
            {
                sb.AppendLine($"## Section `[{schemaKv.Key}]`");
                // TODO: Add comments/description if we parsed them. Currently AST doesn't store comments.
                sb.AppendLine("| Key | Type | Required | Default | Range |");
                sb.AppendLine("| --- | --- | :---: | --- | --- |");

                foreach(var prop in schemaKv.Value.Properties)
                {
                    if (prop.Value is Yini.Model.YiniSchemaDefinition def)
                    {
                        string req = def.Requirement == Yini.Model.SchemaRequirement.Required ? "Yes" : "No";
                        string defVal = def.DefaultValue != null ? def.DefaultValue.ToString() : "-";
                        string range = "";
                        if (def.Min != null) range += $"Min: {def.Min} ";
                        if (def.Max != null) range += $"Max: {def.Max}";

                        sb.AppendLine($"| `{prop.Key}` | `{def.TypeName}` | {req} | {defVal} | {range} |");
                    }
                }
                sb.AppendLine();
            }

            File.WriteAllText(outFile, sb.ToString());
            Console.WriteLine($"Documentation generated to {outFile}");
            return 0;
        }
    }
}
