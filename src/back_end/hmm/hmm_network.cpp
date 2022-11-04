#include <string.h>
#include <stdio.h>
#include <math.h>
#include "hmm.h"

//-----------------------------------------------------------------------------
// Name: HmmNetwork::HmmNetwork
// Desc: Constructor
//-----------------------------------------------------------------------------
HmmNetwork::HmmNetwork(int _verbLvl)
{
	verbLvl = _verbLvl;
	i16_num_seq = 0;
	nStates = 0;
	numDim = 0;
	i16_reco_frame = 0;
}
//-----------------------------------------------------------------------------
// Name: HmmNetwork::~HmmNetwork
// Desc: Destructor
//-----------------------------------------------------------------------------
HmmNetwork::~HmmNetwork()
{
}

//-----------------------------------------------------------------------------
// Name: HmmNetwork::AddModel
// Desc:
//-----------------------------------------------------------------------------
void HmmNetwork::AddModel(HmmModel *m)
{
	model = m;
	states = m->states;
	transProb = m->transProb;
	nStates = m->nStates;
	numDim = m->nDim;
	f64_gauss_const = pow(2.0*PI, numDim);



	// Allocate log transition prob matrix
	logTransP = new FLOAT64*[nStates + 2];

	for(int i = 0; i < nStates + 2; i++)
		logTransP[i] = new FLOAT64[nStates + 2];

	ComputeLogTransP();  // Precompute log of transition probs
	ComputeGaussConst(); // Precompute gaussian constants

	m_reco_prev = new HmmObsV[nStates];
	m_reco_curr = new HmmObsV[nStates];
	m_reco_hist = new INT16[nStates];
}



void HmmNetwork::AddTrainSeq()
{
#if 0
	// HACK exclude sequences with low avg. energy
	if(m_seq.size() > 0)
	{
		HmmSeq& s = m_seq.back();
		FLOAT64 e = 0.0;
		for(int t = 0; t < s.size(); t++)
			e += s[t].obs[12];
		e /= s.size();

		if( e < 100.0)
		{
			printf("WARNING: Rejecting previous file due to low energy!\n");
			m_seq.pop_back();
		}
	}
#endif


	HmmSeq seq;
	seq.reserve(256);
	m_seq.push_back(seq);
	i16_num_seq = m_seq.size();
}

void HmmNetwork::AddTrainObs(FLOAT64 *o)
{
	HmmObs obs;
	obs.alpha = new FLOAT64[nStates];
	obs.beta  = new FLOAT64[nStates];
	obs.b     = new FLOAT64[nStates];
	obs.obs   = new FLOAT64[numDim];

	memcpy(obs.obs, o, numDim * sizeof(FLOAT64));
	m_seq.back().push_back(obs);

	// TODO release this memory when training is complete
}

void HmmNetwork::NewRecoSeq()
{
	memset(m_reco_hist, 0, nStates * sizeof(INT16));
	i16_reco_frame = 0;
}



//-----------------------------------------------------------------------------
// Name: HmmNetwork::AddObservation
// Desc:
//-----------------------------------------------------------------------------
void HmmNetwork::AddRecoObs(FLOAT64* o)
{
	if(i16_reco_frame == 0)
	{
		// Loop through each state of the current model and compute prior prob
		for(int j = 0; j < nStates; j++)
		{
			// alpha_t(i) = pi_i * b_i(X_i)
			m_reco_prev[j].prev_state = -1;
			m_reco_prev[j].score = LogGaussPdf(&states[j], o) + (logTransP[0][j+1]);
		}
	}
	else
	{
		FLOAT64 best_score = LOG_ZERO;
		INT16 best_state = -1;

		//Loop through each state of the current model and compute prob
		for(int j = 0; j < nStates; j++)
		{
			// Initialization
			m_reco_curr[j].score = m_reco_prev[0].score + logTransP[1][j+1];
			m_reco_curr[j].prev_state = 0;

			// Compute transition probabilities
			for(int i = 1; i < nStates; i++)
			{
				// Skip low probabilities
				if(logTransP[i+1][j+1] <= LOG_SMALL)
					continue;

				// Viterbi probability
				FLOAT64 temp = m_reco_prev[i].score + logTransP[i+1][j+1];

				if(temp > m_reco_curr[j].score)
				{
					m_reco_curr[j].score = temp;
					m_reco_curr[j].prev_state = i;
				}
			}

			// Multiply by emission probability
			FLOAT64 b = LogGaussPdf(&states[j], o);
			m_reco_curr[j].score += b;

			// Histogram computation
			if(b > best_score)
			{
				best_score = b;
				best_state = j;
			}
		}

		m_reco_hist[best_state]++;

		// Swap prev & curr pointers
		HmmObsV* temp = m_reco_curr;
		m_reco_curr = m_reco_prev;
		m_reco_prev = temp;
	}

	// Increment reco frame
	i16_reco_frame++;
}


FLOAT64 HmmNetwork::GetRecoScore()
{
	HmmObsV final_obs;

	// Final score and state
	final_obs.score = m_reco_prev[0].score + logTransP[1][nStates+1];

	for(int i = 1; i < nStates; i++)
	{
		// Skip zero probabilities
		if(logTransP[i+1][nStates+1] <= LOG_SMALL)
			continue;

		FLOAT64 score = m_reco_prev[i].score + logTransP[i+1][nStates+1];

		if(score > final_obs.score )
		{
			final_obs.score = score;
			final_obs.prev_state = i;
		}
	}

   //printf("Number of frames: %d\n", i16_reco_frame);
	final_obs.score = (final_obs.score / i16_reco_frame);

	//// New normalization
	//if(model->lengthMean > 0.0 && model->lengthVar > 0.0)
	//{
	//	FLOAT64 diff = (model->lengthMean - i16_reco_frame);
	//	final_obs.score -= 3 * ((diff * diff) / model->lengthVar);
	//}

	return final_obs.score;
}

