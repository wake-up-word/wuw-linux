const fs = require('fs');
const path = require('path');
const childProcess = require('child_process');

console.log('Generating FEF for training data for "Operator"...');



const result = childProcess.execSync(`
../build/run_gen_fef \
-G \
-L ${inputModel} \
-i ${outputFile} \
-o ${num_iter} \
-d ${numInterlaced} \
-D \
`, { stdio: ['pipe', 'pipe', 'ignore'] }).toString();
console.log(result);

"-G", 
                "-L", "./lists/WUWII_Operator.list",
                "-i", "./test/WUWII_Corpus/calls",
                "-o", "./build/fe",
                "-d", "./build/out",
                "-D"