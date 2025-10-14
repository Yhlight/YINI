# YINI Cookbook: Practical Examples

Welcome to the YINI Cookbook! This document provides practical examples and tutorials to help you get the most out of the YINI language.

## Advanced Schema Validation

One of YINI's most powerful features is its ability to validate configuration files against a predefined schema. This ensures that your configurations are always correct and complete, preventing common errors at runtime.

### Example: Graphics Settings

Let's look at a common use case: managing graphics settings for a game. We want to define a set of rules that all user graphics settings must follow.

You can find the complete source for this example in the `examples/schema_example.yini` file.

### The Schema (`graphics_schema.yini`)

First, we define our schema in a separate file. This is a best practice, as it keeps your rules separate from your configuration data.

```yini
[#schema]
[GraphicsSettings]
resolution_width = !, int, =1920, min=800
resolution_height = !, int, =1080, min=600
fullscreen = ?, bool, =true
render_quality = ?, string, ="High"
shadow_quality = ?, string, ="Medium"
texture_filtering = ?, string, ="Anisotropic"
vignette_enabled = !, bool, e
```

Let's break down these rules:

*   **`resolution_width`**:
    *   `!`: This key is **required**. The application will throw an error if it's missing.
    *   `int`: The value must be an integer.
    *   `=1920`: If the key is missing, a default value of `1920` will be applied.
    *   `min=800`: The value must be at least `800`.

*   **`fullscreen`**:
    *   `?`: This key is **optional**.
    *   `bool`: The value must be a boolean (`true` or `false`).
    *   `=true`: If the key is missing, it will default to `true`.

*   **`vignette_enabled`**:
    *   `!`: This key is **required**.
    *   `bool`: The value must be a boolean.
    *   `e`: If the key is missing, **throw an error**. This overrides any default (`=`) behavior.

### The Configuration (`user_settings.yini`)

Now, here is a user's configuration file. To apply the schema, the user's config file would typically use an `[#include]` directive to pull in the schema file.

```yini
# [#include]
# += graphics_schema.yini

[GraphicsSettings]
resolution_width = 1280
resolution_height = 720
fullscreen = false
render_quality = "Low"
shadow_quality = "Low"
texture_filtering = "Trilinear"
vignette_enabled = true
```

### How to Use It

You would use the `yini` command-line tool to validate the user's settings against the schema:

```bash
yini validate graphics_schema.yini user_settings.yini
```

If the validation is successful, the tool will exit silently. If there are any errors (for example, if `resolution_width` was `700`, which is less than the `min` of `800`), the tool will print an error message and exit with a non-zero status code.