FLOAT64 HmmNetwork::GetHistScore()
{
	// New normalization
	if(model->lengthMean > 0.0 && model->lengthVar > 0.0)
	{
		FLOAT64 diff = (model->lengthMean - i16_reco_frame);
		FLOAT64 m = ((diff * diff) / (2 * model->lengthVar));
		return -Log(model->lengthVar) - m;
	}

	return 0;

	FLOAT64* exVal = new FLOAT64[nStates];
	FLOAT64 norm = 0.0;
	for(int j = 0; j < nStates; j++)
	{
		exVal[j] = 1.0 / (1.0 - transProb[j+1][j+1]);
		norm += exVal[j];
	}


	FLOAT64 sum = 0.0;

	//printf("\nHist: [");
	// Compute histogram diff
	for(int j = 5; j < nStates - 5; j++)
	{
		FLOAT64 delta = ((FLOAT64)m_reco_hist[j] / (i16_reco_frame-1)) - (exVal[j] / norm);
		//printf("%.2f ", delta);
		//if(delta > 0)
		if(m_reco_hist[j] > 0)
			sum += delta * delta; //abs(delta);// exp(0.01* delta);
	}
 	//printf("]\n");

	delete[] exVal;
	return sqrt(sum);
}


//-----------------------------------------------------------------------------
// Name: HmmNetwork::ComputeEmission
// Desc: Computes emission probabilities for all training sequences
//-----------------------------------------------------------------------------
void HmmNetwork::ComputeEmission()
{
	for(int m = 0; m < i16_num_seq; m++)
	{
		int T = m_seq[m].size();

		for(int t = 0; t < T; t++)
		{
			HmmObs* o = &m_seq[m][t];
			for(int j = 0; j < nStates; j++)
				o->b[j] = LogGaussPdf(&states[j], o->obs);
		}
	}
}


//-----------------------------------------------------------------------------
// Name: HmmNetwork::ComputeForward
// Desc: Computes forward probability (alpha) on all training sequences
//-----------------------------------------------------------------------------
void HmmNetwork::ComputeForward()
{
	for(int m = 0; m < i16_num_seq; m++)
	{
		int T = m_seq[m].size();

		// Initialization
		for(int j = 0; j < nStates; j++)
		{
			if(logTransP[0][j+1] > LOG_SMALL)
				m_seq[m][0].alpha[j] = logTransP[0][j+1] + m_seq[m][0].b[j];
			else
				m_seq[m][0].alpha[j] = LOG_ZERO;
		}

		// Loop through each observation
		for(int t = 1; t < T; t++)
		{
			// Loop through each state of the current model and compute prob
			for(int j = 0; j < nStates; j++)
			{
				FLOAT64 alpha = LOG_ZERO;

				// Compute transition probabilities
				for(int i = 0; i < nStates; i++)
				{
					if(logTransP[i+1][j+1] > LOG_SMALL)
						alpha = LogAdd(alpha, m_seq[m][t-1].alpha[i] + logTransP[i+1][j+1]);
				}

				// Multiply by emission probability
				m_seq[m][t].alpha[j] = alpha + m_seq[m][t].b[j];
			}
		}

		// Finalization
		alpha_final[m] = LOG_ZERO;

		for(int j = 0; j < nStates; j++)
		{
			if(logTransP[j+1][nStates+1] > LOG_SMALL)
				alpha_final[m] = LogAdd(alpha_final[m], logTransP[j+1][nStates+1] + m_seq[m][T-1].alpha[j]);
		}

		//DisplayLattice(m);
	}
}

//-----------------------------------------------------------------------------
// Name: HmmNetwork::ComputeBackward
// Desc: Computes backward probability (beta) on all training sequences
//-----------------------------------------------------------------------------
void HmmNetwork::ComputeBackward()
{
	for(int m = 0; m < i16_num_seq; m++)
	{
		int T = m_seq[m].size();

		// Initialization
		for(int j = 0; j < nStates; j++)
			m_seq[m][T-1].beta[j] = logTransP[j+1][nStates+1];

		// Go backwards through the trellis
		for(int t = T-2; t >= 0; t--)
		{
			// Loop through each state of the current model and compute prob
			for(int i = 0; i < nStates; i++)
			{
				m_seq[m][t].beta[i] = LOG_ZERO;

				for(int j = 0; j < nStates; j++)
				{
					if(logTransP[i+1][j+1] > LOG_SMALL)
						m_seq[m][t].beta[i] = LogAdd(m_seq[m][t].beta[i], (m_seq[m][t+1].beta[j] + logTransP[i+1][j+1] + m_seq[m][t+1].b[j]));
				}
			}
		}

		// Finalization
		beta_final[m] = LOG_ZERO;

		for(int j = 0; j < nStates; j++)
			beta_final[m] = LogAdd(beta_final[m], logTransP[0][j+1] + m_seq[m][0].beta[j] + m_seq[m][0].b[j]);

	}
}


