const childProcess = require('child_process');
const path = require('path');
const fs = require('fs');
const config = require('./config');
const {
    commands,
    hmmTrainingDir,
    svmTrainingInput,
    svmModel,
} = config;

module.exports = {
    run: () => {
        let command;
        let result;
        try {
            console.log(`Training svm model...`)

            command = `${commands.svmTrain} \
            ${svmTrainingInput} \
            ${svmModel}
            `;
            console.log('Running:');
            console.log(command);
            result = childProcess.execSync(command, { stdio: ['pipe', 'pipe', 'ignore'] }).toString();
            console.log(result)

            console.log(`Training svm model complete.`)

        }
        catch (err) {
            console.log(err)
            console.log('Error Output: ')
            console.log(err.output.toString('utf8'))
        }
    }
}

if (process.argv[1] == __filename) {
    module.exports.run();
}