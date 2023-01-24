const path = require('path');
const fs = require('fs');
const config = require('./config');
const {
    hmmTrainingDir,
    svmTrainingDir,
    svmTrainingInput,
    svmTrainingCSVInputIV,
    svmTrainingCSVInputOOV,
} = config;

module.exports = {
    run: () => {
        try {
            console.log(`Building svm training input...`)
            fs.mkdirSync(svmTrainingDir, { recursive: true });

            const scores_model_iv = {};
            for (let featureNum = 1; featureNum <= 3; featureNum++) {
                fs.readFileSync(path.resolve(hmmTrainingDir, `scores_model_iv_${featureNum}.txt`)).toString()
                    .trim()
                    .split('\n')
                    .forEach(line => {
                        const entry = line.split(',');
                        const fefFile = entry[0];
                        scores_model_iv[fefFile] = scores_model_iv[fefFile] || [];
                        scores_model_iv[fefFile] = [...scores_model_iv[fefFile], ...entry.slice(1)];
                    });
            }

            const scores_model_oov = {};
            for (let featureNum = 1; featureNum <= 3; featureNum++) {
                fs.readFileSync(path.resolve(hmmTrainingDir, `scores_model_oov_${featureNum}.txt`)).toString()
                    .trim()
                    .split('\n')
                    .forEach(line => {
                        const entry = line.split(',');
                        const fefFile = entry[0];
                        scores_model_oov[fefFile] = scores_model_oov[fefFile] || [];
                        scores_model_oov[fefFile] = scores_model_oov[fefFile].concat(entry.slice(1));
                    });
            }

            // console.log(scores_model_iv)
            // console.log(scores_model_oov)

            const output = [
                ...Object.values(scores_model_iv)
                    .map(entry => (
                        ['+1', ...(entry.map((v, i) => `${i + 1}:${v}`))].join(' ')
                    )),
                ...Object.values(scores_model_oov)
                    .map(entry => (
                        ['-1', ...(entry.map((v, i) => `${i + 1}:${v}`))].join(' ')
                    )),
            ]
            fs.writeFileSync(svmTrainingInput, output.join('\n'));

            // console.log(output)

            const csv_output_iv = Object.values(scores_model_iv)
                .map(entry => (
                    Object.values(entry).join(',')
                ));
            fs.writeFileSync(svmTrainingCSVInputIV, csv_output_iv.join('\n'));

            const csv_output_oov = Object.values(scores_model_oov)
            .map(entry => (
                Object.values(entry).join(',')
            ));
            fs.writeFileSync(svmTrainingCSVInputOOV, csv_output_oov.join('\n'));

            console.log(`Building svm training input complete.`)

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