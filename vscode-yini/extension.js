const vscode = require('vscode');
const cp = require('child_process');
const fs = require('fs');
const path = require('path');

let diagnosticCollection;

class YiniCompletionItemProvider {
    provideCompletionItems(document, position, token, context) {
        const linePrefix = document.lineAt(position).text.substr(0, position.character);

        // Suggest macros after '@'
        if (linePrefix.endsWith('@')) {
            const text = document.getText();
            const macroRegex = /\[#define\]\s*([\s\S]*?)(?=\[|$)/g;
            const defineBlockMatch = macroRegex.exec(text);
            if (defineBlockMatch && defineBlockMatch[1]) {
                const defineBlock = defineBlockMatch[1];
                const keyRegex = /^([a-zA-Z0-9_]+)\s*=/gm;
                let match;
                const completions = [];
                while ((match = keyRegex.exec(defineBlock)) !== null) {
                    completions.push(new vscode.CompletionItem(match[1], vscode.CompletionItemKind.Variable));
                }
                return completions;
            }
        }

        // For simplicity, we won't implement full context-aware section/key completion
        // in this environment as it would require a much more complex parser on the JS side.
        // This structure is a placeholder for a more robust implementation.

        return undefined;
    }
}


function activate(context) {
    diagnosticCollection = vscode.languages.createDiagnosticCollection('yini');
    context.subscriptions.push(diagnosticCollection);

    // Register Completion Provider
    context.subscriptions.push(
        vscode.languages.registerCompletionItemProvider('yini', new YiniCompletionItemProvider(), '@')
    );

    // Diagnostics logic from before
    context.subscriptions.push(
        vscode.workspace.onDidOpenTextDocument(doc => {
            if (doc.languageId === 'yini') {
                updateDiagnostics(doc);
            }
        })
    );
    context.subscriptions.push(
        vscode.workspace.onDidChangeTextDocument(e => {
            if (e.document.languageId === 'yini') {
                updateDiagnostics(e.document);
            }
        })
    );
    context.subscriptions.push(
        vscode.workspace.onDidCloseTextDocument(doc => {
            diagnosticCollection.delete(doc.uri);
        })
    );
    if (vscode.window.activeTextEditor) {
        if (vscode.window.activeTextEditor.document.languageId === 'yini') {
            updateDiagnostics(vscode.window.activeTextEditor.document);
        }
    }
}

function updateDiagnostics(document) {
    const cliPath = vscode.workspace.getConfiguration('yini').get('cli.path');
    if (!cliPath || !fs.existsSync(cliPath)) {
        return;
    }

    const tempFilePath = path.join(path.dirname(document.uri.fsPath), `~${path.basename(document.uri.fsPath)}.tmp`);
    fs.writeFileSync(tempFilePath, document.getText());

    cp.exec(`"${cliPath}" "${tempFilePath}"`, (err, stdout, stderr) => {
        fs.unlinkSync(tempFilePath);

        const diagnostics = [];
        if (stderr) {
            const regex = /\[(\d+):(\d+)\]: (.*)/;
            const match = stderr.match(regex);

            if (match) {
                const line = parseInt(match[1], 10) - 1;
                const column = parseInt(match[2], 10) - 1;
                const message = match[3];

                const range = new vscode.Range(line, column, line, 100);
                const diagnostic = new vscode.Diagnostic(range, message, vscode.DiagnosticSeverity.Error);
                diagnostics.push(diagnostic);
            }
        }

        diagnosticCollection.set(document.uri, diagnostics);
    });
}

function deactivate() {
    diagnosticCollection.clear();
}

module.exports = {
    activate,
    deactivate
};