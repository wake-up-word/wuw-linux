#ifndef HMM_H
#define HMM_H

#include <vector>
#include <list>
#include <math.h>
#include "common/defines.h"

using namespace std;

typedef unsigned char BYTE;

#define PI 3.14159265358979323846
const FLOAT64 LOG_ZERO  = -1e10;   // ~log(0)
const FLOAT64 LOG_SMALL = -0.5e10;   // log values < LOG_SMALL are set to LOG_ZERO
const FLOAT64 MIN_EXP = -704.0;
const FLOAT64 MIN_LOG = 1e-306;

//-----------------------------------------------------------------------------
// Structs
//-----------------------------------------------------------------------------
struct HmmMix
{
	FLOAT64*  means;   // Means
	FLOAT64*  vars;    // Diagonal covariance matrix
	FLOAT64   gconst;	// Precomputed constant for gaussian fn
	FLOAT64   logGconst;	// Precomputed constant for gaussian fn

	FLOAT64   det;      // Determinant
	FLOAT64   logDet;   // Log of the determinant

};

struct HmmState
{
	INT16     nMix;    // Number of mixture models
	FLOAT64*  weights;   // Weights
	HmmMix*   mixtures;
};

struct HmmModel
{
	INT16       nStates;   // Number of states
	INT16       nDim;
	FLOAT64**   transProb; // Transition probability matrix
	HmmState*   states;
	FLOAT64     lengthMean;
	FLOAT64     lengthVar;
};


struct HmmObsV
{
	FLOAT64 score;
	INT16 prev_state;
};

struct HmmObs
{
	FLOAT64* obs;	 // Observation feature vector
	FLOAT64* alpha;  // Alpha for each state
	FLOAT64* beta;   // Beta for each state
	FLOAT64* b;      // Emission probability for each state
	INT16 state_occ; // State occupancy
	INT16 mix_occ;   // Mixture occupancy
};

typedef vector<HmmObs> HmmSeq;




//-----------------------------------------------------------------------------
// HmmNetwork Class Definition
//-----------------------------------------------------------------------------
class HmmNetwork
{
public:
	// Initialization
	HmmNetwork(int _verbLvl = 0);
	~HmmNetwork();
	void AddModel(HmmModel* m);
	void ReverseModel();

	// Evaluation
	void NewRecoSeq();
	void AddRecoObs(FLOAT64* o);
	FLOAT64 GetRecoScore();
	FLOAT64 GetHistScore();

	// Training
	void AddTrainSeq();
	void AddTrainObs(FLOAT64* o);

	void InitModel();
	void TrainModel(int num_iter);
	void AlignModel(char* filename);

	// Utility
	void DisplayLattice(int m);
	void PrintModelParams();


private:
	// Gaussian distribution computation
	FLOAT64 LogGaussPdf(HmmState* s, FLOAT64* o);
	FLOAT64 GaussPdf(HmmState* s, FLOAT64* o);
	FLOAT64 SGaussPDF(HmmState* s, FLOAT64* o, int k);
	FLOAT64 SLGaussPDF(HmmMix* mix, FLOAT64* o);


	void ComputeGaussConst();
	void ComputeVarianceFloors(FLOAT64 coeff);

	// Model initialization functions
	bool KMeans(bool doInit);
	FLOAT64 ViterbiSegmentation();
	void ComputeSegmentInfo();
	void ComputeDurationStats();

	// Model initialization utility functions
	FLOAT64 ComputeDistance(FLOAT64* v1, FLOAT64* v2);
	FLOAT64 ComputeMahalanobis(FLOAT64* x, FLOAT64* mu, FLOAT64* var);
	FLOAT64 ComputeChiSquared(INT16 j);

	// Training functions
	void ComputeEmission();
	void ComputeForward();
	void ComputeBackward();

	// Utilities
	void ComputeLogTransP();
	FLOAT64 LogAdd(FLOAT64 logX, FLOAT64 logY);
	inline FLOAT64 Log(FLOAT64 x) {return (x > MIN_LOG ? log(x) : LOG_ZERO);}
	inline FLOAT64 Exp(FLOAT64 logX) {return (logX > MIN_EXP ? exp(logX) : 0.0);}


	// Training parameters
	INT16**   segment;
	INT16*    s_sum;
	FLOAT64** s_mean;
	FLOAT64** s_var;
	FLOAT64*  vFloor;

	// Model parameters
	HmmModel* model;
	HmmState* states;         // All the states in the current network
	FLOAT64** transProb;
	FLOAT64** logTransP;

	INT16 nStates;
	INT16 numDim;

	// Observation sequences
	INT16 i16_num_seq;
	vector< HmmSeq > m_seq; // Sequences used for batch training
	vector< FLOAT64 > alpha_final;
	vector< FLOAT64 > beta_final;

	//NEW
	vector<HmmObsV*> backtrack;

	HmmObsV* m_reco_prev;
	HmmObsV* m_reco_curr;
	INT16*   m_reco_hist;
	INT16    i16_reco_frame;


	FLOAT64 f64_gauss_const;
	int verbLvl;
};

//-----------------------------------------------------------------------------
// HmmUtil Class Definition
//-----------------------------------------------------------------------------
class HmmUtil
{
public:
	static bool SaveBinaryHmm(HmmModel* m, char* filename);
	static HmmModel* CreateHmmPrototype(INT16 nDimensions, INT16 nMix, INT16 nRegularStates, INT16 nSilBeg, INT16 nSilEnd, INT16 nSkip);
	static HmmModel* LoadBinaryHmm(char* filename);
	static void ReverseModel(HmmModel* m);
	static void ExpandModel(HmmModel* m);

private:
	static void InitializeModel(HmmModel* m);
	static FLOAT64 LogDet(FLOAT64* diag, INT16 d);
};




#endif