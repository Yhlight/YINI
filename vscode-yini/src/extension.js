const vscode = require('vscode');

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


function activate(context) {
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
