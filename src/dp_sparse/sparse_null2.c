/* null2 biased-composition model calculation, for sparse DP.
 */
#include "p7_config.h"

#include "easel.h"
#include "esl_vectorops.h"

#include "base/p7_profile.h"

#include "misc/logsum.h"

#include "dp_sparse/p7_sparsemx.h"
#include "dp_sparse/sparse_null2.h"

#if 0 /* during dev */

/* Function:  
 * Synopsis:  
 *
 * Purpose:   
 *
 * Args:      wrk : space for M+1 floats. wrk[0] = inserts; wrk[1..M] = match k=1..M
 *
 * Returns:   
 *
 * Throws:    (no abnormal error conditions)
 *
 * Xref:      
 */


int
p7_sparse_Null2(const P7_SPARSEMX *apd, const P7_ENVELOPES *env, float *wrk)
{
  const P7_SPARSEMASK *sm  = apd->sm;
  const float         *ppp = apd->dp;
  int                  M   = apd->M;
  int d,g,i,z;
  
  g = 0;
  for (d = 1; d <= env->D; d++)
    {
      /*****************************************************************
       * Because of anchor and envelope constrained sparsity,
       * mechanics of simply enumerating matrix cells ppp[i,k] are
       * heavy. Don't despair; all the following chunk of code does is to
       * count
       *    c_s = \sum_i=ia(d)..ib(d) P(\pi_i = s)
       * for each emitting state s: c_s counts are stored in wrk[].
       */
      /* wrk[0,1..M] accumulates expected usage count of any insert, match[1..M] for domain <d> */
      esl_vec_FSet(wrk, M+1, 0.0f);
      /* <ppp> maintenance (1): skip leading rows <i>, get <ppp> onto start of row <ia(d)> */
      while (env->arr[d].i0 > sm->seg[g].ib)  g++;                          // find which segment <g> that anchor <d> is in
      if (env->arr[d-1].i0 < sm->seg[g].ia)                                 // If <d> is 1st domain in seg <g>
	{                                                                   // then for ASC rows ia(g)..ia(d)-1, ASC is only in UP sector;
	  for (i = sm->seg[g].ia; i < env->arr[d].ia; i++)                  // advance <ppp> over UP(i) rows.
	    for (z = 0; z < sm->n[i] && sm->k[i][z] < env->arr[d].k0; z++)
	      ppp += p7S_NSCELLS;
	}
      else                                                                  // Else <d> isn't first,
	{                                                                   // and for ASC rows ib(d-1)..ia(d)-1, 
	  for (i = sm->arr[d-1].ib+1; i < env->arr[d].ia; i++)              // advance <ppp> over both DOWN(i) and UP(i) supercells.
	    {
	      for (z = sm->n[i]-1; z >= 0       && sm->k[i][z] >= env->arr[d-1].k0; z--)  // Order of access doesn't matter here; only # of sparse supercells in DOWN(i)
		ppp += p7S_NSCELLS;  
	      for (z = 0;          z < sm->n[i] && sm->k[i][z] <  env->arr[d].k0;   z++)  // UP row includes any supercells 1..k0(d)-1
		ppp += p7S_NSCELLS; 
	    }
	}
      
      /* UP sector ia(d)..i0(d)-1; 1..k0(d)-1 */
      for (i = env->arr[d].ia; i < env->arr[d].i0; i++)
	{
	  /* <ppp> maintenance (2): skip leading DOWN supercells when i in ASC DOWN+UP row */
	  if (env->arr[d-1].i0 >= sm->seg[g].ia)
	    for (z = sm->n[i]-1; z >= 0 && sm->k[i][z] >= env->arr[d-1].k0; z--)
	      ppp += p7S_NSCELLS;
	  
	  for (z = 0; z < sm->n[i] && sm->k[i][z] < env->arr[d].k0; z++, ppp += p7S_NSCELLS)
	    {
	      wrk[0]           += ppp[p7S_IL] + ppp[p7S_IG];
	      wrk[sm->k[i][z]] += ppp[p7S_ML] + ppp[p7S_MG];
	    }
	}

      /* DOWN sector i0(d)..ib(d); k0(d)..M */
      for (i = env->arr[d].i0; i <= env->arr[d].ib; i++)
	{
	  z = 0; while (z < sm->n[i] && sm->k[i][z] < env->arr[d].k0) z++;
	  for (; z < sm->n[i]; z++, ppp += p7S_NSCELLS)
	    {
	      wrk[0]           += ppp[p7S_IL] + ppp[p7S_IG];
	      wrk[sm->k[i][z]] += ppp[p7S_ML] + ppp[p7S_MG];
	    }

	  /* <ppp> maintenance (3); skip trailing UP supercells when i is ASC DOWN/UP row */
	  if (env->arr[d+1].i0 <= sm->seg[g].ib)
	    for (z = 0; z < sm->n[i] && sm->k[i][z] < env->arr[d+1].k0; z++)   
	      ppp += p7S_NSCELLS;
	}
	
      /* <ppp> maintenance (4): if <d> is last anchor in seg, 
       * skip trailing rows ib(d)+1..ib(g), which must be
       * DOWN only rows.
       */
      if (env->arr[d+1].i0 > sm->seg[g].ib)
	for (i = env->arr[d].ib+1; i <= sm->seg[g].ib; i++)
	  for (z = sm->n[i]-1; z >= 0 && sm->k[i][z] >= env->arr[d].k0; z--)  // DOWN is k0(d)..M; order doesn't matter, so we can skip it backwards.
	    ppp += p7S_NSCELLS;
      /* Heavyweight sparse AEC matrix traversal is now complete.
       * wrk[] contains expected usage counts.
       */

      
      /* null2[x] = \sum_s f_s [ e_s(x) / f(x) ]
       *          = \logsum_s log(f_s) + log [ e_s(x) / f(x) ]
       */
      esl_vec_FNorm(wrk, M+1);
      esl_vec_FLog (wrk, M+1);
      for (x = 0; x < gm->abc->K; x++)
	{
	  
	}



    } // end loop over domains <d>

}
#endif /* during dev */



