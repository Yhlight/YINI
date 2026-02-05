using System;
using System.IO;
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

            var exceptions = new System.Collections.Concurrent.ConcurrentQueue<Exception>();
            int successCount = 0;

            System.Threading.Tasks.Parallel.ForEach(files, file =>
            {
                try
                {
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
                    var doc = compiler.Compile(File.ReadAllText(file), Path.GetDirectoryName(file));

                    if (outFile != null)
                    {
                        using(var fs = File.Create(outFile))
                        {
                            var writer = new YiniBinaryWriter();
                            writer.Write(doc, fs);
                        }
                    }
                    System.Threading.Interlocked.Increment(ref successCount);
                }
                catch (Exception ex)
                {
                    exceptions.Enqueue(new Exception($"Error building {file}: {ex.Message}"));
                }
            });

            Console.WriteLine($"Built {successCount}/{files.Length} files.");
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
    }
}