//-----------------------------------------------------------------------------
// Name: HmmNetwork::TrainModel
// Desc:
//-----------------------------------------------------------------------------
void HmmNetwork::TrainModel(int num_iter)
{
	// Allocate memory for accumulators
	FLOAT64*** mean_num   = new FLOAT64**[nStates];
	FLOAT64*** var_num    = new FLOAT64**[nStates];

	FLOAT64** mix_prob   = new FLOAT64*[nStates];
	FLOAT64*  state_prob = new FLOAT64[nStates];

	FLOAT64**  trans_num  = new FLOAT64*[nStates];
	FLOAT64*   trans_den  = new FLOAT64[nStates];
	FLOAT64*   trans_entr  = new FLOAT64[nStates];
	FLOAT64*   trans_exit  = new FLOAT64[nStates];

	for(int j = 0; j < nStates; j++)
	{
		int K = states[j].nMix;

		mean_num[j]   = new FLOAT64*[K];
		var_num[j]    = new FLOAT64*[K];
		mix_prob[j]   = new FLOAT64[K];

		trans_num[j] = new FLOAT64[nStates];

		for(int k = 0; k < K; k++)
		{
			mean_num[j][k] = new FLOAT64[numDim];
			var_num[j][k]  = new FLOAT64[numDim];
		}
	}
	// End: Allocating memory for accumulators

	//printf("Initial Model Parameters:\n");
	//PrintModelParams();

	for(int iter = 0; iter < num_iter; iter++)
	{

		printf("\n--------------------------------------------------\n", iter+1);
		printf("Baum-Welch Iteration %d\n", iter+1);

		// Reset accumulator values
		memset(trans_den, 0, nStates * sizeof(FLOAT64));
		memset(trans_entr, 0, nStates * sizeof(FLOAT64));
		memset(trans_exit, 0, nStates * sizeof(FLOAT64));
		memset(state_prob, 0, nStates * sizeof(FLOAT64));

		for(int j = 0; j < nStates; j++)
		{
			int K = states[j].nMix;

			memset(mix_prob[j], 0, K * sizeof(FLOAT64));
			memset(trans_num[j], 0, (nStates+1) * sizeof(FLOAT64));

			for(int k = 0; k < K; k++)
			{
				memset(mean_num[j][k], 0, numDim * sizeof(FLOAT64));
				memset(var_num[j][k], 0, numDim * sizeof(FLOAT64));
			}
		}

		alpha_final.clear(); alpha_final.resize(m_seq.size());
		beta_final.clear(); beta_final.resize(m_seq.size());

		// Compute alpha and beta values
		ComputeGaussConst();
		ComputeEmission();
		ComputeForward();
		ComputeBackward();

		FLOAT64 log_total = 0.0;

		int seq = 0;
		for(int m = 0; m < i16_num_seq; m++)
		{
			//printf("Alpha: %e Beta: %e Diff: %e\n", alpha_final[m], beta_final[m], (alpha_final[m] - beta_final[m]));
			if(alpha_final[m] < LOG_SMALL)
				continue;

			seq++;
			alpha_final[m] = (alpha_final[m] + beta_final[m]) / 2.0;
			log_total += alpha_final[m];
		}

		log_total /= seq;

		printf(" Average log probability for %d sequences: %f\n", seq, log_total);
		fflush(stdout);

		// Loop through each sequence
		for(int m = 0; m < i16_num_seq; m++)
		{
			if(	alpha_final[m] < LOG_SMALL )
			{
				printf("  BaumWelch: Skipping sequence %d\n", m+1);
				continue;
			}

			int T = m_seq[m].size();

			// Loop through each state
			for(int j = 0; j < nStates; j++)
			{
				int K = states[j].nMix;

				FLOAT64 sum = LOG_ZERO;
				FLOAT64 betasum = LOG_ZERO;

				// Loop through each time, T
				for(int t = 0; t < T; t++)
				{
					// Compute state probability
					state_prob[j] += Exp(m_seq[m][t].alpha[j] + m_seq[m][t].beta[j] - alpha_final[m]);

					// Loop through each mixture
					for(int k = 0; k < K; k++)
					{
						// Compute Likelihood
						FLOAT64 u_jt = LOG_ZERO;
						if(t == 0)
						{
							u_jt = logTransP[0][j+1];
						}
						else
						{
							for(int i = 0; i < nStates; i++)
								u_jt = LogAdd(u_jt, (logTransP[i+1][j+1] + m_seq[m][t-1].alpha[i]));
						}

						FLOAT64 c_jk = log(states[j].weights[k]);
						FLOAT64 b_jkt = SLGaussPDF(&states[j].mixtures[k], m_seq[m][t].obs);
						FLOAT64 beta_jt = m_seq[m][t].beta[j];

						FLOAT64 mix_likelihood = Exp(u_jt + c_jk + b_jkt + beta_jt - alpha_final[m]);


						//printf("Mix likelihood (state: %d, mix: %d, time: %d): %.3g\n", j, k, t, mix_likelihood);
						//printf("Compare with %g\n", logbase->InvLogB(m_seq[m][t].alpha[j] + m_seq[m][t].beta[j] - alpha_final[m]));

						// Update mean and variance accumulators
						for(int d = 0; d < numDim; d++)
						{
							FLOAT64 diff = m_seq[m][t].obs[d];// - states[j].mixtures[k].means[d];

							mean_num[j][k][d] += (mix_likelihood * m_seq[m][t].obs[d]);
							var_num[j][k][d] += (mix_likelihood * diff * diff);
						}

						// Update mixture probability counter
						mix_prob[j][k] += mix_likelihood;
					}
				}


				FLOAT64 den = LOG_ZERO, num = LOG_ZERO;

				// Denominator for transition probabilities
				for(int t = 0; t < T; t++)
					den = LogAdd(den, (m_seq[m][t].alpha[j] + m_seq[m][t].beta[j]));

				den -= alpha_final[m];

				trans_den[j] += Exp(den);

				// Numerator for transition probabilities
				//int i = j;
				for(int i = 0; i < nStates; i++)
				{
					num = LOG_ZERO;
					for(int t = 0; t < (T-1); t++)
						num = LogAdd(num, (m_seq[m][t].alpha[j] + logTransP[j+1][i+1] + m_seq[m][t+1].beta[i] + m_seq[m][t+1].b[i]));
					num -= alpha_final[m];

					trans_num[j][i] += Exp(num);
				}

				// Exit probabilities
				trans_exit[j] += Exp(m_seq[m][T-1].alpha[j] + m_seq[m][T-1].beta[j] - alpha_final[m]);

				// Entrance probabilities
				trans_entr[j] += Exp(m_seq[m][0].alpha[j] + m_seq[m][0].beta[j] - alpha_final[m]);

			}
		}

		// Now update parameters based on the accumulators
		for(int j = 0; j < nStates; j++)
		{
			int K = states[j].nMix;

			for(int k = 0; k < K; k++)
			{
				states[j].mixtures[k].det = 1.0;
				states[j].weights[k] = mix_prob[j][k] / state_prob[j];

				for(int d = 0; d < numDim; d++)
				{
					FLOAT64 mean = (mean_num[j][k][d] / mix_prob[j][k]);
					states[j].mixtures[k].means[d] = (mean_num[j][k][d] / mix_prob[j][k]);
					states[j].mixtures[k].vars[d] = (var_num[j][k][d] / mix_prob[j][k]) - (mean * mean);
					if(states[j].mixtures[k].vars[d] < vFloor[d])
					{
						//printf("ERR: State %d Mixture %d Dim: %d : variance floor\n", j, k, d);
						states[j].mixtures[k].vars[d] = vFloor[d];
					}
				}
			}

			// Entrance
			transProb[0][j+1] = trans_entr[j] / i16_num_seq;

			// Exit
			transProb[j+1][nStates+1] = trans_exit[j] / trans_den[j];

			for(int i = 0; i < nStates; i++)
			{
				if(trans_den[j])
					transProb[j+1][i+1] = trans_num[j][i] / trans_den[j];
				else
					transProb[j+1][i+1] = 0.0;
			}

			ComputeLogTransP();
		}

		//PrintModelParams();
	}

	// TODO release memory for accumulators

	//ComputeForward();
	//DisplayLattice(m);
	//PrintModelParams();
}


