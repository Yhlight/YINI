import * as vscode from 'vscode';
import * as cp from 'child_process';
import * as path from 'path';
import * as fs from 'fs';

function getWorkspaceRoot(): string | undefined {
  const folders = vscode.workspace.workspaceFolders;
  return folders && folders.length > 0 ? folders[0].uri.fsPath : undefined;
}

function getCliPath(): string | undefined {
  const root = getWorkspaceRoot();
  if (!root) { return undefined; }
  const candidates = [
    path.join(root, 'yini_cli'),
    path.join(root, 'build', 'yini_cli'),
  ];
  for (const c of candidates) {
    if (fs.existsSync(c)) { return c; }
  }
  return undefined;
}

function runCli(args: string[], cwd?: string): Promise<{ code: number, stdout: string, stderr: string }>
{
  return new Promise((resolve) => {
    const cli = getCliPath();
    const command = cli ?? 'yini_cli';
    const child = cp.spawn(command, args, { cwd, shell: false });
    let stdout = '';
    let stderr = '';
    child.stdout.on('data', d => stdout += d.toString());
    child.stderr.on('data', d => stderr += d.toString());
    child.on('close', code => resolve({ code: code ?? -1, stdout, stderr }));
  });
}

function createDiagnosticsCollection(): vscode.DiagnosticCollection {
  return vscode.languages.createDiagnosticCollection('yini');
}

async function checkDocument(doc: vscode.TextDocument, collection: vscode.DiagnosticCollection)
{
  if (doc.languageId !== 'yini') { return; }
  const filePath = doc.uri.fsPath;
  const root = getWorkspaceRoot();
  const result = await runCli(['check', filePath], root);
  const diagnostics: vscode.Diagnostic[] = [];
  if (result.code !== 0) {
    // Fallback: attach the error to the first line if no location info is present
    const firstLine = doc.lineAt(0);
    const range = new vscode.Range(firstLine.range.start, firstLine.range.end);
    const message = result.stderr.trim() || 'YINI check failed';
    diagnostics.push(new vscode.Diagnostic(range, message, vscode.DiagnosticSeverity.Error));
  }
  collection.set(doc.uri, diagnostics);
}

function registerCompletionProvider(context: vscode.ExtensionContext)
{
  const provider: vscode.CompletionItemProvider = {
    provideCompletionItems(document, position) {
      const suggestions: vscode.CompletionItem[] = [];
      const push = (label: string, detail?: string, insertText?: string) => {
        const item = new vscode.CompletionItem(label, vscode.CompletionItemKind.Keyword);
        item.detail = detail;
        if (insertText) { item.insertText = new vscode.SnippetString(insertText); }
        suggestions.push(item);
      };
      push('[#define]', 'Define macro section', '[#define]\n$0');
      push('[#include]', 'Include files section', '[#include]\n+= ${1:file1.yini}\n$0');
      push('[Section]', 'New section header', '[$1]\n$0');
      push('true', 'Boolean true');
      push('false', 'Boolean false');
      push('+=', 'Register/append entry');
      push('RGB(r,g,b)', 'Color');
      push('#RRGGBB', 'Hex color');
      push('[1, 2, 3]', 'Array');
      push('{{key: value}}', 'Map');
      push('(x, y)', 'Coordinate 2D');
      push('(x, y, z)', 'Coordinate 3D');
      return suggestions;
    }
  };
  context.subscriptions.push(vscode.languages.registerCompletionItemProvider('yini', provider));
}

function registerCommands(context: vscode.ExtensionContext, collection: vscode.DiagnosticCollection)
{
  context.subscriptions.push(vscode.commands.registerCommand('yini.compileCurrent', async () => {
    const editor = vscode.window.activeTextEditor;
    if (!editor || editor.document.languageId !== 'yini') { vscode.window.showWarningMessage('Open a YINI file to compile.'); return; }
    const input = editor.document.uri.fsPath;
    const output = input.replace(/\.[^.]+$/, '.ymeta');
    const root = getWorkspaceRoot();
    const result = await runCli(['compile', input, output], root);
    if (result.code === 0) {
      vscode.window.showInformationMessage(`Compiled to ${path.basename(output)}`);
    } else {
      vscode.window.showErrorMessage(result.stderr || 'Compile failed');
    }
    // Refresh diagnostics after compile
    await checkDocument(editor.document, collection);
  }));

  context.subscriptions.push(vscode.commands.registerCommand('yini.decompileCurrent', async (resource?: vscode.Uri) => {
    const uri = resource ?? vscode.window.activeTextEditor?.document.uri;
    if (!uri) { vscode.window.showWarningMessage('No file selected.'); return; }
    const fsPath = uri.fsPath;
    if (!fsPath.match(/\.ymeta$/i)) { vscode.window.showWarningMessage('Select a .ymeta file to decompile.'); return; }
    const root = getWorkspaceRoot();
    const result = await runCli(['decompile', fsPath], root);
    if (result.code === 0) {
      const doc = await vscode.workspace.openTextDocument({ content: result.stdout, language: 'yini' });
      await vscode.window.showTextDocument(doc, { preview: true });
    } else {
      vscode.window.showErrorMessage(result.stderr || 'Decompile failed');
    }
  }));

  context.subscriptions.push(vscode.commands.registerCommand('yini.openDocs', async () => {
    const root = getWorkspaceRoot();
    if (!root) { vscode.window.showWarningMessage('Open a workspace with YINI sources.'); return; }
    const docsPath = path.join(root, 'YINI.md');
    const uri = vscode.Uri.file(docsPath);
    try {
      const doc = await vscode.workspace.openTextDocument(uri);
      await vscode.commands.executeCommand('markdown.showPreview', doc.uri);
    } catch {
      vscode.window.showWarningMessage('YINI.md not found in workspace.');
    }
  }));

  context.subscriptions.push(vscode.commands.registerCommand('yini.openCli', async () => {
    const root = getWorkspaceRoot();
    const cli = getCliPath();
    const terminal = vscode.window.createTerminal({ name: 'YINI CLI', cwd: root });
    terminal.show();
    terminal.sendText(cli ? `${cli} --help` : `yini_cli --help`);
  }));
}

export function activate(context: vscode.ExtensionContext)
{
  const diagnostics = createDiagnosticsCollection();

  // Run diagnostics on open and save with debounce
  const debounced = new Map<string, NodeJS.Timeout>();
  const scheduleCheck = (doc: vscode.TextDocument) => {
    const key = doc.uri.toString();
    const existing = debounced.get(key);
    if (existing) { clearTimeout(existing); }
    debounced.set(key, setTimeout(() => { checkDocument(doc, diagnostics); }, 300));
  };

  if (vscode.window.activeTextEditor) { scheduleCheck(vscode.window.activeTextEditor.document); }
  context.subscriptions.push(vscode.workspace.onDidOpenTextDocument(doc => scheduleCheck(doc)));
  context.subscriptions.push(vscode.workspace.onDidSaveTextDocument(doc => scheduleCheck(doc)));
  context.subscriptions.push(vscode.workspace.onDidChangeTextDocument(e => scheduleCheck(e.document)));

  registerCompletionProvider(context);
  registerCommands(context, diagnostics);
}

export function deactivate() {
  // no-op
}

