#include "udp.cpp"

void fusion(int markerID){

	Vec3d avgAngles, avgCoords;

	for(int k = 0; k < 3; k++){
        avgCoords[k] = 0;
        avgAngles[k] = 0;
    }

    int cameraCount = 0;

    for(int j = 0; j < 6; j++){

        if(dataToProcess[markerID][j].valuesStored){

            if(print_flag){
                cout << "\nAVG: Marker " << markerID << " found by camera "<< j+1 << endl;
                cout << "AVG: Using fixed marker ID: " << dataToProcess[markerID][j].fixedMarker << endl;
                cout << "AVG: Coordinates:\t" << dataToProcess[markerID][j].coords << endl;
                cout << "AVG: Angle:\t\t" << dataToProcess[markerID][j].angles << endl << endl;
            }

            // For averaging
            for(int k = 0; k < 3; k++){
                avgCoords[k] += dataToProcess[markerID][j].coords[k];
                avgAngles[k] += dataToProcess[markerID][j].angles[k];
            }
            cameraCount++;
        }

    } // for j./loop

    for(int k = 0; k < 3; k++){
        avgCoords[k] /= cameraCount;
        avgAngles[k] /= cameraCount;
    }

    if(print_flag){
    	cout << "\nSEND: Sending final data data for marker " << markerID << endl;
	    cout << "SEND: Coordinates to send:\t" << avgCoords << endl;
	    cout << "SEND: Angles to send:\t\t" << avgAngles << endl;
	}

	UDPSendFinal(markerID, avgAngles, avgCoords);
}

int main(){

	timespec start, stop, start_proc, stop_proc, start_comm, stop_comm;
	double delta;

	UDPSet(true);

	thread t[RPI_NO];
	print_flag = true;

	while(1){

		// while loop exeution time measurement
		clock_gettime(CLOCK_MONOTONIC, &start);

		if(print_flag) cout<<"\n---------- RECEIVE LOOP START ----------"<<endl;

		// assigns 0's to received_data vector
		received_data.assign(received_data.size(), 0);

	    // reset valuesStored and markerFound flags
	    for(int i = 0; i < 100; i++){

	        for(int j = 0; j < 6; j++){
	            dataToProcess[i][j].valuesStored = false;
	        }
	    }

	    clock_gettime(CLOCK_MONOTONIC, &start_comm);

	    // receive and store data in dataToProcess
		for(int i=0; i<6; i++){
			t[i] = thread(UDPRec, i);
		}

		for(int i=0; i<6; i++){
			t[i].join();
		}

		clock_gettime(CLOCK_MONOTONIC, &stop_comm);

		delta = ( stop_comm.tv_sec - start_comm.tv_sec )
             + (double)( stop_comm.tv_nsec - start_comm.tv_nsec )
               / (double)MILLION;

        if(print_flag) cout<<"\nTIMECOMM: "<<delta<<endl;

        clock_gettime(CLOCK_MONOTONIC, &start_proc);
		
		// perform fusion on dataToProcess
		for(int i=50; i<100; i++){
			if(received_data[i]){
				fusion(i);
			}
		}

		clock_gettime(CLOCK_MONOTONIC, &stop_proc);

		delta = ( stop_proc.tv_sec - start_proc.tv_sec )
             + (double)( stop_proc.tv_nsec - start_proc.tv_nsec )
               / (double)MILLION;

        if(print_flag) cout<<"\nTIMECOMM: "<<delta<<endl;

		if(print_flag){
			cout<<"\nPacket sequence numbers:"<<endl;
            for(int k=1; k<7;k++){
	            cout<<"CAM"<<k<<"\t\t";
	        }
	        cout<<endl;
	        for(int k=0; k<10;k++){
	            for(int cam_no=0; cam_no<6; cam_no++){
	                cout<<seq_nos[k][cam_no]<<"("<<times[k][cam_no]<<")\t";
	            }
	            cout<<endl;
	        }
	    }

		if(print_flag) cout<<"\n----------  RECEIVE LOOP END  ----------"<<endl;

		char c=(char)waitKey(25);
		if(c==27)
			break;

		// while loop exeution time measurement
		clock_gettime(CLOCK_MONOTONIC, &stop);

		delta = ( stop.tv_sec - start.tv_sec )
             + (double)( stop.tv_nsec - start.tv_nsec )
               / (double)MILLION;

        if(print_flag) cout<<"\nTIMELOOP: "<<delta<<endl;

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