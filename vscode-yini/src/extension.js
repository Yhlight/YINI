const { workspace, ExtensionContext } = require('vscode');
const { LanguageClient, LanguageClientOptions, ServerOptions, TransportKind } = require('vscode-languageclient/node');
const path = require('path');

let client;

function activate(context) {
    console.log('Activating YINI language server...');

    const serverCommand = context.asAbsolutePath(path.join('..', 'build', 'bin', 'yini-ls'));
    console.log(`Server command: ${serverCommand}`);

    const serverOptions = {
        command: serverCommand,
        args: [],
        transport: TransportKind.stdio
    };

    const clientOptions = {
        documentSelector: [{ scheme: 'file', language: 'yini' }],
        synchronize: {
            fileEvents: workspace.createFileSystemWatcher('**/*.yini')
        }
    };

    client = new LanguageClient(
        'yiniLanguageServer',
        'YINI Language Server',
        serverOptions,
        clientOptions
    );

    client.start();
    console.log('YINI Language Server started.');
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
