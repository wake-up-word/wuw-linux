const fs = require('fs');
const path = require('path');
console.log("Building operator model..");

const root = path.resolve(__filename, '..', '..');
const corpusDir = path.resolve(root, 'temp', 'wuw-corpus-ii');
const dataJson = require(path.resolve(corpusDir, 'data.json'));

console.log(dataJson);