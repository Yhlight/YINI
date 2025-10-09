const { workspace, ExtensionContext } = require('vscode');
const { LanguageClient, LanguageClientOptions, ServerOptions, TransportKind } = require('vscode-languageclient/node');
const path = require('path');

let client;

function activate(context) {
    // The server is implemented in C++
    // The path needs to be adjusted depending on the installation location.
    // For development, we assume it's in the project's build directory.
    const serverCommand = path.join(context.extensionPath, '..', 'build', 'bin', 'yini-langserver');

    const serverOptions = {
        command: serverCommand,
        args: [],
        transport: TransportKind.stdio
    };

    // Options to control the language client
    const clientOptions = {
        documentSelector: [{ scheme: 'file', language: 'yini' }],
        synchronize: {
            // Notify the server about file changes to '.yini' files contained in the workspace
            fileEvents: workspace.createFileSystemWatcher('**/*.yini')
        }
    };

    // Create the language client and start the client.
    client = new LanguageClient(
        'yiniLanguageServer',
        'YINI Language Server',
        serverOptions,
        clientOptions
    );

    // Start the client. This will also launch the server
    client.start();
}

function deactivate() {
    if (!client) {
        return undefined;
    }
    return client.stop();
}

module.exports = {
    activate,
    deactivate
};