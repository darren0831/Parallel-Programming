#include <iostream>
#include <mpi.h>
#include <stdlib.h>
#include <algorithm>

//basic
using namespace std;
char sorted=0;

int main(int argc,char *argv[])
{
    int numtasks,rank,op_check;

    int  N=atoi(argv[1]);
    char *in_file=argv[2];
    char *out_file=argv[3];
	
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Offset offset;
	MPI_Comm custom_world = MPI_COMM_WORLD;
	MPI_Group origin_group, new_group;
	
	if (N < numtasks) {
		MPI_Comm_group(custom_world, &origin_group);
		// remove unwanted ranks
		int ranges[][3] = {{N, numtasks-1, 1}};
		MPI_Group_range_excl(origin_group, 1, ranges, &new_group);
		// create a new communicator
		MPI_Comm_create(custom_world, new_group, &custom_world);
		if (custom_world == MPI_COMM_NULL) {
			// terminate those unwanted processes
			MPI_Finalize();
			exit(0);
		}
		numtasks = N;
	}
	
    //MPI open file
    MPI_File file_in,file_out;
    op_check=MPI_File_open(custom_world, in_file, MPI_MODE_RDONLY, MPI_INFO_NULL,&file_in);
	
    //check the file open state
	if (op_check!=MPI_SUCCESS){
		cout << "開檔失敗啦 嗚嗚" <<endl;
		MPI_Abort(custom_world,op_check);
	}

    int data_count=N/numtasks;
    int front=data_count*rank;
    int rear=front+data_count-1;
    
	//Assigning the remainder data to the last rank of processor
	if(rank == numtasks-1) {
		data_count+=(N % numtasks);
	}
	
	//put in the data to the local buffer
	float *local_buf=(float*)malloc(data_count*sizeof(MPI_FLOAT));
	offset=front*sizeof(MPI_FLOAT);
    MPI_File_read_at(file_in,offset,local_buf,data_count,MPI_FLOAT,MPI_STATUS_IGNORE);
	MPI_File_close(&file_in);
	
    while(!sorted){
            sorted=1;
            //odd phase LOCAL sorted
            if(front%2==0){
                    for(int i=1;i<data_count-1;i+=2){
                            if (local_buf[i]>local_buf[i+1]){
                                swap(local_buf[i],local_buf[i+1]);
                                sorted=0;
                            }
                    }
            }
            else{
                    for(int i=0;i<data_count-1;i+=2){
                            if(local_buf[i]>local_buf[i+1]){
                                swap(local_buf[i],local_buf[i+1]);
                                sorted=0;
                            }
                    }
            }
            //odd phase  communicate between two processors
			//rear send=10 front send 01
            if(rank!=0 && front%2==0){
                    float buffer;
                    MPI_Sendrecv(&local_buf[0], 1, MPI_FLOAT, rank-1, 0 , &buffer, 1 ,MPI_FLOAT , rank-1 , 1 ,custom_world, MPI_STATUS_IGNORE);
					if(local_buf[0]<buffer){
                            local_buf[0]=buffer;
                            sorted=0;
                    }
            }
            //MPI_Barrier(custom_world);
            if(rank!=(numtasks-1) && rear%2==1){
                    float buffer1;
					MPI_Sendrecv(&local_buf[data_count-1], 1, MPI_FLOAT, rank+1, 1, &buffer1, 1 ,MPI_FLOAT , rank+1 , 0 ,custom_world, MPI_STATUS_IGNORE);
                    if(local_buf[data_count-1]>buffer1){
                        local_buf[data_count-1]=buffer1;
                        sorted=0;
                    }
            }
            //MPI_Barrier(custom_world);
            //even phase local sorted part
            if(front%2==0){
                    for(int i=0;i<data_count-1;i+=2){
                            if(local_buf[i]>local_buf[i+1]){
                                swap(local_buf[i],local_buf[i+1]);
                                sorted=0;
                            }
                    }
            }
            else{
                     for(int i=1;i<data_count-1;i+=2){
                            if (local_buf[i]>local_buf[i+1]){
                                swap(local_buf[i],local_buf[i+1]);
                                sorted=0;
                            }
                    }
            }
            //even phase communicate between two processes
            if(rank!=0 && front%2==1){
                    float buffer;
					MPI_Sendrecv(&local_buf[0], 1, MPI_FLOAT, rank-1, 0 , &buffer, 1 ,MPI_FLOAT , rank-1 , 1 ,custom_world, MPI_STATUS_IGNORE);
					if(local_buf[0]<buffer){
                            local_buf[0]=buffer;
                            sorted=0;
                    }
            }
            //MPI_Barrier(custom_world);
            if(rank!=(numtasks-1) && rear%2==0){
                    float buffer1;
					MPI_Sendrecv(&local_buf[data_count-1], 1, MPI_FLOAT, rank+1, 1, &buffer1, 1 ,MPI_FLOAT , rank+1 , 0,custom_world, MPI_STATUS_IGNORE);
                    if(local_buf[data_count-1]>buffer1){
                        local_buf[data_count-1]=buffer1;
                        sorted=0;
                    }
            }
            //MPI_Barrier(custom_world);
            //sorted detection
            char tmp=sorted;
            MPI_Allreduce(&tmp, &sorted, 1,MPI_CHAR, MPI_BAND, custom_world);

    }
	
	MPI_File_open(custom_world, out_file, MPI_MODE_CREATE|MPI_MODE_WRONLY , MPI_INFO_NULL, &file_out);
    MPI_File_write_at(file_out , offset, local_buf , data_count, MPI_FLOAT, MPI_STATUS_IGNORE);
    MPI_File_close(&file_out);

    free(local_buf);
	
	MPI_Barrier(custom_world);
    MPI_Finalize();
    return 0;
}

