#include "mphf.h"

create_c_list_type(clause_t, int8_t)
create_c_list_type(cnf_t, clause_t)

/*  nExpectedElements can be 0 */
MPHFBuilder *MPHFBuilderAlloc(uint32_t nExpectedElements) {
  MPHFBuilder *mphfb = (MPHFBuilder *)malloc(1 * sizeof(MPHFBuilder));
  if(mphfb == NULL) return NULL;

  if(MPHFHash_list_init(&mphfb->pHashes, nExpectedElements) != C_LIST_NO_ERROR) {
    free(mphfb);
    return NULL;
  }
 
  return mphfb;
}

void MPHFBuilderFree(MPHFBuilder *mphfb) {
  MPHFHash_list_free(&mphfb->pHashes, NULL);
  free(mphfb);
}

//Can be used to build with externally hashed elements
uint8_t MPHFBuilderAddHash(MPHFBuilder *mphfb, MPHFHash pHash) {
  return MPHFHash_list_push(&mphfb->pHashes, pHash);
}

uint8_t MPHFBuilderAddElement(MPHFBuilder *mphfb, const void *pElement, size_t nElementBytes) {
  MPHFHash pHash = MPHFGenerateHashesFromElement(pElement, nElementBytes);
  return MPHFBuilderAddHash(mphfb, pHash);
}

double MPHFCalculateBound(uint32_t nNumElements) {
  uint32_t i;
  long double n = (long double) nNumElements;
  long double s = 1.0, t = 0.0;
  
  for (i = 1; i <= nNumElements; i++) {
    s *= n / (long double) i;
    while (s > 100) { t += 5; s /= 32; }
  }
  
  s = log(s) / log (2);
  
  return (s+t) / n;
}

void removeDups (clause_t *clause) {
  uint32_t i, j;
  for (i = 0; i < clause->nLength - 1; i++) {
    for (j = i+1; j < clause->nLength; j++) {
      //Duplicate literal
      if (clause->pList[i] == clause->pList[j]) {
        clause->pList[j--] = clause->pList[--clause->nLength];
      }
    }
  }
}

uint8_t isTautology (clause_t clause) {
  uint32_t i, j;
  for (i = 0; i < clause.nLength-1; i++) {
    for (j = i+1; j < clause.nLength; j++) {
      //Tautology
      if (clause.pList[i] == -clause.pList[j]) {
        return 1;
      }
    }
  }

  return 0;
}

void compareAndAppend (cnf_t *pCNF, clause_t x, clause_t y, uint32_t n) {
  uint32_t i, j, l = 0, v = 0;
  uint32_t k = x.nLength;
  if (n != (1<<k)) { v = 1<<(k-1); l = n - v; }

  for (i = 0; i < n; i++) {
    uint32_t b = k; if ((i >= l) && (i < v)) b--;

    clause_t clause;
    clause_t_init(&clause, 2*b);
    clause.nLength = 2*b;
    
    for (j = 0; j < b; j++) {
      uint8_t pos = (i>>j) & 1;
      if (pos) {
        clause.pList[j*2  ] = -x.pList[j];
        clause.pList[j*2+1] = -y.pList[j]; }
      else {
        clause.pList[j*2  ] =  x.pList[j];
        clause.pList[j*2+1] =  y.pList[j]; }
    }

    if (!isTautology (clause)) {
      removeDups (&clause);
      cnf_t_push(pCNF, clause);
    } else {
      clause_t_free(&clause, NULL);
    }
  }
}

MPHFQuerier *MPHFBuilderFinalize(MPHFBuilder *mphfb, MPHFParameters params) {
  uint32_t i, j;

  uint32_t m = mphfb->pHashes.nLength;
  uint32_t n = (uint32_t) ceil(((double) m) * params.fBitsPerElement);
  uint32_t k = 3;
  uint32_t nNumUNSATCalls = 0;
  
  while (m > (1<<k)) k++;
  
  uint8_t *pSolution = NULL;
  while(pSolution == NULL) {
    cnf_t *pVectors = cnf_t_alloc(m);
    for(i = 0; i < m; i++) {
      clause_t_init(&pVectors->pList[i], k);
      MPHFGenerateVectorFromHash(mphfb->pHashes.pList[i], &pVectors->pList[i], n, nNumUNSATCalls);
    }
    
    cnf_t *pCNF = cnf_t_alloc(m*m);
    
    for (i = 0; i < m; i++)
      for (j = i + 1; j < m; j++)
        compareAndAppend(pCNF, pVectors->pList[i], pVectors->pList[j], m);
    
    //Force unused literals to false
    uint8_t *used = (uint8_t *)calloc (sizeof(uint8_t), n+1);
    for (i = 0; i < pCNF->nLength; i++)
      for (j = 0; j < pCNF->pList[i].nLength; j++)
        used[abs(pCNF->pList[i].pList[j])] = 1;
    
    for (i = 1; i <= n; i++) {
      if (!used[i]) {
        clause_t unit_clause;
        clause_t_init(&unit_clause, 1);
        unit_clause.nLength = 1;
        unit_clause.pList[0] = -i;
        cnf_t_push(pCNF, unit_clause);
      }
    }

    free(used);
    
    pSolution = find_solution_external(pCNF, n, params.solver_string);

    for(i = 0; i < m; i++)
      clause_t_free(&pVectors->pList[i], NULL);
    cnf_t_free(pVectors, NULL);
    free(pVectors);
    
    for(i = 0; i < pCNF->nLength; i++)
      clause_t_free(&pCNF->pList[i], NULL);
    cnf_t_free(pCNF, NULL);
    free(pCNF);
    
    if(pSolution == NULL) {
      nNumUNSATCalls++;
    }
  }
  
  return MPHFCreateQuerierFromBuilder(mphfb, pSolution, n, nNumUNSATCalls);
}
