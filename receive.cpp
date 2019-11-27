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
                cout << "\nAVG: Marker " << markerID << " found by camera "<< j << endl;
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

	UDPSet(true);

	print_flag = true;

	while(1){

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
		for(int i=0; i<RPI_NO; i++){
			UDPRec(i);
		}
		
		// perform fusion on dataToProcess
		for(int i=50; i<100; i++){
			if(received_data[i]){
				fusion(i);
			}
		}

		if(print_flag) cout<<"\n----------  RECEIVE LOOP END  ----------"<<endl;

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

		char c=(char)waitKey(25);
		if(c==27)
			break;

		
	}

	return 0;

}