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
    const yiniDoc = {
        sections: new Map(),
        macros: new Map(),
        schemas: new Map()
    };
    let currentSection = null;
    let inSchema = false;

    const sectionRegex = /^\s*\[\s*([^\]]+?)\s*\]/;
    const keyRegex = /^\s*(\w+)\s*=/;
    const macroRegex = /^\s*(\w+)\s*=/;

    for (let i = 0; i < document.lineCount; i++) {
        const line = document.lineAt(i);
        const sectionMatch = line.text.match(sectionRegex);

        if (sectionMatch) {
            currentSection = sectionMatch[1].split(':')[0].trim();
            inSchema = false;
            if (currentSection === '#schema') {
                inSchema = true;
                currentSection = null; // Reset section context
            } else if (currentSection === '#define') {
                currentSection = '#define';
            }
            else {
                if (!yiniDoc.sections.has(currentSection)) {
                    yiniDoc.sections.set(currentSection, {
                        keys: new Map(),
                        location: new vscode.Location(document.uri, line.range)
                    });
                }
            }
        } else {
            if (inSchema) {
                // This is a simplified schema parser for hover info.
                // It doesn't validate, just extracts the rule string.
                const keyMatch = line.text.match(keyRegex);
                if (keyMatch) {
                    const key = keyMatch[1];
                    const rule = line.text.substring(line.text.indexOf('=') + 1).trim();
                    if (!yiniDoc.schemas.has(currentSection)) {
                         yiniDoc.schemas.set(currentSection, new Map());
                    }
                    yiniDoc.schemas.get(currentSection).set(key, rule);
                }
            }
            else if (currentSection === '#define') {
                const macroMatch = line.text.match(macroRegex);
                if (macroMatch) {
                    yiniDoc.macros.set(macroMatch[1], new vscode.Location(document.uri, line.range));
                }
            }
            else if (currentSection) {
                const keyMatch = line.text.match(keyRegex);
                if (keyMatch && yiniDoc.sections.has(currentSection)) {
                    const key = keyMatch[1];
                    yiniDoc.sections.get(currentSection).keys.set(key, new vscode.Location(document.uri, line.range));
                }
            }
        }
    }
    return yiniDoc;
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

    // --- Hover Provider ---
    const hoverProvider = vscode.languages.registerHoverProvider('yini', {
        provideHover(document, position, token) {
            const yiniDoc = parseYiniDocument(document);
            const range = document.getWordRangeAtPosition(position);
            const word = document.getText(range);

            // Find which section the word is in
            let currentSection = null;
            for (let i = position.line; i >= 0; i--) {
                const line = document.lineAt(i);
                const sectionMatch = line.text.match(/^\s*\[\s*([^\]]+?)\s*\]/);
                if (sectionMatch) {
                    currentSection = sectionMatch[1].split(':')[0].trim();
                    break;
                }
            }

            if (currentSection && yiniDoc.schemas.has(null)) { // Simple schema lookup
                const schema = yiniDoc.schemas.get(null);
                if (schema.has(word)) {
                    const rule = schema.get(word);
                    const markdownString = new vscode.MarkdownString();
                    markdownString.appendCodeblock(`(schema) ${word} = ${rule}`, 'yini');
                    return new vscode.Hover(markdownString, range);
                }
            }

            return null;
        }
    });
     context.subscriptions.push(hoverProvider);

    // --- Definition Provider ---
    const definitionProvider = vscode.languages.registerDefinitionProvider('yini', {
        provideDefinition(document, position, token) {
            const yiniDoc = parseYiniDocument(document);
            const range = document.getWordRangeAtPosition(position);
            const word = document.getText(range);
            const line = document.lineAt(position.line).text;

            // Go to macro definition
            if (line.includes(`@${word}`)) {
                return yiniDoc.macros.get(word);
            }

            // Go to section definition (from cross-section reference)
            const refMatch = line.match(/@\{([\w\.]*)\}/);
            if (refMatch) {
                const sectionName = refMatch[1].split('.')[0];
                if (yiniDoc.sections.has(sectionName)) {
                    return yiniDoc.sections.get(sectionName).location;
                }
            }

            return null;
        }
    });
    context.subscriptions.push(definitionProvider);
}

module.exports = {
    activate
};
