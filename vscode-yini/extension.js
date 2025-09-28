const vscode = require('vscode');
const cp = require('child_process');
const fs = require('fs');
const path = require('path');

let diagnosticCollection;

function activate(context) {
    diagnosticCollection = vscode.languages.createDiagnosticCollection('yini');
    context.subscriptions.push(diagnosticCollection);

    // Run diagnostics on file open
    context.subscriptions.push(
        vscode.workspace.onDidOpenTextDocument(doc => {
            if (doc.languageId === 'yini') {
                updateDiagnostics(doc);
            }
        })
    );

    // Run diagnostics on file change
    context.subscriptions.push(
        vscode.workspace.onDidChangeTextDocument(e => {
            if (e.document.languageId === 'yini') {
                updateDiagnostics(e.document);
            }
        })
    );

    // Clear diagnostics on file close
    context.subscriptions.push(
        vscode.workspace.onDidCloseTextDocument(doc => {
            diagnosticCollection.delete(doc.uri);
        })
    );

    // Initial check for currently active editor
    if (vscode.window.activeTextEditor) {
        if (vscode.window.activeTextEditor.document.languageId === 'yini') {
            updateDiagnostics(vscode.window.activeTextEditor.document);
        }
    }
}

function updateDiagnostics(document) {
    const cliPath = vscode.workspace.getConfiguration('yini').get('cli.path');
    if (!cliPath || !fs.existsSync(cliPath)) {
        // Silently fail if CLI path is not configured or not found.
        // A more robust extension would show an error message to the user.
        return;
    }

    // We need to write the content to a temporary file because the CLI works on files, not stdin.
    const tempFilePath = path.join(path.dirname(document.uri.fsPath), `~${path.basename(document.uri.fsPath)}.tmp`);
    fs.writeFileSync(tempFilePath, document.getText());

    cp.exec(`"${cliPath}" "${tempFilePath}"`, (err, stdout, stderr) => {
        fs.unlinkSync(tempFilePath); // Clean up the temporary file

        const diagnostics = [];
        if (stderr) {
            // Example error: Syntax Error in tests/invalid_syntax.yini [5:1]: Expected ']' to close section header.
            const regex = /\[(\d+):(\d+)\]: (.*)/;
            const match = stderr.match(regex);

            if (match) {
                const line = parseInt(match[1], 10) - 1; // VSCode is 0-based
                const column = parseInt(match[2], 10) - 1; // VSCode is 0-based
                const message = match[3];

                const range = new vscode.Range(line, column, line, 100); // Highlight a portion of the line
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