const vscode = require('vscode');
const { LanguageClient, TransportKind } = require('vscode-languageclient/node');
const path = require('path');

let client;

function activate(context) {
    // LSP server executable path
    // Assumes yini_lsp is in PATH or relative to extension
    const serverCommand = process.env.YINI_LSP_PATH || 'yini_lsp';
    
    // Server options - use stdio for communication
    const serverOptions = {
        command: serverCommand,
        args: [],
        transport: TransportKind.stdio
    };
    
    // Client options - which files to monitor
    const clientOptions = {
        documentSelector: [
            { scheme: 'file', language: 'yini' }
        ],
        synchronize: {
            // Notify server about file changes to .yini files
            fileEvents: vscode.workspace.createFileSystemWatcher('**/*.yini')
        }
    };
    
    // Create and start the language client
    client = new LanguageClient(
        'yiniLanguageServer',
        'YINI Language Server',
        serverOptions,
        clientOptions
    );
    
    // Start the client (and launch the server)
    client.start();
    
    console.log('YINI Language Server activated');
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
