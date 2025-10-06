public class ApiSettings
{
    public const string SectionName = "ApiSettings";

    public string ApiKey { get; set; } = string.Empty;
    public int RateLimit { get; set; } = 100;
    public bool EnableCaching { get; set; } = true;
}