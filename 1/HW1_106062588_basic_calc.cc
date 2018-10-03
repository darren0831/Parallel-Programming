#include <unistd.h>
#include <cstdio>
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
	double ct=0;
	double ct_all;
	
    MPI_Init(&argc,&argv);
	double start_time = MPI_Wtime();
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
	
	//file open
    MPI_File file_in,file_out;
    
	op_check=MPI_File_open(custom_world, in_file, MPI_MODE_RDONLY, MPI_INFO_NULL,&file_in);
	//check the file open state
	
	if (op_check!=MPI_SUCCESS){
		cout << "open fail~" <<endl;
		MPI_Abort(custom_world,op_check);
	}

    int data_count=N/numtasks;
    int front=data_count*rank;
    int rear=front+data_count-1;
    
	
	//Assigning the remainder data to the last rank of processor
	if(rank == numtasks-1 ) {
		data_count+=(N % numtasks);
	}
	/*
	if(front >= N)data_count=0;			// those processor don't exist any data
	if(rank==numtasks-1 && data_count) {
		data_count=N-data_count*rank;	//the last one
	}
	*/
	//put in the data to the local buffer
	float *local_buf=(float*)malloc(data_count*sizeof(MPI_FLOAT));
	offset=front*sizeof(MPI_FLOAT);
	
	double IO_start_time_1 = MPI_Wtime();
    MPI_File_read_at(file_in,offset,local_buf,data_count,MPI_FLOAT,MPI_STATUS_IGNORE);
	MPI_File_close(&file_in);
	double IO_end_time_1 = MPI_Wtime();
	
	/*
	FILE * fp;
	
	fp = fopen(argv[2],"rb");
	float *buffer = new float[N];
	double IO_start_time_1 = MPI_Wtime();
	fread(buffer, sizeof(float), N, fp);
	for(int i=0; i<data_count; i++){
		local_buf[i] = buffer[offset+i];
	}
	fclose(fp);
	double IO_end_time_1 = MPI_Wtime();
	*/
	while(!sorted){
	double cst_1 , cet_1;
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
            //sent front
			//rear send=10 front send 01
			if(rank!=0 && front%2==0){
                    // send the first one 
                    float buffer;
					cst_1 = MPI_Wtime();
                    MPI_Sendrecv(&local_buf[0], 1, MPI_FLOAT, rank-1, 0 , &buffer, 1 ,MPI_FLOAT , rank-1 , 1 ,custom_world, MPI_STATUS_IGNORE);
					cet_1 = MPI_Wtime();
					ct += (cet_1 - cst_1);
					if(local_buf[0]<buffer){
                            local_buf[0]=buffer;
                            sorted=0;
                    }
            }
            //MPI_Barrier(custom_world);
            if(rank!=(numtasks-1) && rear%2==1){
                    //send his mom 
                    float buffer1;
					cst_1 = MPI_Wtime();
					MPI_Sendrecv(&local_buf[data_count-1], 1, MPI_FLOAT, rank+1, 1, &buffer1, 1 ,MPI_FLOAT , rank+1 , 0 ,custom_world, MPI_STATUS_IGNORE);
                    cet_1 = MPI_Wtime();
					ct += (cet_1 - cst_1);
					if(local_buf[data_count-1]>buffer1){
                        local_buf[data_count-1]=buffer1;
                        sorted=0;
                    }
            }
            MPI_Barrier(custom_world);
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
                    // send the first one 
                    float buffer;
					cst_1 = MPI_Wtime();
					MPI_Sendrecv(&local_buf[0], 1, MPI_FLOAT, rank-1, 0 , &buffer, 1 ,MPI_FLOAT , rank-1 , 1 ,custom_world, MPI_STATUS_IGNORE);
					cet_1 = MPI_Wtime();
					ct += (cet_1 - cst_1);
					if(local_buf[0]<buffer){
                            local_buf[0]=buffer;
                            sorted=0;
                    }
            }
            //MPI_Barrier(custom_world);
            if(rank!=(numtasks-1) && rear%2==0){
                    //send his mom 
                    float buffer1;
					cst_1 = MPI_Wtime();
					MPI_Sendrecv(&local_buf[data_count-1], 1, MPI_FLOAT, rank+1, 1, &buffer1, 1 ,MPI_FLOAT , rank+1 , 0,custom_world, MPI_STATUS_IGNORE);
					cet_1 = MPI_Wtime();
					ct += (cet_1 - cst_1);
					if(local_buf[data_count-1]>buffer1){
                        local_buf[data_count-1]=buffer1;
                        sorted=0;
                    }
            }
			 
			MPI_Barrier(custom_world);
			
            char tmp=sorted;
            MPI_Allreduce(&tmp, &sorted, 1,MPI_CHAR, MPI_BAND, custom_world);

    }
	
	double IO_start_time_2 = MPI_Wtime();
	MPI_File_open(custom_world, out_file, MPI_MODE_CREATE|MPI_MODE_WRONLY , MPI_INFO_NULL, &file_out);
    MPI_File_write_at(file_out , offset, local_buf , data_count, MPI_FLOAT, MPI_STATUS_IGNORE);
    MPI_File_close(&file_out);
	double IO_end_time_2 = MPI_Wtime();

	
	MPI_Barrier(custom_world);
	MPI_Allreduce(&ct, &ct_all, 1, MPI_DOUBLE, MPI_SUM, custom_world);
	
	double end_time = MPI_Wtime();
	double IO_R = IO_end_time_1-IO_start_time_1;
	double IO_W = IO_end_time_2-IO_start_time_2;
	
	if(rank==0){
	cout<<"-------------------------result-------------------"<<endl;
	cout<<"total exec. time = "<< end_time-start_time <<endl;
	cout<<"I/O readtime= "<< IO_R <<"    I/O writetime= "<< IO_W <<endl;
	cout<<"Total I/O time = "<<IO_R+IO_W<< endl;
	cout<<"Communication time (rank0) = "<< ct << endl;
	cout<<"Communication time (Avg.) = " << (float)(ct_all/(float)numtasks) << endl;
	}
	if(rank==numtasks-1)cout<<"Communication time (rank last) = "<< ct << endl;
	
    MPI_Finalize();
    return 0;
}