//-----------------------------------------------------------------------------
// Name: HmmNetwork::LogGaussPdf
// Desc: Computes the log probability of a gaussian mixture model
//-----------------------------------------------------------------------------
FLOAT64 HmmNetwork::LogGaussPdf(HmmState* s, FLOAT64* o)
{
	FLOAT64 p = LOG_ZERO;

	for(int k = 0; k < s->nMix; k++)
	{
		if(s->weights[k] > 0.0)
			p = LogAdd(p, Log(s->weights[k]) + SLGaussPDF(&s->mixtures[k], o));
	}
	return p;
}

//-----------------------------------------------------------------------------
// Name: HmmNetwork::SLGaussPDF
// Desc: Computes the single mixture log gauss pdf
//-----------------------------------------------------------------------------
inline FLOAT64 HmmNetwork::SLGaussPDF(HmmMix* mix, FLOAT64* o)
{
	// Compute Mahalanobis distance
	FLOAT64 m = 0.0;
	for(int d = 0; d < numDim; d++)
	{
		FLOAT64 diff = o[d] - mix->means[d];
		m += (diff * diff / mix->vars[d]);
	}

	// HACK fix this
	if(*(int64_t*)&m == 0xfff8000000000000)
	{
		return LOG_ZERO;
	}

	FLOAT64 ans = (-0.5 * (mix->logGconst + m));
	return ans;
}


//-----------------------------------------------------------------------------
// Name: GaussPdf
// Desc: This function computes the multivariate, multimixture gaussian function.
//-----------------------------------------------------------------------------
FLOAT64 HmmNetwork::GaussPdf(HmmState* s, FLOAT64* o)
{
	FLOAT64 p = 0.0;

	for(int i = 0; i < s->nMix; i++)
		p += (s->weights[i] * SGaussPDF(s, o, i));

	return (p);
}

//-----------------------------------------------------------------------------
// Name: SingleMixGaussPdf
// Desc: This function computes the gaussian pdf for a single mixture. It does not
//       factor in the mixture weight.
//-----------------------------------------------------------------------------
inline FLOAT64 HmmNetwork::SGaussPDF(HmmState* s, FLOAT64* o, int k)
{
	// Compute Mahalanobis distance
	FLOAT64 m = 0.0;
	for(int j = 0; j < numDim; j++)
	{
		FLOAT64 diff = (o[j] - s->mixtures[k].means[j]);
		m += (diff * diff) / (s->mixtures[k].vars[j]);
	}

	// TODO precompute sqrt equation
	m = (exp(-.5 *  m) / (sqrt(s->mixtures[k].det * f64_gauss_const)));

	if(*(int64_t*)&m == 0xfff8000000000000)
		m = 0.0;

	return m;
}



//-----------------------------------------------------------------------------
// Name: HmmNetwork::DisplayLattice()
// Desc: Displays the current lattice
//-----------------------------------------------------------------------------
void HmmNetwork::DisplayLattice(int m)
{
	if(m >= 0)
	{
		printf("Displaying lattice for sequence %d\n", m + 1);
		for(int t = 0; t < m_seq[m].size(); t++)
		{
			printf("%d: ", t);
			for(int j = 0; j < nStates; j++)
				printf("(A:%.3f B:%.3f)\t", m_seq[m][t].alpha[j], m_seq[m][t].beta[j]);
				//printf("(A:%.3f)\t", m_seq[m][t].alpha[j]);
			printf("\n");
		}
	}
}

//-----------------------------------------------------------------------------
// Name: HmmNetwork::ComputeDistance()
// Desc: Computes Euclidean distance between 2 points
//-----------------------------------------------------------------------------
inline FLOAT64 HmmNetwork::ComputeDistance(FLOAT64* v1, FLOAT64* v2)
{
	FLOAT64 sum = 0.0;
	for(int d = 0; d < numDim; d++)
	{
		FLOAT64 diff = v1[d]-v2[d];
		sum += diff * diff;
	}
	return sqrt(sum);
}

