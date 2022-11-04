/******************************  Include Files  ******************************/

#include <vector>
#include <algorithm>
#include <iterator>
#include <iostream>

using namespace std;

#include "wuw_config.h"
#include "front_end/front_end.h"
#include "back_end/back_end.h"
#include "process_in_samples.h"
#include "audio_stream.h"
#include "back_end/hmm/hmm_util.cpp"

//-----------------------------------------------------------------------------
// Name: wmain
// Desc: program entry point
//-----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    INT16 nDimensions = std::stoi(argv[1]);
    INT16 nRegularStates = std::stoi(argv[2]);
    INT16 nMix = std::stoi(argv[3]);
    INT16 nSkip = std::stoi(argv[4]);
    INT16 nSilBeg = std::stoi(argv[5]);
    INT16 nSilEnd = std::stoi(argv[6]);
    char *outfile = argv[7];

    HmmModel *m = HmmUtil::CreateHmmPrototype(nDimensions, nMix, nRegularStates, nSilBeg, nSilEnd, nSkip);
    HmmUtil::SaveBinaryHmm(m, outfile);
    return (0);
}
