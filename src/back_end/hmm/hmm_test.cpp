#include <stdio.h>
#include <math.h>

#include "iniparser.h"
#include "Hmm.h"

//-----------------------------------------------------------------------------
// Name: ParseVector
// Desc: Parses a string into a vector of template type
//-----------------------------------------------------------------------------
template <typename T> 
bool HmmUtil::ParseVector(char* str, T* vec, int n)
{
	for(int i = 0; i < n; i++)
		vec[i] = 0.0f;

	char* temp = _strdup(str);
	char* seps = " ,\t\n";
	char *token, *next;
	
	token = strtok_s(temp, seps, &next);
	for(int i = 0; i < n; i++)
    {
		if(token == NULL)
			break;

		vec[i] = (T)(atof(token));
        token = strtok_s( NULL, seps, &next);
    }

	free(temp);
	return true;
}

//-----------------------------------------------------------------------------
// Name: LogDet
// Desc: Computes log determinant of a diagonal matrix
//-----------------------------------------------------------------------------
FLOAT64 HmmUtil::LogDet(FLOAT64* diag, INT16 d)
{
	FLOAT64 det = 1.0;

	// Determinant of a diagonal matrix is simply the product of the diagonal terms
	for(int i = 0; i < d; i++)
		det *= diag[i];
	
	return log(det);
}

void HmmUtil::InitializeModel(HmmModel* m)
{
	for(int i = 0; i < m->nStates; i++)
	{
		for(int j = 0; j < m->states[i].nMix; j++)
		{
			// Compute determinant and log of determinant
			m->states[i].mixtures[j].logDet = LogDet(m->states[i].mixtures[j].vars, m->nDim);
			m->states[i].mixtures[j].det = exp(LogDet(m->states[i].mixtures[j].vars, m->nDim));
		}
	}
}

//-----------------------------------------------------------------------------
// Name: LoadTextHmm
// Desc: Parses a file and allocates an HMM. Memory must be freed later.
//-----------------------------------------------------------------------------
HmmModel* HmmUtil::LoadTextHmm(wchar_t* filename)
{
	char* temp_str = new char[80];
	dictionary * d;
	d = iniparser_load(filename);
	if(d == NULL)
	{
		printf("Couldn't load file: %s\n", filename);
		return NULL;
	}

	HmmModel* m = new HmmModel;

	m->nStates = iniparser_getint(d, "hmm:num_states", 0);
	m->nDim = iniparser_getint(d, "hmm:num_dimensions", 0);
	m->states = new HmmState[m->nStates];

	// Load each state
	for(int i = 0; i < m->nStates; i++)
	{
		sprintf(temp_str, "state%d:num_mixtures", i+1);
		m->states[i].nMix = iniparser_getint(d, temp_str, 0);

		m->states[i].weights = new FLOAT64[m->states[i].nMix];

		sprintf(temp_str, "state%d:weights", i+1);
		ParseVector<FLOAT64>(iniparser_getstr(d, temp_str), m->states[i].weights, m->states[i].nMix);

		m->states[i].mixtures = new HmmMix[m->states[i].nMix];

		// Load each gaussian mixture
		for(int j = 0; j < m->states[i].nMix; j++)
		{
			m->states[i].mixtures[j].means = new FLOAT64[m->nDim];
			m->states[i].mixtures[j].vars = new FLOAT64[m->nDim];

			sprintf(temp_str, "state%d/mix%d:means", i+1, j+1);
			ParseVector<FLOAT64>(iniparser_getstr(d, temp_str), m->states[i].mixtures[j].means, m->nDim);

			sprintf(temp_str, "state%d/mix%d:variances", i+1, j+1);
			ParseVector<FLOAT64>(iniparser_getstr(d, temp_str), m->states[i].mixtures[j].vars, m->nDim);
		}
	}

	m->transProb = new FLOAT64*[m->nStates + 2];

	// Load transition probabilities
	for(int i = 0; i < m->nStates + 2; i++)
	{
		m->transProb[i] = new FLOAT64[m->nStates + 2];

		sprintf(temp_str, "transitions:state%d", i);
		ParseVector<FLOAT64>(iniparser_getstr(d, temp_str), m->transProb[i], m->nStates + 2);
	}

	iniparser_freedict(d);
	delete temp_str;

	InitializeModel(m);
	return m;
}
//-----------------------------------------------------------------------------
// m =       |n_states|n_dimens||state_1|state_2|...|state_n|
// state_n = |n_mix|weights|mix_1|mix_2|...|mix_n|
// mix_n =   |means|vars|
//-----------------------------------------------------------------------------
bool HmmUtil::SaveBinaryHmm(HmmModel* m, wchar_t* filename)
{
	FILE* f = _wfopen(filename, L"wb");
	if(f == NULL)
	{
		wprintf(L"Couldn't open file: %s\n", filename);
		return false;
	}

	fwrite(&m->nStates, sizeof(INT16), 1, f); // Number of states
	fwrite(&m->nDim, sizeof(INT16), 1, f); // Number of dimensions

	// Write each state
	for(int i = 0; i < m->nStates; i++)
	{
		fwrite(&m->states[i].nMix, sizeof(INT16), 1, f);                             // Number of mixtures
		fwrite(m->states[i].weights, sizeof(FLOAT64), m->states[i].nMix, f);  // Weights

		// Write each mixture
		for(int j = 0; j < m->states[i].nMix; j++)
		{
			fwrite(m->states[i].mixtures[j].means, sizeof(FLOAT64), m->nDim, f); // Means
			fwrite(m->states[i].mixtures[j].vars, sizeof(FLOAT64), m->nDim, f);  // Variances
		}
	}

	// Write transition probabilities
	for(int i = 0; i < m->nStates + 2; i++)
		fwrite(m->transProb[i], sizeof(FLOAT64), m->nStates + 2, f);

	// Write sequence duration mean and variance
	fwrite(&m->lengthMean, sizeof(FLOAT64), 1, f);
	fwrite(&m->lengthVar, sizeof(FLOAT64), 1, f);

	fclose(f);
	return true;
}

