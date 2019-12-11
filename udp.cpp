#include "localize.h"
#include "local_settings.h"

#define REC_IP        "192.168.0.1"
#define FINAL_IP      "192.168.0.3"

int socketFileDescrPi;
int socketFileDescrCent[RPI_NO]; 
int socketFileDescrFinal;
int seq_no = 0;

const struct sockaddr *sendToAddr, *sendToFinalAddr;
socklen_t sendToAddrLen, sendToFinalAddrLen;

/*typedef struct = {
    int CAM_NO,
    int markerID,
    int fixedMarkerID,
    double coordX, 
    double coordY,
    double coordZ, 
    double angleX, 
    double angleY, 
    double angleZ,
    int seq_no,
    time_t now,
} payload;*/

void *get_in_addr(struct sockaddr *sa){
    return &(((struct sockaddr_in*)sa)->sin_addr);
}

const char* getPiPort(){
    if(CAM_NO == 1) return "3001";
    else if(CAM_NO == 2) return "3002";
    else if(CAM_NO == 3) return "3003";
    else if(CAM_NO == 4) return "3004";
    else if(CAM_NO == 5) return "3005";
    else if(CAM_NO == 6) return "3006";
    else if(CAM_NO == 0) return "3000"; // just so we don't get error on receiver
    else{
        cout<<"Error setting const value for port"<<endl;
        exit(1);
    }
}

void UDPSend(int markerID, int fixedMarkerID, Vec3d data, Vec3d rot){

    int send_status;
    Vec<double,11> send_data;
    char ipstr[INET6_ADDRSTRLEN];
    delta = ( mark.tv_sec - start.tv_sec )
             + (double)( mark.tv_nsec - start.tv_nsec )
               / (double)BILLION;

    if(seq_no > INT_MAX) seq_no = 0;

    send_data = {
        double(CAM_NO),
        double(markerID),
        double(fixedMarkerID),
        data[0], 
        data[1],
        data[2], 
        rot[0], 
        rot[1], 
        rot[2],
        double(seq_no),
        delta
    };

    seq_no++;
    
    //inet_ntop(AF_INET, get_in_addr((struct sockaddr *)sendToAddr), ipstr, sizeof ipstr);
    //if(print_flag) cout<<"UDPSet:  Sending to: "<<ipstr<<endl;

    if(print_flag && OUTP_MEM) cout<<"Size of sent data is "<<sizeof(send_data)<<" bytes"<<endl;
    
    // Transmit message
    send_status = sendto(
        socketFileDescrPi,
        &send_data,
        sizeof(send_data),
        0,
        sendToAddr, 
        sendToAddrLen
    );

    if(print_flag){ 
        cout<<"SEQ NO:\t"<<seq_no<<endl;
        cout<<"TIME:\t"<<delta<<endl;
    }

    if (send_status == -1){

        cout<<"\nUDPSend: error sending data"<<"\n error code: "<< send_status << endl;

    }
}

void UDPRec(int threadIndex){

    int rec_status;
    sockaddr_in recv_addr;
    char s[INET6_ADDRSTRLEN];
    Vec<double,11> recv_data;
    socklen_t addr_len = sizeof(recv_addr);
    char recv_addr_string[INET_ADDRSTRLEN];
    int camera_no;
    int markerID;
    int camera_index;

    if(print_flag) cout<<"\nUDPRec: waiting to receive message"<<endl;

    // Receive mesasge
    rec_status = recvfrom(
        socketFileDescrCent[threadIndex], 
        &recv_data,
        sizeof(recv_data),
        0,
        (struct sockaddr *)&recv_addr, 
        &addr_len
    );

    if (rec_status == -1){

        if(print_flag) cout<< "\nUDPRec: error receiving data"<<"\n error code: "<< rec_status << endl;

    }

    camera_no = (int)recv_data[0];
    markerID = recv_data[1];
    camera_index = camera_no - 1;

    if(markerID != 0){ // only if marker actually found

        // get number of camera that sent

        if(print_flag) cout<<"\nUDPRec: Message received from camera "<<camera_no<<endl;

        dataToProcess[markerID][camera_index].fixedMarker = recv_data[2];
        dataToProcess[markerID][camera_index].coords[0] = recv_data[3];
        dataToProcess[markerID][camera_index].coords[1] = recv_data[4];
        dataToProcess[markerID][camera_index].coords[2] = recv_data[5];
        dataToProcess[markerID][camera_index].angles[0] = recv_data[6];
        dataToProcess[markerID][camera_index].angles[1] = recv_data[7];
        dataToProcess[markerID][camera_index].angles[2] = recv_data[8];
        dataToProcess[markerID][camera_index].valuesStored = true;
        received_data[markerID] = true;

        if(print_flag){
            cout<<"UDPRec:  - markerID: "<<markerID<<endl;
            cout<<"UDPRec:  - coordinates: ["
                <<dataToProcess[markerID][camera_index].coords[0]<<" , "
                <<dataToProcess[markerID][camera_index].coords[1]<<" , "
                <<dataToProcess[markerID][camera_index].coords[2]<<"]"<<endl;
            cout<<"UDPRec:  - rotation: ["
                <<dataToProcess[markerID][camera_index].angles[0]<<" , "
                <<dataToProcess[markerID][camera_index].angles[1]<<" , "
                <<dataToProcess[markerID][camera_index].angles[2]<<"]"<<endl;
        }
    } else{
        if(print_flag){
            cout<<"\nUDPRec: Message received from camera: "<<recv_data[0]<<endl;
            cout<<"UDPRec: No marker found."<<endl;
        }
    }

    for(int i=9; i>0; i--){
        seq_nos[i][camera_index] = seq_nos[i-1][camera_index];
        times[i][camera_index] = times[i-1][camera_index];
    }

    seq_nos[0][camera_index] = recv_data[9];
    times[0][camera_index] = recv_data[10];

}

