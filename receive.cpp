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

            // For diffs
            /*for(int di = j+1; di < 8; di++){
                if(dataToProcess[markerID][di].valuesStored){
                    diffs[j][di][0] = abs(dataToProcess[markerID][di].coords[0] - dataToProcess[markerID][j].coords[0]) * 1000;
                    diffs[di][j][0] = diffs[j][di][0];
                    diffs[j][di][1] = abs(dataToProcess[markerID][di].coords[1] - dataToProcess[markerID][j].coords[1]) * 1000;
                    diffs[di][j][1] = diffs[j][di][1];
                }
            }*/
        }

    } // for j

    for(int k = 0; k < 3; k++){
        avgCoords[k] /= cameraCount;
        avgAngles[k] /= cameraCount;
    }

    if(print_flag){
    	cout << "\nSEND: Sending final data data for marker " << markerID << endl;
	    cout << "SEND: Coordinates to send:\t" << avgCoords << endl;
	    cout << "SEND: Angles to send:\t\t" << avgAngles << endl;
	}

	UDPFinalSend(markerID, avgAngles, avgCoords);
}

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

		if(print_flag) cout<<"\n---------- RECEIVE LOOP START ----------"<<endl;

		// assigns 0's to received_data vector
		received_data.assign(received_data.size(), 0);

	    // reset valuesStored and markerFound flags
	    for(int i = 0; i < 100; i++){

	        for(int j = 0; j < 6; j++){
	            dataToProcess[i][j].valuesStored = false;
	        }
	    }

	    // receive and store data in dataToProcess
		UDPRec();

		recvd_any = false;

		for(int i = 0; i < RPI_NO; i++){
			// get number of camera that sent
		    int camera_no = (int)recv_data_aggr[i][0];

		    if(print_flag) cout<<"\nUDPRec: Message received from camera "<<camera_no<<endl;

		    int markerID = recv_data_aggr[i][1];

		    if( markerID != 0) recvd_any = true;

		    int camera_index = camera_no - 1;

		    dataToProcess[markerID][camera_index].fixedMarker = recv_data_aggr[i][2];
		    dataToProcess[markerID][camera_index].coords[0] = recv_data_aggr[i][3];
		    dataToProcess[markerID][camera_index].coords[1] = recv_data_aggr[i][4];
		    dataToProcess[markerID][camera_index].coords[2] = recv_data_aggr[i][5];
		    dataToProcess[markerID][camera_index].angles[0] = recv_data_aggr[i][6];
		    dataToProcess[markerID][camera_index].angles[1] = recv_data_aggr[i][7];
		    dataToProcess[markerID][camera_index].angles[2] = recv_data_aggr[i][8];
		    dataToProcess[markerID][camera_index].valuesStored = true;
		    received_data[markerID] = true;

		}
		
		if(recvd_any){
			// perform fusion on dataToProcess
			for(int i=50; i<100; i++){
				if(received_data[i]){
					fusion(i);
				}
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