inline FLOAT64 HmmNetwork::ComputeMahalanobis(FLOAT64* x, FLOAT64* mu, FLOAT64* var)
{
	FLOAT64 sum = 0.0;
	for(int d = 0; d < numDim; d++)
	{
		FLOAT64 diff = x[d] - mu[d];
		sum += (diff * diff / var[d]);
	}
	return sum;
}

FLOAT64 HmmNetwork::ViterbiSegmentation()
{
	HmmObsV final_obs;
	FLOAT64 avg_score = 0.0;
	int num_seq = 0;

	for(int m = 0; m < i16_num_seq; m++)
	{
		int T = m_seq[m].size();
		HmmObsV** trellis = new HmmObsV*[T];
		trellis[0] = new HmmObsV[nStates];

		// Initialization
		for(int j = 0; j < nStates; j++)
		{
			trellis[0][j].score = LogGaussPdf(&states[j], m_seq[m][0].obs) + (logTransP[0][j+1]);
			trellis[0][j].prev_state = -1;
		}

		for(int t = 1; t < T; t++)
		{
			trellis[t] = new HmmObsV[nStates];

			// Loop through each state of the current model and compute prob
			for(int j = 0; j < nStates; j++)
			{
				// Initialize by assuming best incoming state is state 1
				trellis[t][j].score = trellis[t-1][0].score + logTransP[1][j+1];
				trellis[t][j].prev_state = 0;

				// Find the maximum probability transition
				for(int i = 1; i < nStates; i++)
				{
					if(LOG_ZERO == logTransP[i+1][j+1])
						continue;

					FLOAT64 score = trellis[t-1][i].score + (logTransP[i+1][j+1]);
					if(score > trellis[t][j].score )
					{
						trellis[t][j].score = score;
						trellis[t][j].prev_state = i;
					}
				}

				// Multiply by emission probability
				trellis[t][j].score += LogGaussPdf(&states[j], m_seq[m][t].obs);
			}
		}

		// Final score and state
		final_obs.score = trellis[T-1][0].score + (logTransP[1][nStates+1]);
		final_obs.prev_state = 0;

		for(int i = 0; i < nStates; i++)
		{
			FLOAT64 score = trellis[T-1][i].score + (logTransP[i+1][nStates+1]);
			if(score > final_obs.score )
			{
				final_obs.score = score;
				final_obs.prev_state = i;
			}
		}

#if 0
		for(int t = 0; t < T; t++)
		{
			printf("%d: ", t);
			for(int j = 0; j < nStates; j++)
				printf("%.3f %d\t", trellis[t][j].score, trellis[t][j].prev_state);
			printf("\n");
		}
#endif

		if(final_obs.score < LOG_SMALL)
		{
			printf("  ViterbiSeg: Skipping sequence %d (length: %d)\n", m+1, m_seq[m].size());
			memset(segment[m], 0, (nStates + 1 ) * sizeof(INT16));
		}
		else
		{
			avg_score += final_obs.score;
			num_seq++;

			// Perform backtracking and re-segmentation
			int prev = nStates;
			int curr = final_obs.prev_state;

			// Initialize segment
			for(int i = prev; i > curr; i--)
				segment[m][i] = T;

			for(int t = T-1; t >= 0; t--)
			{
				if(trellis[t][curr].prev_state != curr)
				{
					prev = curr;
					curr = trellis[t][curr].prev_state;

					// HACK Only works for left-right HMMs
					for(int i = prev; i > curr; i--)
						segment[m][i] = t;
				}
			}
		}

		// Cleanup
		for(int t = 0; t < T; t++)
			delete[] trellis[t];
		delete[] trellis;
	}
	return (avg_score / num_seq);
}

