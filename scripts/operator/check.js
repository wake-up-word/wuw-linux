const config = require('./config');
const {
    fefAllListFile,
    fefOperatorListFile,
    fefOutOfVocabListFile,
    corpusDir,
    svmModel,
    hmmTrainingDir,
    dataJson,
    commands,
    archive,
    resultsOutputDir
} = config;

let expectedDetected = dataJson.entries
.filter((entry) => (entry.utteranceNumber == '001' || entry.utteranceNumber == '006') && !entry.transcription.toLowerCase().includes('operator'));
console.log({expectedDetected});