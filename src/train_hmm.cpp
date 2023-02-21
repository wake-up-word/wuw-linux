#include <stdio.h>
#include <errno.h>
#include <iostream>

using namespace std;

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <filesystem>

#include "back_end/hmm/hmm.h"

#define VALIDATE_INPUT(x, err)               \
    if (!(x))                                \
    {                                        \
        printf("Input error: " err "\n"); \
        return 1;                            \
    }

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
int numDim = 0;
int header_size = 1024;
int sample_size = sizeof(FLOAT32);
int numInterlaced = 3;
int featureNum = 0;

bool bBinaryModel = false;
bool bDoTraining = false;
bool bDoAlignment = false;
bool bShowInfScores = false;


//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------
void print_usage();
void print_version();
bool ProcessSingleFile(char *filename, char *szOutputDir, char *szOutputScoreName, int featureNum, HmmNetwork *hmm_net, HmmNetwork *hmm_rev, HmmNetwork *hmm_exp);

//-----------------------------------------------------------------------------
// Name: main()
// Desc: Entry point
//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
    char *szModelFile = NULL;
    char *szInputFile = NULL;
    char *szOutputFile = NULL;
    char *szOutputScoreName = NULL;
    char *szAlignFile = NULL;
    char *szOutputDir = NULL;
    bool bInputScript = false;
    int num_iter = 0;

    print_version();

    // Read command line arguments
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] != '-' || strlen(argv[i]) < 2)
        {
            print_usage();
            return 1;
        }

        switch (argv[i][1])
        {
        case 'M':
            bBinaryModel = true;
            // Fall through
        case 'm':
            szModelFile = argv[++i];
            printf("Model file: %s\n", szModelFile);
            break;
        case 'I':
            bInputScript = true;
            // Fall through
        case 'i':
            szInputFile = argv[++i];
            printf("Input file: %s\n", szInputFile);
            break;
        case 's':
            szOutputFile = argv[++i];
            break;
        case 'n':
            szOutputScoreName = argv[++i];
            break;
        case 't':
            num_iter = atoi(argv[++i]);
            bDoTraining = true;
            break;
        case 'f':
            sample_size = atoi(argv[++i]);
            if (sample_size > 8)
                sample_size /= 8;
            break;
        case 'h':
            header_size = atoi(argv[++i]);
            break;
        case 'o':
            // bSaveOutput = true;
            szOutputDir = argv[++i];
            break;
        case 'a':
            bDoAlignment = true;
            szAlignFile = argv[++i];
            break;
        case 'x':
            numInterlaced = atoi(argv[++i]);
            featureNum = atoi(argv[++i]); // 1 indexed i think
            break;
        case 'z':
            bShowInfScores = true;
            break;
        // case 'v':
        //     hmm_net.SetVFloorCoeff(_wtof(argv[++i]));
        //     break;
        default:
            printf("Unknown command: -%c\n", argv[i][1]);
            print_usage();
            return 1;
        }
    }

    VALIDATE_INPUT(szModelFile, "Please specify an input model file name");
    VALIDATE_INPUT(szInputFile, "Please specify an input file name");

    //for(int featureNum = 1; featureNum <= numInterlaced; featureNum++) {
        HmmNetwork *hmm_net, *hmm_rev, *hmm_exp;

        hmm_net = new HmmNetwork(0);
        hmm_rev = new HmmNetwork(0);
        hmm_exp = new HmmNetwork(0);

        HmmModel *model1, *model2, *model3;
        printf("Training feature: %d\n", featureNum);

        // Load model (2 copies of it)
        if (bBinaryModel)
        {
            model1 = HmmUtil::LoadBinaryHmm(szModelFile);
            model2 = HmmUtil::LoadBinaryHmm(szModelFile);
            model3 = HmmUtil::LoadBinaryHmm(szModelFile);
        }
        // else
        // {
        // 	model1 = HmmUtil::LoadTextHmm(szModelFile);
        // 	model2 = HmmUtil::LoadTextHmm(szModelFile);
        // 	model3 = HmmUtil::LoadTextHmm(szModelFile);
        // }

        // Verify correct loading
        if (!(model1 && model2 && model3)) {
            printf("Verify correct loading! \n");
            return 1;
        }

        model2->lengthMean = 0;
        model3->lengthMean = 0;
        model2->lengthVar = 0;
        model3->lengthVar = 0;

        // Reverse the second model
        HmmUtil::ReverseModel(model2);
        HmmUtil::ExpandModel(model3);

        // Add models to network
        hmm_net->AddModel(model1);
        hmm_rev->AddModel(model2);
        hmm_exp->AddModel(model3);

        numDim = model1->nDim;

        if (!bInputScript) // Process just a single file
        {
            ProcessSingleFile(szInputFile, szOutputDir, szOutputScoreName, featureNum, hmm_net, hmm_rev, hmm_exp);
        }
        else // Process multiple files
        {
            printf("Processing multiple files... \n");

            FILE *f_in = fopen(szInputFile, "rt");
            if (!f_in)
            {
                printf("Could not open input file: %s\n", szInputFile);
                return 1;
            }

            FILE *f_temp;

            char szDrive[_MAX_DRIVE];
            char szDir[_MAX_DIR];
            char szFullName[_MAX_PATH];
            char szFile[_MAX_FNAME];

            // _wsplitpath(szInputFile, szDrive, szDir, NULL, NULL);

            strcpy(szDir, std::filesystem::path(szInputFile).parent_path().string().c_str());
            printf("szDir: %s\n", szDir);

            while (fgets(szFile, _MAX_PATH, f_in))
            {
                printf("szFile: %s\n", szFile);
                
                char *p = strchr(szFile, '\n');
                // Ensuring NULL termination of the string
                if (p != NULL)
                    *p = L'\0';
            
                // const char *_szFile = std::filesystem::path(szDir).append(szFile).string().c_str();
                // strcpy(szFullName, _szFile);

                strcpy(szFullName, std::filesystem::path(szDir).append(szFile).string().c_str());
                
                printf("szFullName: %s\n", szFullName);

                if (!ProcessSingleFile(szFullName, szOutputDir, szOutputScoreName, featureNum, hmm_net, hmm_rev, hmm_exp))
                    goto CLEANUP;
            }

            fclose(f_in);
        }

        if (bDoTraining)
        {
            hmm_net->InitModel();          // Initialize model parameters
            hmm_net->TrainModel(num_iter); // Train model

            // hmm_rev.InitModel();          // Initialize model parameters
            // hmm_rev.TrainModel(num_iter); // Train model

            // hmm_exp.InitModel();          // Initialize model parameters
            // hmm_exp.TrainModel(num_iter); // Train model
        }
        else if (bDoAlignment)
        {
            hmm_net->AlignModel(szAlignFile);
            // TODO file name hmm_rev.AlignModel(szAlignFile);
            // TODO file name hmm_exp.AlignModel(szAlignFile);
        }

        if (szOutputFile) {
            printf("Saving trained model\n");
            
            char model_file_name[80];
            sprintf(model_file_name, "trained_model_%i.bhmm", featureNum);
            if(szOutputDir != NULL) {
                std::filesystem::path base = szOutputDir;
                std::filesystem::path modelFile = base.append(model_file_name);
                strcpy(model_file_name, modelFile.c_str());
            }

            HmmUtil::SaveBinaryHmm(model1, model_file_name);
        }

        // Cleanup for next feature

        delete hmm_net;
        delete hmm_rev;
        delete hmm_exp;
        delete model1;
        delete model2;
        delete model3;
    //}

