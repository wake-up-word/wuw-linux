const path = require('path');
const fs = require('fs');
const childProcess = require('child_process');
const config = require('../operator/config');
const {
    hmmTrainingDir,
    hmmTrainingOperatorListFile,
    hmmTrainingOutOfVocabListFile,
    archive,
    commands
} = config;

module.exports = {
    run: () => {
        let command;
        let result;
        let scoreName = 'iv';
        try {
            console.log(`Scoring in vocab...`)
            for (let featureNum = 1; featureNum <= 3; featureNum++) {
                // delete old files if exists
                try {
                    fs.unlinkSync(path.resolve(hmmTrainingDir, `scores_model_${scoreName}_${featureNum}.txt`));
                } catch (err) { }

                console.log(`Scoring hmm feature ${featureNum}...`)
                const trainedModel = path.resolve(archive.archiveConfigDir, `trained_model_${featureNum}.bhmm`)
                command = `${commands.train_hmm} \
                -I ${hmmTrainingOperatorListFile} \
                -M ${trainedModel} \
                -o ${hmmTrainingDir} \
                -n ${scoreName} \
                -x 3 ${featureNum} \
                `;
                console.log('Running:');
                console.log(command);
                result = childProcess.execSync(command, { stdio: ['pipe', 'pipe', 'ignore'] }).toString();
                console.log(result)
                console.log(`Scoring hmm feature ${featureNum} complete.`)
            }
            console.log(`Scoring in vocab complete.`)

            console.log(`Scoring out of vocab...`)
            scoreName = 'oov';
            for (let featureNum = 1; featureNum <= 3; featureNum++) {
                // delete old files if exists
                try {
                    fs.unlinkSync(path.resolve(hmmTrainingDir, `scores_model_${scoreName}_${featureNum}.txt`));
                } catch (err) { }
                console.log(`Scoring hmm feature ${featureNum}...`)
                const trainedModel = path.resolve(hmmTrainingDir, `trained_model_${featureNum}.bhmm`)
                command = `${commands.train_hmm} \
                -I ${hmmTrainingOutOfVocabListFile} \
                -M ${trainedModel} \
                -o ${hmmTrainingDir} \
                -n ${scoreName} \
                -x 3 ${featureNum} \
                `;
                console.log('Running:');
                console.log(command);
                result = childProcess.execSync(command, { stdio: ['pipe', 'pipe', 'ignore'] }).toString();
                console.log(result)
                console.log(`Scoring hmm feature ${featureNum} complete.`)
            }
            console.log(`Scoring out of vocab complete.`)

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