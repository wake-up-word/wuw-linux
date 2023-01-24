const fs = require('fs');
const path = require('path');
const childProcess = require('child_process');
const config = require('./config');
const {
    fefAllListFile,
    fefOperatorListFile,
    corpusDir,
    svmModel,
    hmmTrainingDir,
    commands
} = config;

module.exports = {
    run: () => {
        let command;
        let result;

        console.log("Evaluating training data...");

        const data = fs.readFileSync(fefOperatorListFile).toString().trim().split('\n')

        const model1 = path.resolve(hmmTrainingDir, 'trained_model_1.bhmm');
        const model2 = path.resolve(hmmTrainingDir, 'trained_model_2.bhmm');
        const model3 = path.resolve(hmmTrainingDir, 'trained_model_3.bhmm');

        let content = '';

        for (const recording of data) {
            const file = path.resolve(corpusDir, recording)
            command = `${commands.run_wuw} \
                -i ${file} \
                -d ./out \
                -o ./fe \
                -S ${svmModel} 0.9 0 \
                -M ${model1} \
                   ${model2} \
                   ${model3} \
            `;
            console.log(command)
            console.log()


            result = childProcess.execSync(command, { stdio: ['pipe', 'pipe', 'ignore'] }).toString();
            console.log(result)
            if (result.includes('***WUW DETECTED***')) {
                content += `${file} DETECTED     \n`;
            } else {
                content += `${file} NOT_DETECTED \n`;
            }
        }

        fs.writeFileSync('./results.txt', content, { encoding: 'utf8', flag: 'w' })
        console.log("Evaluating training data complete.");
    }
}

if (process.argv[1] == __filename) {
    module.exports.run();
}