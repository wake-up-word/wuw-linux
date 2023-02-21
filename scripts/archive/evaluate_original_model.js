const fs = require('fs');
const path = require('path');
const childProcess = require('child_process');
const util = require('node:util');
const exec = util.promisify(require('node:child_process').exec);
const config = require('../operator/config');
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

module.exports = {
    run: async () => {
        let command;
        let result;

        console.log("Evaluating training data...");

        fs.mkdirSync(resultsOutputDir, { recursive: true });

        const data = fs.readFileSync(fefAllListFile).toString().trim().split('\n')
            // .splice(0, 1000); // limit count

            const model1 = path.resolve(archive.archiveConfigDir, 'operator3.m');
            const model2 = path.resolve(archive.archiveConfigDir, 'operator3.l');
            const model3 = path.resolve(archive.archiveConfigDir, 'operator3.e');

        let truePositives = 0;
        let falsePositives = 0;
        let trueNegatives = 0;
        let falseNegatives = 0;
        const falsePositiveEntries = [];
        let expectedDetected = dataJson.entries
        .filter((entry) => entry.utteranceNumber == '001' || entry.utteranceNumber == '006').length;
        const evaluating = data.length;
        console.log(`Evaluating ${evaluating} utterances...`)

        const threshold = 1.1;
        
        const results = await Promise.all(data.map(async (recording) => {
            const file = path.resolve(corpusDir, recording)
            command = `${commands.run_wuw} \
                -i ${file} \
                -d ./out \
                -o ./fe \
                -S ${archive.archiveSvmModel} ${threshold} 0 \
                -M ${model1} \
                   ${model2} \
                   ${model3} \
                   `;

            result = await exec(command, { stdio: ['pipe', 'pipe', 'ignore'] });
            console.log(command)
            console.log(result.stdout)
            const dataJsonEntry = dataJson.entries.find((entry) => entry.filePath.trim() == recording.trim());
            if (result.stdout.includes('***WUW DETECTED***')) {
                if(dataJsonEntry.utteranceNumber == '001' || dataJsonEntry.utteranceNumber == '006') {
                    truePositives++;
                } else {
                    falsePositives++;
                    falsePositiveEntries.push(dataJsonEntry);
                }
                return `${file} 1 DETECTED    `;
            } else {
                if(dataJsonEntry.utteranceNumber == '001' || dataJsonEntry.utteranceNumber == '006') {
                    falseNegatives++;
                } else {
                    trueNegatives++;
                }
                return `${file} 0 NOT_DETECTED`;
            }
        }));

        const resultsFile = path.resolve(resultsOutputDir, 'legacy_ results.txt');
        fs.writeFileSync(resultsFile, results.join('\n'), { encoding: 'utf8', flag: 'w' })
        console.log({falsePositiveEntries})
        console.log(`Evaluating training data complete with threshold of ${threshold}`);
        console.log(`Detected ${truePositives}/${expectedDetected} (${truePositives/expectedDetected*100}%)`)
        console.log(`False positives: ${falsePositives}`)
        console.log(`False negatives: ${falseNegatives}`)
    }
}

if (process.argv[1] == __filename) {
    module.exports.run();
}