CLEANUP:
    // TODO add code to destroy an hmm properly
    // delete model1;
    // delete model2;
    // delete model3;

    return 0;
}

//-----------------------------------------------------------------------------
// Name: ProcessSingleFile()
// Desc:
//-----------------------------------------------------------------------------
bool ProcessSingleFile(char *filename, char *scoreOutputDir, char*szOutputScoreName, int featureNum, HmmNetwork *hmm_net, HmmNetwork *hmm_rev, HmmNetwork *hmm_exp)
{

    bool exists = std::filesystem::exists(filename);
    if(!exists) {
        printf("Input file does not exist: %s\n", filename);
        return false;
    }

    FILE *f = fopen(filename, "rb");

    if (f == NULL)
    {
        printf("Could not open input file: %s\n", filename);
        return false;
    }

    if (bDoTraining || bDoAlignment) // Initialize training
    {
        printf(" Loading file: %s\n", filename);
        hmm_net->AddTrainSeq();
    }
    else // Initialize recognition
    {
        hmm_net->NewRecoSeq();
        hmm_rev->NewRecoSeq();
        hmm_exp->NewRecoSeq();

        if (bDoTraining)
            printf("%s,", filename);
    }

    // Skip file header
    if (header_size > 0)
        fseek(f, header_size, SEEK_SET);

    FLOAT32 *oRaw = new FLOAT32[numDim];
    FLOAT64 *o = new FLOAT64[numDim];

    // TODO loop through numInterlaced
    // See: WUW_IO::Write_FEF
    // first mfcc
    // second lpc
    // third e(nhanced)mfcc 
    
    // Pass features to HMM network
    while (true)
    {
        // Handle interlaced files
        if (featureNum > 1)
            fseek(f, (featureNum - 1) * numDim * sample_size, SEEK_CUR);

        // Read features
        if (!fread(oRaw, sample_size, numDim, f))
            break;
        
        // print the values of oRaw
        // printf("oRaw: ");
        // for(int i = 0; i < numDim; i++) {
        //     printf("%i: %f ", i, oRaw[i]);
        // }
        // printf("\n");

        // Handle interlaced files
        // jump to next segment
        if ((numInterlaced - featureNum) > 0)
            fseek(f, (numInterlaced - featureNum) * numDim * sample_size, SEEK_CUR);

        // 32 bit samples must be converted to 64 bit
        if (sample_size == 4)
        {
            for (int i = numDim - 1; i >= 0; i--)
                o[i] = (FLOAT64)oRaw[i];
        }

        // HACK skip extra features
        // fseek(f, sizeof(float) * 42 * 2, SEEK_CUR);

        // check for overflow
        for (int d = 0; d < numDim; d++)
        {
            if (abs(o[d]) > 1e4)
            {
                printf(" Is input data in correct format?\n");
                printf("Feature#: %d Dim: %d Val: %g\n", d, o[d]);
                goto CLEANUP;
            }
        }

        if (bDoTraining || bDoAlignment) // Training
        {
            hmm_net->AddTrainObs(o);
        }
        else // Recognition
        {
            hmm_net->AddRecoObs(o); // Pass features to regular HMM
            hmm_rev->AddRecoObs(o); // Pass features to reverse HMM
            hmm_exp->AddRecoObs(o); // Pass features to expanded HMM
        }
    }

    // If we are doing only recognition, print out results
    if (!bDoTraining && !bDoAlignment)
    {
        FLOAT64 s1 = hmm_net->GetRecoScore(),
                s2 = hmm_rev->GetRecoScore(),
                s3 = hmm_exp->GetRecoScore();
        // s4 = hmm_net.GetHistScore();

        if (s1 < -1000 || s2 < -1000 || s3 < -1000)
        {
            if (bShowInfScores)
                printf("%s,%f,%f,%f\n", filename, 0, 0, 0);
        }
        else {
            printf("%s,%f,%f,%f\n", filename, s1, s2, s3);

            char score_file_name[80];
            if(szOutputScoreName == NULL) {
                sprintf(score_file_name, "scores_model_%i.txt", featureNum);
            } else {
                sprintf(score_file_name, "scores_model_%s_%i.txt", szOutputScoreName, featureNum);
            }


            if(scoreOutputDir != NULL) {
                std::filesystem::path base = scoreOutputDir;
                std::filesystem::path modelFile = base.append(score_file_name);
                strcpy(score_file_name, modelFile.c_str());
            }

            FILE * fp;
            fp = fopen (score_file_name, "a");
            fprintf(fp,"%s,%f,%f,%f\n", filename, s1, s2, s3);
            fclose(fp);
        }
    }

CLEANUP:
    fclose(f);
    return true;
}

//-----------------------------------------------------------------------------
// Name: print_usage()
// Desc:
//-----------------------------------------------------------------------------
void print_usage()
{
    printf("Hmm parameters:\n\n"
           "-i filename\t input file\n"
           "-I filename\t input script (should contain one or more filenames separated by newlines)"
           "-m filename\t input model file (text format)\n"
           "-M filename\t input model file (binary format)\n"
           "-s filename\t output model file (saved as binary format)\n"
           "-t num_iter\t perform Baum-Welch training with specified number of iterations\n");
}

void print_version()
{
    printf("*** Using HMM built on %S\n", __TIMESTAMP__);
}