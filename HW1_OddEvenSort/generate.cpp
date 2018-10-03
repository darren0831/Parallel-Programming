#include <cstdlib>
#include <cstdio>

using namespace std;

int main(int argc, char *argv[]){
    FILE * pFile;
    pFile = fopen ("mytestcase", "wb");
    float *buff = (float *)malloc(sizeof(float) * 332640000);
    for(int i=0; i<332640000; ++i)
        buff[i] = (float)( (float)332640000 - (float)i );
        //buff[i] = (float)(i+1);

    fwrite(buff , sizeof(float), 332640000, pFile);
    fclose(pFile);
}
