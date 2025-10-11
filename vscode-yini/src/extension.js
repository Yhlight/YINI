const vscode = require('vscode');
const path = require('path');

// --- Load the native addon ---
const validator = require(path.join(__dirname, '../build/Release/yini_validator.node'));

const keywords = [
    'Dyna', 'color', 'coord', 'path', 'list', 'array', 'true', 'false'
];

const sectionHeaders = [
    '#define', '#include', '#schema'
];

function parseYiniDocument(document) {
    const sections = new Map();
    let currentSection = null;
    const sectionRegex = /^\s*\[\s*([^\]]+?)\s*\]/;
    const keyRegex = /^\s*(\w+)\s*=/;

    for (let i = 0; i < document.lineCount; i++) {
        const line = document.lineAt(i);
        const sectionMatch = line.text.match(sectionRegex);
        if (sectionMatch) {
            const sectionName = sectionMatch[1].split(':')[0].trim();
            if (!sections.has(sectionName)) {
                sections.set(sectionName, []);
            }
            currentSection = sectionName;
        } else if (currentSection) {
            const keyMatch = line.text.match(keyRegex);
            if (keyMatch) {
                sections.get(currentSection).push(keyMatch[1]);
            }
        }
    }
    return sections;
}

function updateDiagnostics(document, collection) {
    if (document.languageId !== 'yini') {
        return;
    }

    const errorMessage = validator.validateYiniSource(document.getText());
    const diagnostics = [];

    if (errorMessage) {
        // Try to parse line and column from the error message
        const match = errorMessage.match(/Error at line (\d+), column (\d+): (.*)/);
        if (match) {
            const line = parseInt(match[1], 10) - 1;
            const column = parseInt(match[2], 10) - 1;
            const message = match[3];
            const range = new vscode.Range(line, column, line, 100); // Highlight a portion of the line
            diagnostics.push(new vscode.Diagnostic(range, message, vscode.DiagnosticSeverity.Error));
        } else {
            // If we can't parse the line, show the error on the first line
            const range = new vscode.Range(0, 0, 0, 1);
            diagnostics.push(new vscode.Diagnostic(range, errorMessage, vscode.DiagnosticSeverity.Error));
        }
    }

    collection.set(document.uri, diagnostics);
}


function activate(context) {
    // --- Diagnostics ---
    const diagnosticCollection = vscode.languages.createDiagnosticCollection('yini');
    context.subscriptions.push(diagnosticCollection);

    if (vscode.window.activeTextEditor) {
        updateDiagnostics(vscode.window.activeTextEditor.document, diagnosticCollection);
    }

    context.subscriptions.push(vscode.window.onDidChangeActiveTextEditor(editor => {
        if (editor) {
            updateDiagnostics(editor.document, diagnosticCollection);
        }
    }));

    context.subscriptions.push(vscode.workspace.onDidChangeTextDocument(event => {
        updateDiagnostics(event.document, diagnosticCollection);
    }));

    // --- Auto-completion ---
    const provider = vscode.languages.registerCompletionItemProvider('yini', {
        provideCompletionItems(document, position) {
            const linePrefix = document.lineAt(position).text.substr(0, position.character);
            const sections = parseYiniDocument(document);

            // Cross-section reference completion
            const refMatch = linePrefix.match(/@\{([\w\.]*)$/);
            if (refMatch) {
                const refContent = refMatch[1];
                if (refContent.includes('.')) {
                    // Suggest keys for the given section
                    const [sectionName] = refContent.split('.');
                    const keys = sections.get(sectionName) || [];
                    return keys.map(key => new vscode.CompletionItem(key, vscode.CompletionItemKind.Field));
                } else {
                    // Suggest section names
                    return Array.from(sections.keys()).map(sectionName => {
                        const item = new vscode.CompletionItem(sectionName, vscode.CompletionItemKind.Module);
                        item.insertText = `${sectionName}.`;
                        item.command = { title: 'Trigger Suggest', command: 'editor.action.triggerSuggest' };
                        return item;
                    });
                }
            }

            // Section header completion
            if (linePrefix.endsWith('[')) {
                return sectionHeaders.map(header => {
                    return new vscode.CompletionItem(header, vscode.CompletionItemKind.Keyword);
                });
            }

            // Keyword completion
            return keywords.map(keyword => {
                return new vscode.CompletionItem(keyword, vscode.CompletionItemKind.Keyword);
            });
        }
    }, '@', '.', '[');

    context.subscriptions.push(provider);
}

module.exports = {
    activate
};
