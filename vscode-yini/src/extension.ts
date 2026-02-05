import * as path from 'path';
import { workspace, ExtensionContext } from 'vscode';
import {
  LanguageClient,
  LanguageClientOptions,
  ServerOptions,
} from 'vscode-languageclient/node';

let client: LanguageClient;

export function activate(context: ExtensionContext) {
  // The server is implemented in C#
  // Assume the server executable is built and placed in 'bin' folder of extension
  // In dev mode, we might point to the build output of the .NET project

  // Note: Adjust path based on deployment
  let serverExe = context.asAbsolutePath(
    path.join('bin', 'Yini.LSP.dll')
  );

  // Launch dotnet
  let serverOptions: ServerOptions = {
    run: { command: 'dotnet', args: [serverExe] },
    debug: { command: 'dotnet', args: [serverExe] }
  };

  let clientOptions: LanguageClientOptions = {
    documentSelector: [{ scheme: 'file', language: 'yini' }],
    synchronize: {
      fileEvents: workspace.createFileSystemWatcher('**/.clientrc')
    }
  };

  client = new LanguageClient(
    'yiniLanguageServer',
    'YINI Language Server',
    serverOptions,
    clientOptions
  );

  client.start();
}

export function deactivate(): Thenable<void> | undefined {
  if (!client) {
    return undefined;
  }
  return client.stop();
}
