console.log("Building operator model...");

const build_list_files = require('./operator/create_list_files');
build_list_files.run();

const build_gen_fef_files = require('./operator/gen_fef_files');
build_gen_fef_files.run();

const create_hmm_training_list_files = require('./operator/create_hmm_training_list_files');
create_hmm_training_list_files.run();

const gen_empty_hmm = require('./operator/gen_empty_hmm');
gen_empty_hmm.run();

const train_hmm_features = require('./operator/train_hmm_features');
train_hmm_features.run();

const score_hmm_features = require('./operator/score_hmm_features');
score_hmm_features.run();

const build_svm_training_input = require('./operator/build_svm_training_input');
build_svm_training_input.run();

const train_svm = require('./operator/train_svm');
train_svm.run();

console.log("Building operator model complete.");
