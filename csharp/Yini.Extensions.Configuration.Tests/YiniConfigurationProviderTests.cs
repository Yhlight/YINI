using Microsoft.Extensions.Configuration;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.IO;
using Yini.Extensions.Configuration;

[TestClass]
public class YiniConfigurationProviderTests
{
    private IConfigurationRoot LoadConfiguration(string yiniContent)
    {
        var builder = new ConfigurationBuilder();
        var stream = new MemoryStream();
        var writer = new StreamWriter(stream);
        writer.Write(yiniContent);
        writer.Flush();
        stream.Position = 0;

        builder.Add(new YiniConfigurationSource { FileProvider = null, Path = "test.yini", Optional = false, ReloadOnChange = false });
        var provider = new YiniConfigurationProvider(new YiniConfigurationSource());
        provider.Load(stream);
        builder.Add(provider, false);

        return builder.Build();
    }

    [TestMethod]
    public void Load_ReadsTopLevelKeys()
    {
        var yini = @"
[TopLevel]
Key = ""Value""
Number = 123
IsEnabled = true
";
        var config = LoadConfiguration(yini);

        Assert.AreEqual("Value", config["TopLevel:Key"]);
        Assert.AreEqual("123", config["TopLevel:Number"]);
        Assert.AreEqual("true", config["TopLevel:IsEnabled"]);
    }

    [TestMethod]
    public void Load_ReadsNestedKeys()
    {
        var yini = @"
[Logging:LogLevel]
Default = ""Information""
Microsoft.AspNetCore = ""Warning""
";
        var config = LoadConfiguration(yini);

        Assert.AreEqual("Information", config["Logging:LogLevel:Default"]);
        Assert.AreEqual("Warning", config["Logging:LogLevel:Microsoft.AspNetCore"]);
    }

    [TestMethod]
    public void Load_ReadsSimpleArray()
    {
        var yini = @"
[ArraySection]
Endpoints += ""http://localhost:5000""
Endpoints += ""http://localhost:5001""
";
        var config = LoadConfiguration(yini);

        Assert.AreEqual("http://localhost:5000", config["ArraySection:Endpoints:0"]);
        Assert.AreEqual("http://localhost:5001", config["ArraySection:Endpoints:1"]);
    }

    [TestMethod]
    public void Load_ReadsArrayOfObjects()
    {
        var yini = @"
[ArraySection]
Servers += { ""Ip"": ""192.168.1.1"", ""Port"": 80 }
Servers += { ""Ip"": ""192.168.1.2"", ""Port"": 443 }
";
        var config = LoadConfiguration(yini);

        Assert.AreEqual("192.168.1.1", config["ArraySection:Servers:0:Ip"]);
        Assert.AreEqual("80", config["ArraySection:Servers:0:Port"]);
        Assert.AreEqual("192.168.1.2", config["ArraySection:Servers:1:Ip"]);
        Assert.AreEqual("443", config["ArraySection:Servers:1:Port"]);
    }

    [TestMethod]
    public void AddYiniFile_OptionalFileNotFound_DoesNotThrow()
    {
        var builder = new ConfigurationBuilder();
        builder.AddYiniFile("nonexistent.yini", optional: true);
        var config = builder.Build();
        Assert.IsNotNull(config);
    }

    [TestMethod]
    public void AddYiniFile_NonOptionalFileNotFound_Throws()
    {
        var builder = new ConfigurationBuilder();
        Assert.ThrowsException<FileNotFoundException>(() => builder.AddYiniFile("nonexistent.yini", optional: false).Build());
    }
}