bool HmmNetwork::KMeans(bool doInit)
{
	INT8**   k_index = new INT8*[i16_num_seq];

	// Initialize K-Means indices and set to 0
	for(int m = 0; m < i16_num_seq; m++)
	{
		k_index[m] = new INT8[m_seq[m].size()];
		memset(k_index[m], 0, m_seq[m].size() * sizeof(INT8));
	}

	// Proceed for each state in the model
	for(int j = 0; j < nStates; j++)
	{
		int K = states[j].nMix;
		FLOAT64** new_means = new FLOAT64*[K];
		FLOAT64** new_vars = new FLOAT64*[K];
		INT16*    num_index = new INT16[K];

		for(int k = 0; k < K; k++)
		{
			new_means[k] = new FLOAT64[numDim];
			new_vars[k] = new FLOAT64[numDim];
		}


		if(doInit)
		{
			// Initialize K-Means by spreading points uniformly within the range of the data
			for(int d = 0; d < numDim; d++)
			{
				FLOAT64 s_min = s_mean[j][d] - 2 * sqrt(s_var[j][d]);
				FLOAT64 s_max = s_mean[j][d] + 2 * sqrt(s_var[j][d]);
				FLOAT64 range = s_max - s_min;

				for(int k = 0; k < K; k++)
				{
					states[j].mixtures[k].means[d] = s_min + range/(K+1) * (k+1);
					states[j].mixtures[k].vars[d] = s_var[j][d];
				}
			}
		}


		// Initialize K-Means
		bool bChanged = true;
		int k_iter = 0, MAX_ITER = 25;
		if(!doInit)
			MAX_ITER = 1;

		while(bChanged && k_iter < MAX_ITER)
		{
			k_iter++;
			bChanged = false;

			for(int k = 0; k < K; k++)
			{
				memset(new_means[k], 0, numDim * sizeof(FLOAT64));
				memset(new_vars[k], 0, numDim * sizeof(FLOAT64));
			}

			memset(num_index, 0, K * sizeof(INT16));

			// Loop through all points
			for(int m = 0; m < i16_num_seq; m++)
			{
				for(int t = segment[m][j]; t < segment[m][j+1]; t++)
				{
					FLOAT64 min_dist = HUGE_VAL;
					int min_index = -1;

					// Find the nearest mean
					for(int k = 0; k < K; k++)
					{
						FLOAT64 dist;

						if(doInit)
							dist = ComputeDistance(m_seq[m][t].obs, states[j].mixtures[k].means);
						else
							dist = -SLGaussPDF(&states[j].mixtures[k], m_seq[m][t].obs);
						if(dist < min_dist)
						{
							min_dist = dist;
							min_index = k;
						}
					}

					// If a point changed its cluster from from prev. iteration, update and mark as changed
					if(k_index[m][t] != min_index)
					{
						bChanged = true;
						k_index[m][t] = min_index;
					}

					// Update counter for new means AND new variances
					for(int d = 0; d < numDim; d++)
					{
						new_means[min_index][d] += m_seq[m][t].obs[d];
						new_vars[min_index][d] += (m_seq[m][t].obs[d] * m_seq[m][t].obs[d]);
					}
					// Update # of points per cluster
					num_index[min_index]++;
				}
			}

			// Update the means to the center of each cluster
			for(int k = 0; k < K; k++)
			{
				if(num_index[k] == 0)
				{
					if(!bChanged)
						printf("KMEANS: Zero occupancy in state: %d, mixture %d\n", j+1, k+1); // Reinitialize clusters at this point??
					continue;
				}

				for(int d = 0; d < numDim; d++)
				{
					FLOAT64 mean = new_means[k][d] / num_index[k];
					FLOAT64 var = new_vars[k][d] / num_index[k] - (mean * mean);

					states[j].mixtures[k].means[d] = mean;
					states[j].mixtures[k].vars[d] = var > vFloor[d] ? var : vFloor[d];
					//states[j].mixtures[k].vars[d] = s_var[j][d];
					//printf("  State %d, Mix %d, Mean %d = %f\n", j, k, d, states[j].mixtures[k].means[d]);
				}
			}
		}

		//printf("K-Means converged in %d iterations for state %d.\n", k_iter, j+1);

		// Initialize priors based on results from K-Means
		for(int k = 0; k < K; k++)
			states[j].weights[k] = num_index[k] / (FLOAT64)s_sum[j];


		for(int k = 0; k < K; k++)
			delete[] new_means[k];

		delete[] new_means;
		delete[] num_index;
	}

	for(int m = 0; m < i16_num_seq; m++)
		delete[] k_index[m];
	delete[] k_index;


	return false;
}


FLOAT64 HmmNetwork::ComputeChiSquared(INT16 j)
{
	FLOAT64 chi = 0;
	INT16   num_bins = 10;

	INT16* hist[2];
	hist[0] = new INT16[num_bins];
	hist[1] = new INT16[num_bins];

	for(int d = 0; d < numDim; d++)
	{
		memset(hist[0], 0, num_bins * sizeof(INT16));
		memset(hist[1], 0, num_bins * sizeof(INT16));

		FLOAT64 stdev = sqrt(s_var[j][d]);

		for(int m = 0; m < i16_num_seq; m++)
		{
			int start = segment[m][j];
			int end = start + (segment[m][j+1] - start) / 2;

			for(int h = 0; h < 2; h++)
			{
				for(int t = start; t < end; t++)
				{
					// Assign point to correct bin
					FLOAT64 n = (m_seq[m][t].obs[d] - s_mean[j][d]) / stdev + 2.5;
					int bin = floor(n / 5.0 * num_bins + 0.5);
					if(bin < 0)
						bin = 0;
					else if(bin >= num_bins)
						bin = num_bins-1;

					hist[h][bin]++;
				}

				start = end;
				end = segment[m][j+1];
			}
		}

		for(int b = 0; b < num_bins; b++)
		{
			FLOAT64 sum = hist[0][b] + hist[1][b];
			if(sum)
			{
				FLOAT64 diff = hist[0][b] - hist[1][b];
				chi += (diff * diff) / sum;
			}
		}
	}

	delete[] hist[0];
	delete[] hist[1];

	// This normalization ensures that 0 <= chi <= 1
	return (chi / (s_sum[j] * numDim));
}

