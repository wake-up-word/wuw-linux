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
        try {
            console.log("Generating FEF files...");
            command = `${commands.run_gen_fef} \
            -G \
            -L ${fefAllListFile} \
            -i ${corpusDir} \
            -o ${fefTrainingFeDir} \
            -d ${fefTrainingOutDir} \
            -D
            `;

            result = childProcess.execSync(command, { stdio: ['pipe', 'pipe', 'ignore'] }).toString();
            console.log("Generating FEF files complete.");
            console.log(result)
        }
        catch (err) {
            console.log(command)
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