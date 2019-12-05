#include "udp.cpp"

int main(){

	timespec start, stop;
	double delta;
	bool recvd_any;

	UDPSet(true);

	thread t[RPI_NO];
	print_flag = true;

	while(1){

		// while loop exeution time measurement
		clock_gettime(CLOCK_MONOTONIC, &start);

		//if(print_flag) cout<<"\n---------- RECEIVE LOOP START ----------"<<endl;

	    // receive and store data in dataToProcess
		UDPRec();

		//if(print_flag) cout<<"\n----------  RECEIVE LOOP END  ----------"<<endl;

		char c=(char)waitKey(25);
		if(c==27)
			break;

		// while loop exeution time measurement
		clock_gettime(CLOCK_MONOTONIC, &stop);

		delta = ( stop.tv_sec - start.tv_sec )
             + (double)( stop.tv_nsec - start.tv_nsec )
               / (double)MILLION;

        if(print_flag) cout<<"\nTIME: Central while loop time: "<<delta<<endl;

        if(STATIC_OUTPUT){
		    if(print_cnt == 10){
				system("clear");
				print_flag = true;
				print_cnt = 0;
		    } else{
				print_flag = false;
				print_cnt++;
		    }
		    
		}
		
	}

	return 0;

}