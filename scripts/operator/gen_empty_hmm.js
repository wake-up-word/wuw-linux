const childProcess = require('child_process');
const config = require('./config');
const {
    hmmTrainingEmptyModel,
    commands
} = config;

module.exports = {
    run: () => {
        let command;
        let result;

        console.log("Generating empty hmm model...");
        command = `${commands.run_gen_hmm} \
        39 30 6 0 1 1 \
        ${hmmTrainingEmptyModel}
        `;

        result = childProcess.execSync(command, { stdio: ['pipe', 'pipe', 'ignore'] }).toString();
        console.log("Generating empty hmm model complete.");
        console.log(result)
    }
}

if(process.argv[1] == __filename) {
    module.exports.run();
}