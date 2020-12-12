#include <unistd.h>
#include "mphf.h"

uint8_t *find_solution_external(cnf_t *pCNF, uint32_t nNumVariables, char solver_string[256]) {
  uint32_t i, j;
  FILE *satfile;
  char satfilename_tmp[256];
  snprintf(satfilename_tmp, 256, "%s_in.cnf", tmpnam(NULL));
  
  satfile = fopen(satfilename_tmp, "wb+");
  if(satfile == NULL) return NULL;

  //print header
  fprintf(satfile, "p cnf %u %lu", nNumVariables, pCNF->nLength);

  //print clauses
  for(i = 0; i < pCNF->nLength; i++) {
    fprintf(satfile, "\n");
    for(j = 0; j < pCNF->pList[i].nLength; j++) {
      fprintf(satfile, "%d ", pCNF->pList[i].pList[j]);
    }
    fprintf(satfile, "0");
  }

  fclose(satfile);
    
  FILE *outfile;
  char outfilename_tmp[256];
  snprintf(outfilename_tmp, 256, "%s_out.out", tmpnam(NULL));

  char command[1024];
  snprintf(command, 1024, "%s < %s > %s", solver_string, satfilename_tmp, outfilename_tmp);

#ifdef MPHF_PRINT_BUILD_PROCESS
  fprintf(stderr, "command = %s\n", command);
#endif
  
  int success = system(command);
  if(success == 32512) {
    fprintf(stderr, "solver not found in path...exiting\n");
    unlink(satfilename_tmp);
    unlink(outfilename_tmp);
    return NULL;
  } else if(WIFSIGNALED(success)) {
    fprintf(stderr, "Exited with signal %d\n", WTERMSIG(success));
    unlink(satfilename_tmp);
    unlink(outfilename_tmp);
    exit(WTERMSIG(success));
  } else if(WEXITSTATUS(success) == 40) {
    return NULL;
  } else if(WIFEXITED(success)) {
    //Successful run
  } else {
    printf("Not sure how we exited.\n");
    unlink(satfilename_tmp);
    unlink(outfilename_tmp);
    exit(WEXITSTATUS(success));
  }

  unlink(satfilename_tmp);
  
  if((outfile = fopen(outfilename_tmp, "rb")) == NULL) {
    fprintf(stderr, "Can't open '%s' for reading", outfilename_tmp);
    return NULL;
  }

  char a = getc(outfile);
  while(!feof(outfile) && a != 's') {
    do { //skip to the next newline
      if(a != '\n') a = getc(outfile);
    } while(a != '\n' && !feof(outfile));
    if(feof(outfile)) {
      fprintf(stderr, "EOF hit and 's' not found\n");
      fclose(outfile);
      unlink(outfilename_tmp);
      return NULL;
    }
    a = getc(outfile);
  }
  
  char answer[64];
  int32_t err = fscanf(outfile, "%64s", answer);
  if(err != 1) {
    fprintf(stderr, "Problem with result...\n");
    fclose(outfile);
    unlink(outfilename_tmp);
    return NULL;
  }

  if(!strncmp(answer, "UNSATISFIABLE", 13)) {
#ifdef MPHF_PRINT_BUILD_PROCESS
    fprintf(stderr, "SAT formula is unsatisfiable\n");
#endif
    fclose(outfile);
    unlink(outfilename_tmp);
    return NULL;
  }

  if(strncmp(answer, "SATISFIABLE", 11)) {
    fprintf(stderr, "Problem with result...\n");
    fclose(outfile);
    unlink(outfilename_tmp);
    return NULL;
  }
  
  assert(a == 's');
  rewind(outfile);
  while(!feof(outfile) && a != 'v') {
    do { //skip to the next newline
      a = getc(outfile);
    } while(a != '\n' && !feof(outfile));
    if(feof(outfile)) {
      fprintf(stderr, "EOF hit and 'v' not found\n");
      fclose(outfile);
      unlink(outfilename_tmp);
      return NULL;
    }
    a = getc(outfile);
  }

  uint8_t *solution = malloc(sizeof(uint8_t) * (nNumVariables+1));
  
  for(i = 0; i <= nNumVariables; i++) {
    solution[i] = 0; //Assign unassigned variables to 0,
  }

  for(i = 0; i < nNumVariables; i++) {
    int32_t lit;
    err = fscanf(outfile, "%d", &lit);
    if(err == 0) {
      a = getc(outfile);
      if(a == 'v') {
	i--;
	continue;
      } else {
	fprintf(stderr, "Full solution not found...assigning %u unset variables\n", nNumVariables - i);
	break;
      }
    }
    if(err != 1) {
      fprintf(stderr, "Full solution not found...assigning %u unset variables\n", nNumVariables - i);
      break;
    }
    uint32_t var = abs(lit);
    if(var > nNumVariables) {i--; continue;}
    solution[var] = lit > 0;
  }

  fclose(outfile);
  unlink(outfilename_tmp);

#ifdef MPHF_PRINT_BUILD_PROCESS
  for(i = 1; i <= nNumVariables; i++)
    fprintf(stderr, "%u", solution[i]);
  fprintf(stderr, "\n");
#endif
  
  return solution;
}
