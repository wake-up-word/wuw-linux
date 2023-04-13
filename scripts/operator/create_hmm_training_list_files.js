const childProcess = require('child_process');
const path = require('path');
const fs = require('fs');
const config = require('./config');

module.exports = {
    run: () => {
        let command;
        let result;
        try {
            console.log("Generating training list files...");

            const trainingFiles = config.dataJson.entries;

            const oovList = []
            const ivList = []
            const labledButNotTriggered = []
            const labledDelayedTriggered = []

            const durationScale = 0.8;

            for (const entry of trainingFiles) {
                const vadFile = path.resolve(config.fefTrainingOutDir, `WUWII${entry.sessionNumber}_${entry.utteranceNumber}_ulaw_vad.dat`);
                if (!fs.existsSync(vadFile)) {
                    console.log(`${vadFile} does not exist!`);
                }
                const vadTriggers = fs.readFileSync(vadFile).toString().trim().split('\n');
                const labeledStart = entry.startTime * 1000;
                const labeledEnd = entry.endTime * 1000;
                const labeledDuration = labeledEnd - labeledStart;
                // console.log({ vadFile })
                // console.log({ labeledStart, labeledEnd })

                let found = false;
                for (const trigger of vadTriggers) {
                    if (!trigger) continue; // Empty lines

                    const triggerNumber = trigger.split(' ')[0].substring(1, 7);

                    const start = Number(trigger.split(' ')[1].split('\t')[0]);
                    const end = Number(trigger.split(' ')[1].split('\t')[1]);
                    const duration = end - start;
                    const iv =
                        !((start >= labeledEnd) || (end <= labeledStart)) &&
                        (duration >= labeledDuration * durationScale)

                    // (start >= labeledStart && start < labeledEnd) ||
                    // (end > labeledStart && end <= labeledEnd) ||
                    // (start >= labeledStart && end >= labeledEnd)

                    // console.log({ triggerNumber, start, end, iv })

                    if (entry.utteranceNumber == '001' || entry.utteranceNumber == '006') {
                        if (
                            !((start >= labeledEnd) || (end <= labeledStart)) &&
                            (duration < labeledDuration * durationScale)
                        ) {
                            console.log(`Duration too small: ${entry.fileName} ${start} ${duration}`);
                        }

                        if (iv) {
                            if (start >= (entry.startTime * 1000) + 50) {
                                console.log(`Delayed trigger: ${entry.fileName} ${start}`)
                                found = true;
                                labledDelayedTriggered.push({ entry, vadTriggers })
                                continue;
                            }
                            found = true;
                            ivList.push(path.resolve(config.fefTrainingOutDir, `WUWII${entry.sessionNumber}_${entry.utteranceNumber}_ulaw_${triggerNumber.substring(1)}.fef`));
                            continue;
                        }
                    }
                    oovList.push(path.resolve(config.fefTrainingOutDir, `WUWII${entry.sessionNumber}_${entry.utteranceNumber}_ulaw_${triggerNumber.substring(1)}.fef`));
                }
                if (!found && (entry.utteranceNumber == '001' || entry.utteranceNumber == '006')) {
                    labledButNotTriggered.push({ entry, vadTriggers })
                }

            }
            console.log("labledButNotTriggered")
            console.log(labledButNotTriggered)

            fs.mkdirSync(config.hmmTrainingDir, { recursive: true });
            fs.writeFileSync(config.hmmTrainingOperatorListFile, ivList.join('\n'));
            fs.writeFileSync(config.hmmTrainingOutOfVocabListFile, oovList.join('\n'));

            console.log("Generating training list files complete.");
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