//-----------------------------------------------------------------------------
// m =       |n_states|n_dimens||state_1|state_2|...|state_n|
// state_n = |n_mix|weights|mix_1|mix_2|...|mix_n|
// mix_n =   |means|vars|
//-----------------------------------------------------------------------------
HmmModel* HmmUtil::LoadBinaryHmm(wchar_t* filename)
{
	FILE* f = _wfopen(filename, L"rb");
	if(f == NULL)
	{
		wprintf(L"Couldn't open file: %s\n", filename);
		return false;
	}

	HmmModel* m = new HmmModel;

	fread(&m->nStates, sizeof(INT16), 1, f); // Number of states
	fread(&m->nDim, sizeof(INT16), 1, f); // Number of dimensions

	// Allocate memory & read in states
	m->states = new HmmState[m->nStates]; 

	for(int i = 0; i < m->nStates; i++)
	{
		// Number of mixtures
		fread(&m->states[i].nMix, sizeof(INT16), 1, f);  

		// Allocate mem & read in weights
		m->states[i].weights = new FLOAT64[m->states[i].nMix]; 
		fread(m->states[i].weights, sizeof(FLOAT64), m->states[i].nMix, f);

		// Allocate mem and read in each mixture
		m->states[i].mixtures = new HmmMix[m->states[i].nMix]; 

		for(int j = 0; j < m->states[i].nMix; j++)
		{
			m->states[i].mixtures[j].means = new FLOAT64[m->nDim];
			m->states[i].mixtures[j].vars = new FLOAT64[m->nDim];

			fread(m->states[i].mixtures[j].means, sizeof(FLOAT64), m->nDim, f); // Means
			fread(m->states[i].mixtures[j].vars, sizeof(FLOAT64), m->nDim, f);  // Variances
		}
	}

	// Allocate mem & read transition probabilities
	m->transProb = new FLOAT64*[m->nStates + 2];

	for(int i = 0; i < m->nStates + 2; i++)
	{
		m->transProb[i] = new FLOAT64[m->nStates + 2];
		fread(m->transProb[i], sizeof(FLOAT64), m->nStates + 2, f);
	}

	// Write sequence duration mean and variance
	if(!fread(&m->lengthMean, sizeof(FLOAT64), 1, f))
		m->lengthMean = 0.0;
	if(!fread(&m->lengthVar, sizeof(FLOAT64), 1, f))
		m->lengthVar = 0.0;

	//if(m->lengthMean == 0.0 || m->lengthVar == 0.0)
	//{
	//	fprintf(stderr, "WARNING: Duration mean & var not found in model. Using simple normalization\n");
	//}
	//else
	//{
	//	fprintf(stderr, "Model duration mean: %f var: %f\n", m->lengthMean, m->lengthVar);
	//}

	fclose(f);

	InitializeModel(m);
	return m;
}