/* Function:  p7_sparse_Null2ByExpectation()
 * Synopsis:  Calculate null2 lod scores, from sparse decoding matrix.
 *
 * Purpose:   Calculate $\log \frac{f'(x)}{f(x)}$, null2 lod scores for
 *            a given envelope, given a sparse posterior decoding matrix <sxd>, the 
 *            profile <gm>, and the envelope coords <iae,ibe,kae,kbe>.
 *            
 *            Requires a caller-allocated workspace <wrk> with room for at least
 *            <gm->M+1> floats.
 *            
 *            Stores the resulting lod scores in caller-allocated space <null2>,
 *            with room for at least <gm->abc->Kp> scores. 
 *            
 *            $f'(x)$ (the null2 emission probability for x) is defined (for <x=0..K-1>)
 *            the weighted average of the emission probabilities of profile states
 *            used to generate the envelope, weighted by the expected occurrence 
 *            frequency of those states (i.e. by using their posterior probabilities). 
 *            
 *            As for degenerate residues <x=K..Kp-1>: for residues (K+1..Kp-3), 
 *            f'(x) = the mean lod score over canonical residues included in the
 *            degeneracy. For not-residues (gap K, nonresidue *, missing data ~),
 *            set $\log \frac{f'(x)}{f(x)} = 0$, implying $f'(x)=f(x)$.
 *            
 *            This averaging behavior differs from H3.0, which averaged the odds
 *            ratios, not the log odds ratios; the difference is because of a switch
 *            to keeping null2[] in log space and saving some exponentiation calls.
 *            I don't believe that noncanonical residues have enough effect on the
 *            ad hoc null2[] calculation to make it worth worrying about the effect 
 *            of this change.
 *            
 * Args:      gm       - profile
 *            sxd      - sparse posterior probability matrix, already calculated
 *            iae,ibe  - envelope coords on sequence: 1..L
 *            kae,kbe  - envelope coords on profile: 1..M
 *            wrk      - caller provided tmp space for M+1 floats
 *            null2    - RETURN: caller provided space for null2[0..Kp-1] lod scores
 *            
 * Returns:   <eslOK> on success, and <null2> contains lod scores for a null2
 *            composition model.
 *
 * Throws:    (no abnormal error conditions)
 */
