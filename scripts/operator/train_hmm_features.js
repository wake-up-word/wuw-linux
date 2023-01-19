const childProcess = require('child_process');
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