//-----------------------------------------------------------------------------
// Name: HmmUtil::CreateHmmPrototype
// Desc: This function builds a left to right HMM prototype similar to our Matlab 
//       gen_model() function.
//
// Params: 
//
//   nDimensions     - number of dimensions
//   nMix            - number of mixtures (constant for each state)
//   nSpeechStates   - number of states that model speech
//   nSilStatesBeg   - number of beginning states that model silence
//   nSilStatesEnd   - number of end states that model silence
//   nSkip           - number of skips allowed
//
//-----------------------------------------------------------------------------
HmmModel* HmmUtil::CreateHmmPrototype( INT16 nDimensions, INT16 nMix, INT16 nRegularStates, INT16 nSilBeg, INT16 nSilEnd, INT16 nSkip )
{
	HmmModel* m = new HmmModel;
	INT16 nTotalStates = nRegularStates + nSilBeg + nSilEnd;

	// Set number of dimensions and states
	m->nDim = nDimensions;
	m->nStates = nTotalStates;
	
	// Allocate memory for states
	m->states = new HmmState[m->nStates]; 

	// Initialize states
	for(int i = 0; i < m->nStates; i++)
	{
		HmmState* pCurrentState = m->states + i;

		// Set number of mixtures (currently constant for every state)
		pCurrentState->nMix = nMix;

		// Allocate mem for mix weights
		pCurrentState->weights = new FLOAT64[m->states[i].nMix]; 

		// Allocate mem for mixtures
		pCurrentState->mixtures = new HmmMix[pCurrentState->nMix]; 

		// Initialize mixtures
		for(int k = 0; k < pCurrentState->nMix; k++)
		{
			HmmMix* pCurrentMix = pCurrentState->mixtures + k;

			// Initial mix weight is 1.0 / nMix
			pCurrentState->weights[k] = (1.0 / pCurrentState->nMix);

			// Allocate mem for means and vars
			pCurrentMix->means = new FLOAT64[m->nDim];
			pCurrentMix->vars = new FLOAT64[m->nDim];

			// Set initial means and vars to 1.0
			for(int d = 0; d < m->nDim; d++)
			{
				pCurrentMix->means[d] = 1.0;
				pCurrentMix->vars[d] = 1.0;
			}
		}
	}

	// Allocate mem for transition probability matrix and set to zero
	INT16 nTransitions = (m->nStates + 2);

	m->transProb = new FLOAT64*[nTransitions];
	for(int i = 0; i < nTransitions; i++)
	{
		m->transProb[i] = new FLOAT64[nTransitions];
		memset(m->transProb[i], 0, nTransitions * sizeof(FLOAT64));
	}

	INT16 nReachableStates = 0;
	
	// Set entry probabilities
	nReachableStates = (nSilBeg + 1 + nSkip);
	for(int j = 1; j < (1 + nReachableStates); j++)
	{
		m->transProb[0][j] = 1.0 / nReachableStates;
	}

	// Set beginning silence states
	INT16 BeginState = 1;
	INT16 EndState = 1 + nSilBeg;
	
	for(int i = BeginState; i < EndState; i++)
	{
		// Remaining silence states, one regular state + number of skip states
		nReachableStates = (nSilBeg - (i-1)) + 1 + nSkip;
		for(int j = i; j < (i + nReachableStates); j++)
		{
			m->transProb[i][j] = 1.0 / nReachableStates;
		}
	}

	// Set regular states
	BeginState = EndState;
	EndState = 1 + nSilBeg + nRegularStates - 1 - nSkip;

	for(int i = BeginState; i < EndState; i++)
	{
		// Itself, the next state, plus the number skip states
		nReachableStates = 1 + 1 + nSkip;
		for(int j = i; j < (i + nReachableStates); j++)
		{
			m->transProb[i][j] = 1.0 / nReachableStates;
		}
	}

	// Set end silence states
	BeginState = EndState;
	EndState = 1 + nSilBeg + nRegularStates + nSilEnd;

	for(int i = BeginState; i < EndState; i++)
	{
		nReachableStates = nTransitions - i;		
		for(int j = i; j < (i + nReachableStates); j++)
		{
			m->transProb[i][j] = 1.0 / nReachableStates;
		}
	}

	m->lengthMean = 0.0;
	m->lengthVar = 0.0;

	InitializeModel(m);
	return m;
}


void HmmUtil::ReverseModel(HmmModel* m)
{
	INT16 nStates = m->nStates;
	FLOAT64** transProb = m->transProb;

	// Reverse inner states
	for(int i = 1; i < nStates + 1; i++)
	{
		for(int j = i + 1; j < nStates + 1; j++)
		{
			FLOAT64 temp = transProb[i][j];
			transProb[i][j] = transProb[j][i];
			transProb[j][i] = temp;
		}
	}

	// Reverse entry / exit states
	for(int i = 1; i < nStates + 1; i++)
	{
		FLOAT64 temp = transProb[0][i];
		transProb[0][i] = transProb[i][nStates+1];
		transProb[i][nStates+1] = temp;
	}

	// Normalize probabilities to 1.0
	for(int i = 0; i < nStates + 1; i++)
	{
		FLOAT64 sum = 0.0;
		for(int j = 1; j < nStates + 2; j++)
			sum += transProb[i][j];
		
		for(int j = 1; j < nStates + 2; j++)
			//if(sum > 0.0) transProb[i][j] = 1.0; 
			//if(sum > 0.0) transProb[i][j] /= sum;
			if(transProb[i][j] > 0) transProb[i][j] = 1.0;
	}
}

void HmmUtil::ExpandModel(HmmModel* m)
{
	INT16 nStates = m->nStates;
	FLOAT64** transProb = m->transProb;

	// Normalize probabilities to 1.0
	for(int i = 0; i < nStates + 1; i++)
	{
		for(int j = 1; j < nStates + 2; j++)
			transProb[i][j] = 1.0 / (nStates + 1); 
	}

}

