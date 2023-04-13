const fs = require('fs');
const path = require('path');
const childProcess = require('child_process');
const util = require('node:util');
const exec = util.promisify(require('node:child_process').exec);
const config = require('./config');
const {
    fefAllListFile,
    fefOperatorListFile,
    fefOutOfVocabListFile,
    corpusDir,
    svmModel,
    hmmTrainingDir,
    dataJson,
    resultsOutputDir,
    commands
} = config;

module.exports = {
    run: async () => {
        let command;
        let result;

        console.log("Evaluating training data...");

        fs.mkdirSync(resultsOutputDir, { recursive: true });

        const data = fs.readFileSync(fefAllListFile).toString().trim().split('\n')
        // .splice(0, 1000); // limit count

        const model1 = path.resolve(hmmTrainingDir, 'trained_model_1.m');
        const model2 = path.resolve(hmmTrainingDir, 'trained_model_2.l');
        const model3 = path.resolve(hmmTrainingDir, 'trained_model_3.e');


        let truePositives = 0;
        let falsePositives = 0;
        let trueNegatives = 0;
        let falseNegatives = 0;
        const falsePositiveEntries = [];
        let expectedDetected = dataJson.entries
        .filter((entry) => 
        (entry.utteranceNumber == '001' || entry.utteranceNumber == '006') && entry.startTime !== 0 && entry.endTime !== 0).length;
        // const expectedDetected = fs.readFileSync(config.hmmTrainingOperatorListFile).toString().trim().split('\n').length;
        const evaluating = data.length;
        console.log(`Evaluating ${evaluating} utterances...`)

        const threshold = 0.9;

        const results = await Promise.all(data.map(async (recording) => {
            const file = path.resolve(corpusDir, recording)
            command = `${commands.run_wuw} \
                -i ${file} \
                -d ./out \
                -o ./fe \
                -S ${svmModel} ${threshold} 0 \
                -M ${model1} \
                   ${model2} \
                   ${model3} \
                   `;

            result = await exec(command, { stdio: ['pipe', 'pipe', 'ignore'] });
            // console.log(command)
            // console.log(result.stdout)
            const dataJsonEntry = dataJson.entries.find((entry) => entry.filePath.trim() == recording.trim());
            if (result.stdout.includes('***WUW DETECTED***')) {
                if ((dataJsonEntry.utteranceNumber == '001' || dataJsonEntry.utteranceNumber == '006') &&
                    dataJsonEntry.startTime !== 0 && dataJsonEntry.endTime !== 0) {
                    truePositives++;
                } else {
                    falsePositives++;
                    console.log('False Positive!')
                    console.log(command)
                    console.log(result.stdout)
                    falsePositiveEntries.push(dataJsonEntry);
                }
                return `${file} 1 DETECTED    `;
            } else {
                if ((dataJsonEntry.utteranceNumber == '001' || dataJsonEntry.utteranceNumber == '006') &&
                    dataJsonEntry.startTime !== 0 && dataJsonEntry.endTime !== 0) {
                    console.log('False Negative!')
                    console.log(command)
                    console.log(result.stdout)
                    falseNegatives++;
                } else {
                    trueNegatives++;
                }
                return `${file} 0 NOT_DETECTED`;
            }
        }));

        const resultsFile = path.resolve(resultsOutputDir, 'operator_results.txt');
        fs.writeFileSync(resultsFile, results.join('\n'), { encoding: 'utf8', flag: 'w' })
        // console.log({falsePositiveEntries})
        console.log(`Evaluating training data complete with threshold of ${threshold}`);
        console.log(`Detected ${truePositives}/${expectedDetected} (${truePositives / expectedDetected * 100}%)`)
        console.log(`False positives: ${falsePositives}`)
        console.log(`False negatives: ${falseNegatives}`)
        console.log(`Accuracy: ${((truePositives+trueNegatives) / evaluating) * 100}%`)
    }
}

if (process.argv[1] == __filename) {
    module.exports.run();
}