#include <stdio.h>
#include <stdlib.h>


/* ./Decoder [input file]    */

int main(int argc, char* argv[]) {
  FILE * pFile;
  long lSize;
  float * buffer;
  size_t result;

  pFile = fopen ( argv[1] , "rb" );
  if (pFile==NULL) {fputs ("File error",stderr); exit (1);}

  // obtain file size:
  fseek (pFile , 0 , SEEK_END);
  lSize = ftell (pFile) / (long)(sizeof(float));
  //printf("%d\n", lSize);
  rewind (pFile);

  // allocate memory to contain the whole file:
  buffer = (float*) malloc (sizeof(float)*lSize);
  if (buffer == NULL) {fputs ("Memory error",stderr); exit (2);}

  // copy the file into the buffer:
  result = fread (buffer,sizeof(float),lSize,pFile);
  if (result != lSize) {fputs ("Reading error",stderr); exit (3);}

  /* the whole file is now loaded in the memory buffer. */
  for(long i=0; i<lSize; ++i){
    printf("%f\n", buffer[i]);
  }
  printf("size = %d\n", lSize);
  

  // terminate
  fclose (pFile);


  return 0;
}
