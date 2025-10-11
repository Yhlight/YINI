const vscode = require('vscode');

const keywords = [
    'Dyna', 'color', 'coord', 'path', 'list', 'array', 'true', 'false'
];

const sectionHeaders = [
    '[#define]', '[#include]', '[#schema]'
];

function activate(context) {
    const provider = vscode.languages.registerCompletionItemProvider('yini', {
        provideCompletionItems(document, position) {
            const linePrefix = document.lineAt(position).text.substr(0, position.character);

            if (linePrefix.endsWith('[')) {
                return sectionHeaders.map(header => {
                    const item = new vscode.CompletionItem(header, vscode.CompletionItemKind.Keyword);
                    item.insertText = header.substring(1); // Remove leading '['
                    return item;
                });
            }

            const keywordCompletions = keywords.map(keyword => {
                return new vscode.CompletionItem(keyword, vscode.CompletionItemKind.Keyword);
            });

            return keywordCompletions;
        }
    });

    context.subscriptions.push(provider);
}

module.exports = {
    activate
};