//-----------------------------------------------------------------------------
// Name: HmmNetwork::InitModel()
// Desc: Initializes the model for training. Performs segmentation & k-means
//       clustering to determine initial means & vars
//-----------------------------------------------------------------------------
void HmmNetwork::InitModel()
{
	printf("Init Model\n");
	if(i16_num_seq == 0 || nStates == 0 || numDim == 0)
	{
		printf("ERROR: Init model failed. # Seq: %d, # States: %d, # Dim: %d\n", i16_num_seq, nStates, numDim);
		return;
	}

	segment = new INT16*[i16_num_seq+1]; // Segmentation for each sequence
	s_sum   = new INT16[nStates];        // Number of points in each segment
	s_mean  = new FLOAT64*[nStates];
	s_var   = new FLOAT64*[nStates];
	vFloor = new FLOAT64[numDim];        // Variance floor for each dim.

	for(int j = 0; j < nStates; j++)
	{
		s_mean[j] = new FLOAT64[numDim];
		s_var[j] = new FLOAT64[numDim];
	}

	bool bSegmentChanged = true;
	int num_points = 0;
	FLOAT64 score = LOG_ZERO;
	FLOAT64 new_score = LOG_ZERO;
	FLOAT64 thresh = 0.001;


	// Begin with uniform segmentation of each sequence
	for(int m = 0; m < i16_num_seq; m++)
	{
		int T = m_seq[m].size();
		num_points += T;

		segment[m] = new INT16[nStates+1];

		for(int j = 0; j <= nStates; j++)
			segment[m][j] = (T * j) / nStates;
	}

	ComputeVarianceFloors(0.05);
	ComputeDurationStats();
	ComputeSegmentInfo();
	KMeans(true);


	/*	Procedure:
			1) Perform K-Means across corresponding segments of all sequences.
			2) Perform re-segmentation with Viterbi algorithm
			3) If segmentation has changed for any sequence, go to (1)       */

	int MAX_ITER = 100, iter = 0;
	bool doInit;


	while(bSegmentChanged && iter < MAX_ITER)
	{
		iter++;
		bSegmentChanged = false;

		doInit = false;

		// Compute gaussian constants
		ComputeGaussConst();

		// Viterbi segmentation
		new_score = ViterbiSegmentation();

		FLOAT64 diff = (score-new_score)/score;
		score = new_score;

		printf("Alignment score: %.1f (Improvement: %.2f%%)\n", score, diff*100.0);
		if(diff > thresh || diff < 0.0)
			bSegmentChanged = true;

		if(score <= LOG_ZERO)
		{
			printf("Initialization has failed. Aborting...\n");
			break;
		}

		if(verbLvl > 0)
		{
			for(int m = 0; m < i16_num_seq; m++)
			{
				printf("ALIGNMENT sequence %d: ", m+1);
				for(int j = 0; j <= nStates; j++)
					printf(" %d ", segment[m][j]);
				printf("\n");
			}
		}

		fflush(stdout);


		// Computes segmentation info (segment sums, variances, means)
		ComputeSegmentInfo();

		/* Remove dead states and split overextended states
		Procedure:
		   1) look for a dead state
		        NOTE: We consider a state dead if the number of points in it is less
				      than x% of the length of the sequence divided by number of states
		   2) do a chi-squared similarity test between two halves of each segment
		   3) remove the dead state and split the segment with highest dissimilarity
		*/
#if 1
		int threshold = (0.50 * num_points / nStates);
		for(int j = 0; j < nStates; j++)
		{
			//printf("Seg sum (%d): %d  -  tresh(%d) \n", j, s_sum[j], threshold);
			if(s_sum[j] < threshold)
			{
				printf("Removing dead state: %d\n", j+1);
				INT16 s = -1;
				FLOAT64 max_chi = 0;

				// Find the segment with highest dissimilarity
				for(int i = 0; i < nStates; i++)
				{
					// Ignore other dead states
					if(s_sum[i] < threshold * 2 + 1)
						continue;

					// Compute chi-squared test
					FLOAT64 chi = ComputeChiSquared(i);
					if(chi > max_chi)
					{
						s = i;
						max_chi = chi;
					}
					//printf("chi %d = %f\n", i+1, chi);
				}

				printf("Splitting state: %d\n", s+1);

				// j is the dead state s is the split state
				for(int m = 0; m < i16_num_seq; m++)
				{

					if(s< j)
					{
						memmove(&segment[m][s+2], &segment[m][s+1], (j-s-1) * sizeof(INT16));
						segment[m][s+1] = (segment[m][s+2] - segment[m][s]) / 2 + segment[m][s];
					}
					else
					{
						memmove(&segment[m][j+1], &segment[m][j+2], (s-j-1) * sizeof(INT16));
						segment[m][s] = (segment[m][s+1] - segment[m][s]) / 2 + segment[m][s];
					}
					if(verbLvl > 0)
					{
						printf("NEW sequence %d: ", m+1);
						for(int x = 0; x <= nStates; x++)
							printf(" %d ", segment[m][x]);
						printf("\n");
					}
				}

				// After splitting, recompute segment info
				ComputeSegmentInfo();

				bSegmentChanged = true;
				doInit = true;

			}
		}
#endif


		// K-Means
		KMeans(doInit);
	}

	if(verbLvl > 0)
	{
		for(int m = 0; m < i16_num_seq; m++)
		{
			printf("FINAL sequence %d: ", m+1);
			for(int j = 0; j <= nStates; j++)
				printf(" %d ", segment[m][j]);
			printf("\n");
		}
	}
}

// Function name   : HmmNetwork::ComputeVarianceFloors
// Description     : Variance floor will be set to percentage of the global variance
// Return type     : void
// Argument        : FLOAT64 coeff

void HmmNetwork::ComputeVarianceFloors(FLOAT64 coeff)
{
	FLOAT64* g_mean = new FLOAT64[numDim];

	memset(g_mean, 0, numDim * sizeof(FLOAT64));
	memset(vFloor, 0, numDim * sizeof(FLOAT64));

	int numPoints = 0;

	for(int m = 0; m < i16_num_seq; m++)
	{
		numPoints += m_seq[m].size();
		for(int t = 0; t < m_seq[m].size(); t++)
		{
			for(int d = 0; d < numDim; d++)
			{
				g_mean[d] += m_seq[m][t].obs[d];
				vFloor[d] += (m_seq[m][t].obs[d] * m_seq[m][t].obs[d]);
			}
		}
	}

	for(int d = 0; d < numDim; d++)
	{
		g_mean[d] /= numPoints;
		vFloor[d] /= numPoints;
		vFloor[d] -= (g_mean[d] * g_mean[d]);
		vFloor[d] *= coeff;
		printf("vFloor[%d]: %f\n", d+1, vFloor[d]);
	}

	delete[] g_mean;
}


// Function name   : HmmNetwork::ComputeSegmentInfo
// Description     :
// Return type     : void

