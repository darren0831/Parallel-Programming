#include <iostream>
#include <mpi.h>
#include <algorithm>
#include <cmath>
#define flag_0 0
#define flag_1 1
#define flag_2 2
#define flag_3 3

using namespace std;

/*
void merge(float *local_buf, int count, float *recv_buf,int recv_count, float *out){
	int outcount=0,i=0,j=0;
	while(i!=count && j!=recv_count){
		//local_buf and recv_buf haven't finished to scan yet
		if(local_buf[i]<recv_buf[j])out[outcount++]=local_buf[i++];
		else out[outcount++]=recv_buf[j++];
	}
	while(i!=count)out[outcount++]=local_buf[i++];
	while(j!=recv_count)out[outcount++]=recv_buf[j++];
	return;
}
*/

//get  those bigger data
void merge1(float *local_buf,int count,float *recv_buf, int recv_count, float *out,int rank_num){
		int outcount=count,i=count,j=recv_count;
		while(outcount && i && j){
				if(local_buf[i-1]>recv_buf[j-1])out[--outcount]=local_buf[--i];
				else out[--outcount]=recv_buf[--j]; 				
		}
return;
}
//get those smaller data
void merge2(float *local_buf,int count,float *recv_buf, int recv_count, float *out){
		int outcount=0,i=0,j=0;
		while(outcount!=count){
			if(i!=count && j!=recv_count){
				if(local_buf[i]<recv_buf[j])out[outcount++]=local_buf[i++];
				else out[outcount++]=recv_buf[j++]; 				
			}
			else if (j==recv_count && i!=count )out[outcount++]=local_buf[i++];
		}
return;
}

int main(int argc,char *argv[])
{
	int numtasks,rank,op_check;
   
	int  N=atoi(argv[1]);
	char *in_file=argv[2], *out_file=argv[3];
	
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
	
    //MPI open file
    MPI_File file_in,file_out;
    op_check=MPI_File_open(custom_world, in_file, MPI_MODE_RDONLY, MPI_INFO_NULL,&file_in);
    
	//check the file open state
	if (op_check!=MPI_SUCCESS){
		cout << "NOOOOOO YOU SUCK!" <<endl;
		MPI_Abort(custom_world,op_check);
	}
	
	//float min_v,max_v;
	int data_count=(int)((ceil)((double)((double)N/(double)numtasks)));
	//int recv_count=data_count;
	int front=data_count*rank;
	int Max_count=data_count, min_count=data_count;
	if(front >= N)data_count=0;// those processor don't exist any data
	if(rank==numtasks-1 && data_count) data_count=N-data_count*rank;//the last one
	if(rank==numtasks-2 && (N-data_count*(rank+1)))min_count=N-data_count*(rank+1);
	//int Max_count=data_count+(N % numtasks);
	//if(rank == numtasks-1)data_count=Max_count;//Assigning the remainder data to the last rank of processor
	
	//put in the data to the local buffer
	float *local_buf=new float[data_count]();    //(float*)malloc(data_count*sizeof(MPI_FLOAT));
	float *recv_buf=new float[Max_count]();      //(float*)malloc(Max_count*sizeof(MPI_FLOAT));
	float *out=new float[Max_count+data_count]();//(float*)malloc((Max_count+data_count)*sizeof(MPI_FLOAT));
	
	offset=front*sizeof(MPI_FLOAT);
	MPI_File_read_at(file_in,offset,local_buf,data_count,MPI_FLOAT,MPI_STATUS_IGNORE);
	MPI_File_close(&file_in);
	
	//sort the fucking local_buf & recording the min and max value of buffer
	sort(local_buf,local_buf+data_count);
	//min_v=local_buf[0];max_v=local_buf[data_count-1];//try try see 

	for(int i=1 ; i<=numtasks ; i++){
			//odd phase send to rank+1 recv from rank+1
		if((rank != numtasks-1 )&&((rank + i) % 2==0)){
			//if(rank==numtasks-2)recv_count=Max_count;
			MPI_Sendrecv(local_buf,data_count,MPI_FLOAT,rank+1,flag_2,recv_buf,min_count,MPI_FLOAT,rank+1,flag_3,custom_world,MPI_STATUS_IGNORE);
			merge2(local_buf, data_count, recv_buf ,min_count , out);
			for(int j=0; j<data_count ;j++)local_buf[j]=out[j];
		}
		//EVEN PHASE send to rank-1 recv from rank-1
		else if (((rank + i) % 2==1)&&rank){
			//if(rank==numtasks-2)recv_count=data_count;
			MPI_Sendrecv(local_buf,data_count,MPI_FLOAT,rank-1,flag_3,recv_buf,Max_count,MPI_FLOAT,rank-1,flag_2,custom_world,MPI_STATUS_IGNORE);
			merge1(local_buf, data_count, recv_buf ,Max_count , out,rank);
			for(int j=0; j<data_count;j++)local_buf[j]=out[j];
			//for(int j=0; j<data_count ;j++)local_buf[j]=out[Max_count+j];	
			//if(rank==numtasks-1)for(int j=0; j<data_count ;j++)local_buf[j]=out[recv_count+j];	
			//for(int j=0; j<data_count ;j++)local_buf[j]=out[data_count+j];
		}
		//MPI_Barrier(custom_world);		
	}
	delete[] recv_buf;
	delete[] out;
	
	MPI_File_open(custom_world, out_file, MPI_MODE_CREATE|MPI_MODE_WRONLY , MPI_INFO_NULL, &file_out);
    MPI_File_write_at(file_out , offset, local_buf , data_count, MPI_FLOAT, MPI_STATUS_IGNORE);
    MPI_File_close(&file_out);
	
	delete[] local_buf;
	
	MPI_Barrier(custom_world);
	double end_time = MPI_Wtime();
	if(rank==0)cout<<"total time :"<<end_time-start_time<<endl;
    MPI_Finalize();
    return 0;	
}
