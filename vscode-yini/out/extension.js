"use strict";
var __createBinding = (this && this.__createBinding) || (Object.create ? (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    var desc = Object.getOwnPropertyDescriptor(m, k);
    if (!desc || ("get" in desc ? !m.__esModule : desc.writable || desc.configurable)) {
      desc = { enumerable: true, get: function() { return m[k]; } };
    }
    Object.defineProperty(o, k2, desc);
}) : (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    o[k2] = m[k];
}));
var __setModuleDefault = (this && this.__setModuleDefault) || (Object.create ? (function(o, v) {
    Object.defineProperty(o, "default", { enumerable: true, value: v });
}) : function(o, v) {
    o["default"] = v;
});
var __importStar = (this && this.__importStar) || (function () {
    var ownKeys = function(o) {
        ownKeys = Object.getOwnPropertyNames || function (o) {
            var ar = [];
            for (var k in o) if (Object.prototype.hasOwnProperty.call(o, k)) ar[ar.length] = k;
            return ar;
        };
        return ownKeys(o);
    };
    return function (mod) {
        if (mod && mod.__esModule) return mod;
        var result = {};
        if (mod != null) for (var k = ownKeys(mod), i = 0; i < k.length; i++) if (k[i] !== "default") __createBinding(result, mod, k[i]);
        __setModuleDefault(result, mod);
        return result;
    };
})();
Object.defineProperty(exports, "__esModule", { value: true });
exports.activate = activate;
exports.deactivate = deactivate;
const vscode = __importStar(require("vscode"));
const cp = __importStar(require("child_process"));
const path = __importStar(require("path"));
const fs = __importStar(require("fs"));
function getWorkspaceRoot() {
    const folders = vscode.workspace.workspaceFolders;
    return folders && folders.length > 0 ? folders[0].uri.fsPath : undefined;
}
function getCliPath() {
    const root = getWorkspaceRoot();
    if (!root) {
        return undefined;
    }
    const candidates = [
        path.join(root, 'yini_cli'),
        path.join(root, 'build', 'yini_cli'),
    ];
    for (const c of candidates) {
        if (fs.existsSync(c)) {
            return c;
        }
    }
    return undefined;
}
function runCli(args, cwd) {
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
function createDiagnosticsCollection() {
    return vscode.languages.createDiagnosticCollection('yini');
}
async function checkDocument(doc, collection) {
    if (doc.languageId !== 'yini') {
        return;
    }
    const filePath = doc.uri.fsPath;
    const root = getWorkspaceRoot();
    const result = await runCli(['check', filePath], root);
    const diagnostics = [];
    if (result.code !== 0) {
        // Fallback: attach the error to the first line if no location info is present
        const firstLine = doc.lineAt(0);
        const range = new vscode.Range(firstLine.range.start, firstLine.range.end);
        const message = result.stderr.trim() || 'YINI check failed';
        diagnostics.push(new vscode.Diagnostic(range, message, vscode.DiagnosticSeverity.Error));
    }
    collection.set(doc.uri, diagnostics);
}
function registerCompletionProvider(context) {
    const provider = {
        provideCompletionItems(document, position) {
            const suggestions = [];
            const push = (label, detail, insertText) => {
                const item = new vscode.CompletionItem(label, vscode.CompletionItemKind.Keyword);
                item.detail = detail;
                if (insertText) {
                    item.insertText = new vscode.SnippetString(insertText);
                }
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
function registerCommands(context, collection) {
    context.subscriptions.push(vscode.commands.registerCommand('yini.compileCurrent', async () => {
        const editor = vscode.window.activeTextEditor;
        if (!editor || editor.document.languageId !== 'yini') {
            vscode.window.showWarningMessage('Open a YINI file to compile.');
            return;
        }
        const input = editor.document.uri.fsPath;
        const output = input.replace(/\.[^.]+$/, '.ymeta');
        const root = getWorkspaceRoot();
        const result = await runCli(['compile', input, output], root);
        if (result.code === 0) {
            vscode.window.showInformationMessage(`Compiled to ${path.basename(output)}`);
        }
        else {
            vscode.window.showErrorMessage(result.stderr || 'Compile failed');
        }
        // Refresh diagnostics after compile
        await checkDocument(editor.document, collection);
    }));
    context.subscriptions.push(vscode.commands.registerCommand('yini.decompileCurrent', async (resource) => {
        const uri = resource ?? vscode.window.activeTextEditor?.document.uri;
        if (!uri) {
            vscode.window.showWarningMessage('No file selected.');
            return;
        }
        const fsPath = uri.fsPath;
        if (!fsPath.match(/\.ymeta$/i)) {
            vscode.window.showWarningMessage('Select a .ymeta file to decompile.');
            return;
        }
        const root = getWorkspaceRoot();
        const result = await runCli(['decompile', fsPath], root);
        if (result.code === 0) {
            const doc = await vscode.workspace.openTextDocument({ content: result.stdout, language: 'yini' });
            await vscode.window.showTextDocument(doc, { preview: true });
        }
        else {
            vscode.window.showErrorMessage(result.stderr || 'Decompile failed');
        }
    }));
    context.subscriptions.push(vscode.commands.registerCommand('yini.openDocs', async () => {
        const root = getWorkspaceRoot();
        if (!root) {
            vscode.window.showWarningMessage('Open a workspace with YINI sources.');
            return;
        }
        const docsPath = path.join(root, 'YINI.md');
        const uri = vscode.Uri.file(docsPath);
        try {
            const doc = await vscode.workspace.openTextDocument(uri);
            await vscode.commands.executeCommand('markdown.showPreview', doc.uri);
        }
        catch {
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
function activate(context) {
    const diagnostics = createDiagnosticsCollection();
    // Run diagnostics on open and save with debounce
    const debounced = new Map();
    const scheduleCheck = (doc) => {
        const key = doc.uri.toString();
        const existing = debounced.get(key);
        if (existing) {
            clearTimeout(existing);
        }
        debounced.set(key, setTimeout(() => { checkDocument(doc, diagnostics); }, 300));
    };
    if (vscode.window.activeTextEditor) {
        scheduleCheck(vscode.window.activeTextEditor.document);
    }
    context.subscriptions.push(vscode.workspace.onDidOpenTextDocument(doc => scheduleCheck(doc)));
    context.subscriptions.push(vscode.workspace.onDidSaveTextDocument(doc => scheduleCheck(doc)));
    context.subscriptions.push(vscode.workspace.onDidChangeTextDocument(e => scheduleCheck(e.document)));
    registerCompletionProvider(context);
    registerCommands(context, diagnostics);
}
function deactivate() {
    // no-op
}
//# sourceMappingURL=extension.js.map