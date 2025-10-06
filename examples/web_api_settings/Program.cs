using Microsoft.Extensions.Options;
using Yini.Extensions.Configuration;

var builder = WebApplication.CreateBuilder(args);

// Clear existing providers and add our YINI configuration provider
builder.Configuration.Sources.Clear();
builder.Configuration.AddYiniFile("appsettings.yini", optional: false, reloadOnChange: true);

// Configure the ApiSettings class to be injectable via IOptions
builder.Services.Configure<ApiSettings>(
    builder.Configuration.GetSection(ApiSettings.SectionName)
);

var app = builder.Build();

// A minimal API endpoint to demonstrate using the configuration
app.MapGet("/settings", (IOptions<ApiSettings> apiSettingsOptions) =>
{
    var settings = apiSettingsOptions.Value;
    return Results.Ok(new
    {
        settings.ApiKey,
        settings.RateLimit,
        settings.EnableCaching
    });
});

// A simple endpoint to show the app is running
app.MapGet("/", () => "YINI Web API Example is running. Visit /settings to see the configuration.");

app.Run();