# Releasing YINI

This document outlines the process for creating a new release for the YINI project. The process is fully automated using GitHub Actions.

## How to Create a New Release

The release process is triggered automatically whenever a new tag matching the pattern `v*.*.*` (e.g., `v1.2.0`, `v1.2.1`) is pushed to the repository.

To create a new release, follow these steps:

1.  **Ensure you are on the `main` branch** and have pulled the latest changes.

    ```bash
    git checkout main
    git pull origin main
    ```

2.  **Create a new version tag.** Use semantic versioning.

    ```bash
    git tag v1.2.0
    ```

3.  **Push the tag to the remote repository.**

    ```bash
    git push origin v1.2.0
    ```

## The Automation Process

Pushing a new version tag will trigger the `release.yml` workflow in GitHub Actions. This workflow performs the following steps:

1.  **Builds Native Binaries:** It compiles the C++ core library for Windows (`Yini.dll`), macOS (`libYini.dylib`), and Linux (`libYini.so`).
2.  **Packages Artifacts:**
    -   It creates a cross-platform NuGet package, bundling the native binaries for all three platforms.
    -   It creates a Visual Studio Code extension (`.vsix`) file.
3.  **Publishes Packages:**
    -   The NuGet package is published to the **GitHub Packages** registry associated with the repository.
4.  **Creates a GitHub Release:**
    -   A new, formal release is created on the project's GitHub page, corresponding to the pushed tag.
    -   The `.nupkg` (NuGet) and `.vsix` (VS Code Extension) files are uploaded as downloadable assets to this release.

Once the workflow is complete, the new version will be publicly available for consumption.