#include <iostream>
#include <mpi.h>
#include <algorithm>
#include <cmath>
#define flag_0 0
#define flag_1 1
#define flag_2 2
#define flag_3 3

using namespace std;

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
	double ct = 0, cta = 0;
	double cst = 0;
	double cet = 0;
	int load_r,load_l;
	
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
	int front=data_count*rank;
	int Max_count=data_count, min_count=data_count;
	
	
	
	if(front >= N)data_count=0;// those processor don't exist any data
	if(rank==numtasks-1 && data_count) {
		data_count=N-data_count*rank;//the last one
	}
	if(rank==numtasks-2 && (N-data_count*(rank+1))){
		min_count=N-data_count*(rank+1);
	}
	//put in the data to the local buffer
	float *local_buf=new float[data_count]();    //(float*)malloc(data_count*sizeof(MPI_FLOAT));
	float *recv_buf=new float[Max_count]();      //(float*)malloc(Max_count*sizeof(MPI_FLOAT));
	float *out=new float[Max_count+data_count]();//(float*)malloc((Max_count+data_count)*sizeof(MPI_FLOAT));
	
	
	
	offset=front*sizeof(MPI_FLOAT);
	//double IO_start_time_1 = MPI_Wtime();
	MPI_File_read_at(file_in,offset,local_buf,data_count,MPI_FLOAT,MPI_STATUS_IGNORE);
	MPI_File_close(&file_in);
	//double IO_end_time_1 = MPI_Wtime();
	/*
	FILE * fp;
	fp = fopen(argv[2],"rb");
	float *buffer = new float[N];
	//double IO_start_time_1 = MPI_Wtime();
	fread(buffer, sizeof(float), N, fp);
	for(int i=0; i<data_count; i++){
		local_buf[i] = buffer[front+i];
	}
	fclose(fp);
	//double IO_end_time_1 = MPI_Wtime();
	*/
	//sort the fucking local_buf & recording the min and max value of buffer
	sort(local_buf,local_buf+data_count);
	//min_v=local_buf[0];max_v=local_buf[data_count-1];//try try see 
	
	for(int i=1 ; i<=numtasks ; i++){
			//odd phase send to rank+1 recv from rank+1
		if((rank != numtasks-1 )&&((rank + i) % 2==0)){
			cst = MPI_Wtime();
			MPI_Sendrecv(local_buf,data_count,MPI_FLOAT,rank+1,flag_2,recv_buf,min_count,MPI_FLOAT,rank+1,flag_3,custom_world,MPI_STATUS_IGNORE);
			cet = MPI_Wtime();
			merge2(local_buf, data_count, recv_buf ,min_count , out);
			for(int j=0; j<data_count ;j++)local_buf[j]=out[j];
		}
		//EVEN PHASE send to rank-1 recv from rank-1
		else if (((rank + i) % 2==1)&&rank){
			//if(rank==numtasks-2)recv_count=data_count;
			cst = MPI_Wtime();
			MPI_Sendrecv(local_buf,data_count,MPI_FLOAT,rank-1,flag_3,recv_buf,Max_count,MPI_FLOAT,rank-1,flag_2,custom_world,MPI_STATUS_IGNORE);
			cet = MPI_Wtime();
			merge1(local_buf, data_count, recv_buf ,Max_count , out,rank);
			for(int j=0; j<data_count;j++)local_buf[j]=out[j];
		}
		MPI_Barrier(custom_world);
		ct += cet - cst;	
	}
	delete[] recv_buf;
	delete[] out;
	//free(recv_buf);
	//free(out);
	
	
	MPI_File_open(custom_world, out_file, MPI_MODE_CREATE|MPI_MODE_WRONLY , MPI_INFO_NULL, &file_out);
	//double IO_start_time_2 = MPI_Wtime();
	MPI_File_write_at(file_out , offset, local_buf , data_count, MPI_FLOAT, MPI_STATUS_IGNORE);
    MPI_File_close(&file_out);
	//double IO_end_time_2 = MPI_Wtime();
	
	delete[] local_buf;
	//free(local_buf);
	
	MPI_Barrier(custom_world);
	double end_time = MPI_Wtime();
	MPI_Allreduce(&ct, &cta, 1, MPI_DOUBLE, MPI_SUM,custom_world );
	//double IO_R = IO_end_time_1-IO_start_time_1;
	//double IO_W = IO_end_time_2-IO_start_time_2;
	//double IO=IO_R+IO_W;
	//double ioa;
	//MPI_Allreduce(&IO, &ioa, 1, MPI_DOUBLE, MPI_SUM,custom_world );
    if(rank==0){
	cout<<"-------------------------result-------------------"<<endl;
	cout<<"total exec. time = "<< end_time-start_time <<endl;
	//cout<<"I/O readtime= "<< IO_R <<"    I/O writetime= "<< IO_W <<endl;
	//cout<<"Total I/O time = "<<IO_R+IO_W<< endl;
	//cout<<"avg I/O time = "<<ioa/(float)numtasks<< endl;
	cout<<"Communication time (rank0) = "<< ct << endl;
	cout<<"Communication time (Avg.) = " << cta/(float)numtasks << endl;
	}
	if(rank==numtasks-1)cout<<"Communication time (rank last) = "<< ct << endl;
	MPI_Finalize();
    return 0;	
}
