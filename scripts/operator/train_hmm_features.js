const childProcess = require('child_process');
const fs = require('fs');
const path = require('path');
const config = require('./config');
const {
    hmmTrainingDir,
    hmmTrainingOperatorListFile,
    hmmTrainingEmptyModel,
    commands
} = config;

module.exports = {
    run: () => {
        let command;
        let result;
        try {

            for (let featureNum = 1; featureNum <= 3; featureNum++) {
                console.log(`Training hmm feature ${featureNum}...`)
                command = `${commands.train_hmm} \
                -I ${hmmTrainingOperatorListFile} \
                -M ${hmmTrainingEmptyModel} \
                -o ${hmmTrainingDir}\
                -s true \
                -t 10 \
                -x 3 ${featureNum} \
                `;
                console.log('Running:');
                console.log(command);
                result = childProcess.execSync(command, { stdio: ['pipe', 'pipe', 'ignore'] }).toString();
                console.log(result)
                console.log(`Training hmm feature ${featureNum} complete.`)
            }

            fs.copyFileSync(path.resolve(hmmTrainingDir, 'trained_model_1.bhmm'), path.resolve(hmmTrainingDir, 'trained_model_1.m'))
            fs.copyFileSync(path.resolve(hmmTrainingDir, 'trained_model_2.bhmm'), path.resolve(hmmTrainingDir, 'trained_model_2.l'))
            fs.copyFileSync(path.resolve(hmmTrainingDir, 'trained_model_3.bhmm'), path.resolve(hmmTrainingDir, 'trained_model_3.e'))
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