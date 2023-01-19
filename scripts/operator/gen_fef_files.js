const childProcess = require('child_process');
const config = require('./config');
const {
    corpusDir,
    fefTrainingFeDir,
    fefTrainingOutDir,
    fefAllListFile,
    commands
} = config;

module.exports = {
    run: () => {
        let command;
        let result;

        console.log("Generating FEF files...");
        command = `${commands.run_gen_fef} \
        -L ${fefAllListFile} \
        -i ${corpusDir} \
        -o ${fefTrainingFeDir} \
        -d ${fefTrainingOutDir} \
        -D \
        -v
        `;

        result = childProcess.execSync(command, { stdio: ['pipe', 'pipe', 'ignore'] }).toString();
        console.log("Generating FEF files complete.");
        console.log(result)
    }
}

if(process.argv[1] == __filename) {
    module.exports.run();
}