void UDPSet(bool isReceiver){

    int addr_status;
    int send_status, rec_status;
    struct addrinfo hints;
    struct addrinfo *addrInfo, *p;
    char ipstr[INET6_ADDRSTRLEN];

    const char* portPiConst = getPiPort();
    const char* portCentFinal = "3000";
    const char* portsCent[RPI_NO] = {"3001", "3002", "3003", "3004", "3005", "3006"};

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // AF_INET or AF_INET6 to force version
    hints.ai_socktype = SOCK_DGRAM;

    if(isReceiver){
        for(int i=0; i< RPI_NO; i++){
    
            if( (addr_status = getaddrinfo(REC_IP, portsCent[i], &hints, &addrInfo)) != 0) {

                cout << "UDPSet:  Error in getting addr info from "<<REC_IP<< endl;
                cout << gai_strerror(addr_status) << endl;
                exit(1);

            }

            if( (socketFileDescrCent[i] = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol)) == -1){

                cout<<"UDPSet: Error in socket creation "<< endl;
                exit(1);

            }

            if( bind(socketFileDescrCent[i], addrInfo->ai_addr, addrInfo->ai_addrlen) == -1){
                cout<<"UDPSet: Error in binding receiver socket "<< endl;
                close(socketFileDescrCent[i]);
                exit(1);
            }

            // convert fetched IP to a string and print it
            inet_ntop(addrInfo->ai_family, get_in_addr((struct sockaddr *)addrInfo->ai_addr), ipstr, sizeof ipstr);
            cout<<"UDPSet:  Created and bound socket for: "<<ipstr<<":"<<portsCent[i]<<endl;

        }

        // extra socket for an eventual final sending
        if ( (addr_status = getaddrinfo(FINAL_IP, portCentFinal, &hints, &addrInfo)) != 0) {

            cout << "UDPSet:  Error in getting addr info from "<<FINAL_IP<< endl;
            cout << gai_strerror(addr_status) << endl;
            exit(1);

        }

        if ( (socketFileDescrFinal = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol)) == -1){

            cout<<"UDPSet: Error in socket creation for final sending "<< endl;
            exit(1);

        }

        // convert fetched IP to a string and print it
        inet_ntop(addrInfo->ai_family, get_in_addr((struct sockaddr *)addrInfo->ai_addr), ipstr, sizeof ipstr);
        cout<<"UDPSet:  Created final send socket for: "<<ipstr<<":"<<portCentFinal<<endl;

        sendToFinalAddr = addrInfo->ai_addr;
        sendToFinalAddrLen = addrInfo->ai_addrlen;

    }
    else{
        
        cout<<"UDPSet: CAM "<<CAM_NO<<" fetching addr info for "<<REC_IP<<":"<<portPiConst<<endl;

        if( (addr_status = getaddrinfo(REC_IP, portPiConst, &hints, &addrInfo)) != 0) {

            cout << "UDPSet:  Error in getting addr info from "<<REC_IP<< endl;
            cout << gai_strerror(addr_status) << endl;
            exit(1);

        }

        if( (socketFileDescrPi = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol)) == -1){

            cout<<"UDPSet: Error in socket creation "<< endl;
            exit(1);

        }

        // convert fetched IP to a string and print it
        inet_ntop(addrInfo->ai_family, get_in_addr((struct sockaddr *)addrInfo->ai_addr), ipstr, sizeof ipstr);
        cout<<"UDPSet:  Created and bound socket for: "<<ipstr<<":"<<portPiConst<<endl;

        sendToAddr = addrInfo->ai_addr;
        sendToAddrLen = addrInfo->ai_addrlen;
        
        cout<<"UDPSet: sendToAddr set to "<< inet_ntop(addrInfo->ai_family, get_in_addr((struct sockaddr *)sendToAddr), ipstr, sizeof ipstr)<<endl;
    }

    // don't do this or sendTo(Final)Addr pointers will point to freed memory
    // freeaddrinfo(addrInfo);

    cout<<"UDPSet: UDP communication setup complete."<<endl;

}

void UDPSendFinal(int markerID, Vec3d data, Vec3d rot){

    int send_status;
    Vec<double,8> send_data;
    time_t t;
    double now;

    t = time(0);
    now = static_cast<double> (t);
    //std::cout<<"stamps:"<<now<< std::endl;

    send_data = {
        now, 
        double(markerID),
        data[0], 
        data[1],
        data[2], 
        rot[0], 
        rot[1], 
        rot[2]
    }; 
    
    // Transmit message
    send_status = sendto(
        socketFileDescrFinal,
        &send_data,
        sizeof(send_data),
        0,
        sendToFinalAddr, 
        sendToFinalAddrLen
    );

    if (send_status == -1){

        cout<<"\nUDPSend: error sending data to final receiver"<<"\n error code: "<< send_status << endl;

    }
}