int
p7_sparse_Null2ByExpectation(const P7_PROFILE *gm, const P7_SPARSEMX *sxd, 
			     int iae, int ibe, int kae, int kbe,
			     float *wrk, float *null2)
{
  const P7_SPARSEMASK *sm = sxd->sm;
  float *xc  = sxd->xmx;
  float *dpc = sxd->dp;
  float const *rsc;
  int    i,k,x,z;

  /* Skip ahead in <sxd>, to get <dpc> and <xpc> on start of sparse row <iae>
   * (actually, the next row that's stored >= iae)
   */
  for (i = 1; i < iae; i++)
    if (sm->n[i]) {
      if (sm->n[i-1] == 0) xc += p7R_NSCELLS;
      dpc += sm->n[i] * p7R_NSCELLS;
      xc  += p7R_NXCELLS;
    }

  /* wrk[k] is the expected # of uses of Mk (ML|MG). 
   * wrk[kae-1] is the expected # of uses of Ik (IL|IG) and NCJ emission.
   * Collect it, by summation over sparse posterior prob matrix <sxd>
   */
  esl_vec_FSet(wrk+kae-1, kbe-kae+2, 0.0f);
  for (i = iae; i <= ibe; i++)
    {
      if (sm->n[i])
	{
	  if (sm->n[i-1] == 0) xc += p7R_NSCELLS;

	  z = 0;
	  while (z < sm->n[i] && sm->k[i][z] <  kae) z++; 
	  for (; z < sm->n[i] && sm->k[i][z] <= kbe; z++) 
	    {
	      wrk[sm->k[i][z]] += dpc[p7S_ML] + dpc[p7S_MG];
	      wrk[kae-1]       += dpc[p7S_IL] + dpc[p7S_IG]; 
	      dpc += p7S_NSCELLS;
	    }
	  wrk[kae-1] += xc[p7S_N] + xc[p7S_JJ] + xc[p7S_CC];
	  xc         += p7S_NXCELLS;
	}
      else wrk[kae-1] += 1.;
    }

  /* Normalize those expected usage #'s to frequencies.
   * We use those as weights, to calculate null2[x] as 
   * a weighted average of the states used to generate the
   * iae..ibe envelope.
   * If kae..kbe=1..M, then \sum_{k=0}^{M} = Ld = (ibe-iae+1);
   * but because there might be posterior probability outside
   * the kae..kbe envelope, \sum_{k=0}^{M} <= Ld
   */
  esl_vec_FNorm(wrk+kae-1, kbe-kae+2);
  esl_vec_FLog (wrk+kae-1, kbe-kae+2);
  
  /* Collect null2's emission odds: 
   *  null2[x] = \sum_k wrk[k] [e(k,x) / f(x)]
   * but we do it in log space, because that's what the profile's emission scores are:
   *  log null2[x] = \logsum_k log wrk[k] + log [e(k,x)/f(x)]
   */
  for (x = 0; x < gm->abc->K; x++)
    {
      null2[x] = wrk[kae-1];	               /* wrk[0] * 1.0 : emission odds ratio for N,C,J,I emissions is 1.0 */
      rsc = gm->rsc[x] + p7P_NR * kae + p7P_M; /* initialize <rsc> ptr on MSC(kae) */
      for (k = kae; k <= kbe; k++)
	{
	  null2[x] = p7_FLogsum(null2[x], wrk[k] + *rsc); /* this is wrk[k] + MSC(k) for residue x: stepping thru rsc for efficiency */
	  rsc     += p7P_NR;
	}
    }
  /* Now null2[x] = \log \frac{f'(x)}{f(x)} for all canonical x=0..K-1 */
  
  /* make valid scores for all degeneracies, by averaging the odds ratios. */
  esl_abc_FAvgScVec(gm->abc, null2); /* does not set gap, nonres, missing  */ // note 3.0 averaged odds ratios; 3.1 averages lod scores
  null2[gm->abc->K]    = 0.0;        /* gap character    */
  null2[gm->abc->Kp-2] = 0.0;	     /* nonresidue "*"   */
  null2[gm->abc->Kp-1] = 0.0;	     /* missing data "~" */
  return eslOK;
}
      

/*****************************************************************
 * @LICENSE@
 * 
 * SVN $Id$
 * SVN $URL$
 *****************************************************************/

  
