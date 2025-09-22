import * as vscode from 'vscode';
import { spawn } from 'child_process';
import * as path from 'path';

function runCli(args: string[], cwd?: string): Promise<{ code: number, stdout: string, stderr: string }>
{
    return new Promise((resolve) => {
        const cliName = 'yini';
        const proc = spawn(cliName, args, { cwd, shell: true });
        let stdout = '';
        let stderr = '';
        proc.stdout.on('data', d => stdout += d.toString());
        proc.stderr.on('data', d => stderr += d.toString());
        proc.on('close', code => resolve({ code: code ?? -1, stdout, stderr }));
    });
}

async function compileActive()
{
    const editor = vscode.window.activeTextEditor;
    if (!editor) { return; }
    const doc = editor.document;
    if (!doc.fileName.match(/\.yini$/i)) { vscode.window.showWarningMessage('Not a YINI file'); return; }
    await doc.save();
    const { code, stdout, stderr } = await runCli(['compile', doc.fileName], path.dirname(doc.fileName));
    if (code === 0) vscode.window.showInformationMessage(stdout.trim() || 'Compiled');
    else vscode.window.showErrorMessage(stderr.trim() || 'Compile failed');
}

async function decompilePick()
{
    const picked = await vscode.window.showOpenDialog({ filters: { YMETA: ['ymeta', 'YMETA'] }, canSelectMany: false });
    if (!picked || picked.length === 0) { return; }
    const target = picked[0].fsPath;
    const { code, stdout, stderr } = await runCli(['decompile', target], path.dirname(target));
    if (code === 0)
    {
        const doc = await vscode.workspace.openTextDocument({ content: stdout, language: 'yini' });
        vscode.window.showTextDocument(doc);
    }
    else vscode.window.showErrorMessage(stderr.trim() || 'Decompile failed');
}

function provideDiagnostics(doc: vscode.TextDocument, collection: vscode.DiagnosticCollection)
{
    const diagnostics: vscode.Diagnostic[] = [];
    const text = doc.getText();
    const sectionHeader = /^\s*\[(#[a-zA-Z]+|[^\]]+)\](\s*:\s*[^\n]+)?\s*$/;
    const keyValue = /^\s*([A-Za-z_][\w\-]*)\s*=\s*(.+)$/;
    const listAdd = /^\s*\+=\s+(.+)$/;
    const lines = text.split(/\r?\n/);
    for (let i = 0; i < lines.length; i++)
    {
        const line = lines[i];
        if (/^\s*(\/\/|\/\*)/.test(line) || line.trim() === '') continue;
        if (sectionHeader.test(line)) continue;
        if (keyValue.test(line)) continue;
        if (listAdd.test(line)) continue;
        const range = new vscode.Range(i, 0, i, line.length);
        diagnostics.push(new vscode.Diagnostic(range, 'Unrecognized YINI syntax', vscode.DiagnosticSeverity.Warning));
    }
    collection.set(doc.uri, diagnostics);
}

const completionItems: vscode.CompletionItem[] = [
    { label: '[#define]', kind: vscode.CompletionItemKind.Snippet, insertText: new vscode.SnippetString('[#define]\n$0') },
    { label: '[#include]', kind: vscode.CompletionItemKind.Snippet, insertText: new vscode.SnippetString('[#include]\n+= $0') },
    { label: '[Section]', kind: vscode.CompletionItemKind.Snippet, insertText: new vscode.SnippetString('[${1:Section}]$0') },
    { label: 'key = value', kind: vscode.CompletionItemKind.Snippet, insertText: new vscode.SnippetString('${1:key} = ${2:value}$0') },
    { label: '+= value', kind: vscode.CompletionItemKind.Snippet, insertText: new vscode.SnippetString('+= ${1:value}$0') },
    { label: 'true', kind: vscode.CompletionItemKind.Keyword },
    { label: 'false', kind: vscode.CompletionItemKind.Keyword }
];

export function activate(context: vscode.ExtensionContext)
{
    const diagCollection = vscode.languages.createDiagnosticCollection('yini');
    context.subscriptions.push(diacCollectionCleanup(diagCollection));

    context.subscriptions.push(
        vscode.workspace.onDidOpenTextDocument(doc => { if (doc.languageId === 'yini') provideDiagnostics(doc, diagCollection); }),
        vscode.workspace.onDidChangeTextDocument(e => { if (e.document.languageId === 'yini') provideDiagnostics(e.document, diagCollection); }),
        vscode.workspace.onDidSaveTextDocument(doc => { if (doc.languageId === 'yini') provideDiagnostics(doc, diagCollection); })
    );

    if (vscode.window.activeTextEditor?.document.languageId === 'yini')
    {
        provideDiagnostics(vscode.window.activeTextEditor.document, diagCollection);
    }

    context.subscriptions.push(vscode.languages.registerCompletionItemProvider('yini', {
        provideCompletionItems() { return completionItems; }
    }));

    context.subscriptions.push(vscode.commands.registerCommand('yini.compile', compileActive));
    context.subscriptions.push(vscode.commands.registerCommand('yini.decompile', decompilePick));
    context.subscriptions.push(vscode.commands.registerCommand('yini.openDocs', async () => {
        const panel = vscode.window.createWebviewPanel('yiniDocs', 'YINI Docs', vscode.ViewColumn.Beside, { enableScripts: true });
        const html = await getDocsHtml(context);
        panel.webview.html = html;
    }));
}

function diacCollectionCleanup(collection: vscode.DiagnosticCollection): vscode.Disposable
{
    return { dispose() { collection.clear(); collection.dispose(); } };
}

async function getDocsHtml(context: vscode.ExtensionContext): Promise<string>
{
    const docPath = vscode.Uri.joinPath(context.extensionUri, 'docs', 'index.html');
    try
    {
        const bytes = await vscode.workspace.fs.readFile(docPath);
        return Buffer.from(bytes).toString('utf8');
    }
    catch
    {
        return '<html><body><h1>YINI Docs</h1><p>No docs bundled.</p></body></html>';
    }
}

export function deactivate() {}