void HmmNetwork::ComputeSegmentInfo()
{
	memset(s_sum, 0, nStates * sizeof(INT16));

	for(int j = 0; j < nStates; j++)
	{
		memset(s_mean[j], 0, numDim * sizeof(FLOAT64));
		memset(s_var[j], 0, numDim * sizeof(FLOAT64));

		int K = states[j].nMix;

		for(int m = 0; m < i16_num_seq; m++)
		{
			// Compute total number of points per state
			s_sum[j] += (segment[m][j+1] - segment[m][j]);

			for(int t = segment[m][j]; t < segment[m][j+1]; t++)
			{
				for(int d = 0; d < numDim; d++)
				{
					s_mean[j][d] += m_seq[m][t].obs[d];
					s_var[j][d] += (m_seq[m][t].obs[d] * m_seq[m][t].obs[d]);
				}
			}
		}

		for(int d = 0; d < numDim; d++)
		{
			s_mean[j][d] /= s_sum[j];
			s_var[j][d]  /= s_sum[j];
			s_var[j][d] -= (s_mean[j][d] * s_mean[j][d]);
		}
	}
}


// Function name   : HmmNetwork::ComputeDurationStats
// Description     : Computes duration mean and variance for all training sequences
// Return type     : void

void HmmNetwork::ComputeDurationStats()
{
	FLOAT64 mean = 0.0;
	FLOAT64 var = 0.0;

	FILE* f = fopen("durations.txt", "wt");

	for(int m = 0; m < i16_num_seq; m++)
	{
		mean += m_seq[m].size();
		var  += m_seq[m].size() * m_seq[m].size();

		fprintf(f, "%d\n", m_seq[m].size());
	}

	fclose(f);

	mean = mean / i16_num_seq;
	var = var / i16_num_seq;
	var = var - (mean * mean);

	model->lengthMean = mean;
	model->lengthVar = var;

}

// Function name   : HmmNetwork::PrintModelParams
// Description     :
// Return type     : void

void HmmNetwork::PrintModelParams()
{
	for(int j = 0; j < nStates; j++)
	{
		printf("\n  State %d\n", j+1);
		for(int k = 0; k < states[j].nMix; k++)
		{
			printf("    Mix %d Weight = %.2f\n", k+1, states[j].weights[k]);
			for(int d = 0; d < numDim; d++)
				printf("      Dim: %d Mean = %.3f  Var = %.3f\n", d+1, states[j].mixtures[k].means[d], states[j].mixtures[k].vars[d]);
		}

		printf("  Trans: (prior: %.3f)", (transProb[0][j+1]));
		for(int i = 0; i < nStates + 1; i++)
			printf(" %.3f ", (transProb[j+1][i+1]));

		printf("\n\n");
	}
}


// Function name   : HmmNetwork::ComputeGaussConst
// Description     :
// Return type     : void

void HmmNetwork::ComputeGaussConst()
{
	for(int j = 0; j < nStates; j++)
	{
		for(int k = 0; k < states[j].nMix; k++)
		{
			// Compute log determinant
			FLOAT64 det = 1.0;

			for(int d = 0; d < numDim; d++)
				det *= states[j].mixtures[k].vars[d];

			// Compute gauss constant for log gauss pdf
			states[j].mixtures[k].logGconst = log(det) + log(2.0 * PI) * numDim;
		}
	}
}


// Function name   : HmmNetwork::LogAdd
// Description     :
// Return type     : FLOAT64
// Argument        : FLOAT64 logX
// Argument        : FLOAT64 logY

FLOAT64 HmmNetwork::LogAdd(FLOAT64 logX, FLOAT64 logY)
{
	FLOAT64 diff, sum;

	if(logX > logY)
	{
		sum = logX;
		diff = logY - logX;
	}
	else
	{
		sum = logY;
		diff = logX - logY;
	}

	sum += Log(1.0 + Exp(diff));

	if(sum < LOG_SMALL)
		sum = LOG_ZERO;

	return sum;
}


// Function name   : HmmNetwork::ComputeLogTransP
// Description     :
// Return type     : void

void HmmNetwork::ComputeLogTransP()
{
	for(int i = 0; i < nStates + 2; i++)
	{
		for(int j = 0; j < nStates + 2; j++)
			logTransP[i][j] = Log(transProb[i][j]);
	}
}


// Function name   : HmmNetwork::AlignModel
// Description     :
// Return type     : void
// Argument        : char* filename

void HmmNetwork::AlignModel(char* filename)
{
	segment = new INT16*[i16_num_seq+1];		// Segmentation per sequence
	s_sum   = new INT16[nStates];		// Number of points in each segment
	s_mean  = new FLOAT64*[nStates];
	s_var   = new FLOAT64*[nStates];

	for(int j = 0; j < nStates; j++)
	{
		s_mean[j] = new FLOAT64[numDim];
		s_var[j] = new FLOAT64[numDim];
	}

	for(int m = 0; m < i16_num_seq; m++)
	{
		segment[m] = new INT16[nStates+1];
	}

	FLOAT64 score = ViterbiSegmentation();
	printf("Alignment score: %f\n", score);
	ComputeSegmentInfo();

	FILE* f = fopen(filename, "wb");

	// Write # of dim
	fwrite(&numDim, sizeof(INT16), 1, f);

	// Write # of states
	fwrite(&nStates, sizeof(INT16), 1, f);

	for(int j = 0; j < nStates; j++)
	{
		// Write # of obs for this state
		fwrite(&s_sum[j], sizeof(INT16), 1, f);

		for(int m = 0; m < i16_num_seq; m++)
		{
			for(int t = segment[m][j]; t < segment[m][j+1]; t++)
				fwrite(m_seq[m][t].obs, sizeof(FLOAT64), numDim, f);
		}
	}

	fclose(f);
}