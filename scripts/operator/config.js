const path = require('path');

const root = path.resolve(__filename, '..', '..', '..');
const buildPath = path.resolve(root, 'build')
const temp = path.resolve(root, 'temp');

// Commands
const run_wuw = path.resolve(buildPath, 'run_wuw')
const run_gen_fef = path.resolve(buildPath, 'run_gen_fef')
const run_gen_hmm = path.resolve(buildPath, 'run_gen_hmm')
const train_hmm = path.resolve(buildPath, 'train_hmm')

const svmTrain = path.resolve(temp, 'libsvm', 'svm-train');
const svmPredict = path.resolve(temp, 'libsvm', 'svm-predict');

const corpusDir = path.resolve(temp, 'wuw-corpus-ii');
const dataJson = require(path.resolve(corpusDir, 'data.json'));

const trainingdir = path.resolve(temp, 'training');
const fefTrainingDir = path.resolve(trainingdir, 'fef');
const fefTrainingFeDir = path.resolve(fefTrainingDir, 'fe');
const fefTrainingOutDir = path.resolve(fefTrainingDir, 'out');
const fefOperatorListFile = path.resolve(fefTrainingDir, 'operator.list');
const fefOutOfVocabListFile = path.resolve(fefTrainingDir, 'oov.list');
const fefAllListFile = path.resolve(fefTrainingDir, 'all.list');

const hmmTrainingDir = path.resolve(trainingdir, 'hmm');
const hmmTrainingEmptyModel = path.resolve(hmmTrainingDir, 'empty_model.bhmm');
const hmmTrainingOperatorListFile = path.resolve(hmmTrainingDir, 'operator_training_input.txt');
const hmmTrainingOutOfVocabListFile = path.resolve(hmmTrainingDir, 'oov_training_input.txt'); // not actually used for training, scoring only

const svmTrainingDir = path.resolve(trainingdir, 'svm');
const svmTrainingInput = path.resolve(svmTrainingDir, 'svm_input.txt');
const svmTrainingCSVInputIV = path.resolve(svmTrainingDir, 'svm_input_IV.csv');
const svmTrainingCSVInputOOV = path.resolve(svmTrainingDir, 'svm_input_OOV.csv');
const svmModel = path.resolve(svmTrainingDir, 'operator_model.s');
const svmEvalOutputFile = path.resolve(svmTrainingDir, 'evaluate_results.txt');

const archiveDir = path.resolve(root, 'archive');
const archiveConfigDir = path.resolve(archiveDir, 'config');
const archiveSvmModel = path.resolve(archiveConfigDir, 'operator3.s');
// const archiveWindowsWUW = path.resolve(archiveDir, 'demo', 'run_e-wuw.exe');
const archiveWindowsWUW = path.resolve(`C:/Users/chris/git/wuw-windows/Win32/Debug/run_e-wuw.exe`);

const resultsOutputDir = path.resolve(temp, 'results');
module.exports = {
    root,
    buildPath,
    corpusDir,
    trainingdir,
    fefTrainingDir,
    fefTrainingFeDir,
    fefTrainingOutDir,
    fefOperatorListFile,
    fefOutOfVocabListFile,
    fefAllListFile,
    hmmTrainingDir,
    hmmTrainingEmptyModel,
    hmmTrainingOperatorListFile,
    hmmTrainingOutOfVocabListFile,
    svmTrainingDir,
    svmTrainingInput,
    svmTrainingCSVInputIV,
    svmTrainingCSVInputOOV,
    svmModel,
    svmEvalOutputFile,
    dataJson,
    resultsOutputDir,
    commands: {
        run_wuw,
        run_gen_fef,
        run_gen_hmm,
        train_hmm,
        svmTrain,
        svmPredict
    },
    archive: {
        archiveDir,
        archiveConfigDir,
        archiveSvmModel,
        archiveWindowsWUW
    }
}

if(process.argv[1] == __filename) {
    console.log(module.exports)
}