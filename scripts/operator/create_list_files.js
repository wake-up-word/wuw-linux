const fs = require('fs');
const path = require('path');

const config = require('./config');
const {
    fefTrainingDir,
    fefOperatorListFile,
    fefOutOfVocabListFile,
    fefAllListFile,
    fefTrainingOutDir,
    hmmTrainingDir,
    hmmTrainingOperatorListFile,
    hmmTrainingOutOfVocabListFile,
    dataJson
} = config;

module.exports = {
    run: () => {
        const trainingFiles = dataJson.entries;
        const operatorFiles = trainingFiles
            .filter(file => file.utteranceNumber == '001') // Only "Operator" utterances
            .splice(0, 250); // Limit file count

        const outOfVocabFiles = trainingFiles
            .filter(file => file.utteranceNumber != '001') // Omit "Operator" utterances
            .filter(file => file.utteranceNumber != '006') // Omit "Operator in sentence" utterances
            .splice(0, 1000); // Limit file count

        fs.mkdirSync(fefTrainingDir, { recursive: true });
        fs.writeFileSync(fefOperatorListFile, operatorFiles.map(file => file.filePath).join('\n'));
        fs.writeFileSync(fefOutOfVocabListFile, outOfVocabFiles.map(file => file.filePath).join('\n'));
        fs.writeFileSync(fefAllListFile, trainingFiles.map(file => file.filePath).join('\n'));
        fs.mkdirSync(hmmTrainingDir, { recursive: true });
        const hmmTrainingOperatorFiles = operatorFiles.map(f => path.resolve(fefTrainingOutDir, `${f.fileName.replace('.', '_')}_00000.fef`));
        fs.writeFileSync(hmmTrainingOperatorListFile, hmmTrainingOperatorFiles.join('\n'));
        const hmmTrainingOutOfVocabFiles = outOfVocabFiles.map(f => path.resolve(fefTrainingOutDir, `${f.fileName.replace('.', '_')}_00000.fef`));
        fs.writeFileSync(hmmTrainingOutOfVocabListFile, hmmTrainingOutOfVocabFiles.join('\n'));
    }
}

if (process.argv[1] == __filename) {
    module.exports.run();
}