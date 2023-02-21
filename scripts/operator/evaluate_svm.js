const childProcess = require('child_process');
const path = require('path');
const fs = require('fs');
const config = require('./config');
const {
    commands,
    hmmTrainingDir,
    svmTrainingInput,
    svmModel,
    svmEvalOutputFile
} = config;

module.exports = {
    run: () => {
        let command;
        let result;
        try {
            console.log(`Evaluating svm model...`)

            command = `${commands.svmPredict} \
            ${svmTrainingInput} \
            ${svmModel} \
            ${svmEvalOutputFile}
            `;
            console.log('Running:');
            console.log(command);
            result = childProcess.execSync(command, { stdio: ['pipe', 'pipe', 'ignore'] }).toString();
            console.log(result)

            console.log(`Evaluating svm model complete.`)

        }
        catch (err) {
            console.log(err)
            console.log('Error Output: ')
            console.log(err.output.toString('utf8'))
            throw err;
        }
    }
}

if (process.argv[1] == __filename) {
    module.exports.run();
}