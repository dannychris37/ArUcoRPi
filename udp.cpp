#include "localize.h"
#include "local_settings.h"

#define REC_IP          "192.168.0.1"
#define GATHER_IP       "192.168.0.102"
#define FINAL_IP        "192.168.0.4"

int socketFileDescrPi;
int socketFileDescrPiReceiver[RPI_NO];
int socketFileDescrCent;

const struct sockaddr *sendToAddr, *sendToFinalAddr;
socklen_t sendToAddrLen, sendToFinalAddrLen;

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

void UDPSet(bool isCentral){

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

    if(isCentral){ // if central PC
    
        if( (addr_status = getaddrinfo(FINAL_IP, "3002", &hints, &addrInfo)) != 0) {

            cout << "UDPSet:  Error in getting addr info from "<<REC_IP<< endl;
            cout << gai_strerror(addr_status) << endl;
            exit(1);

        }

        if( (socketFileDescrCent = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol)) == -1){

            cout<<"UDPSet: Error in socket creation "<< endl;
            exit(1);

        }

        if( bind(socketFileDescrCent, addrInfo->ai_addr, addrInfo->ai_addrlen) == -1){
            cout<<"UDPSet: Error in binding receiver socket "<< endl;
            close(socketFileDescrCent);
            exit(1);
        }

        // convert fetched IP to a string and print it
        inet_ntop(addrInfo->ai_family, get_in_addr((struct sockaddr *)addrInfo->ai_addr), ipstr, sizeof ipstr);
        cout<<"UDPSet:  Created and bound socket for: "<<ipstr<<":"<<portsCent[2]<<endl;

    }
    else{ // if pi
        if(CAM_NO != 2){ // sending socket for each CAM (except 2)

            if( (addr_status = getaddrinfo(GATHER_IP, portPiConst, &hints, &addrInfo)) != 0) {

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
            cout<<"UDPSet:  Created sending socket for: "<<ipstr<<":"<<portPiConst<<endl;

            sendToAddr = addrInfo->ai_addr;
            sendToAddrLen = addrInfo->ai_addrlen;
            
            cout<<"UDPSet: sendToAddr set to "<< inet_ntop(addrInfo->ai_family, get_in_addr((struct sockaddr *)sendToAddr), ipstr, sizeof ipstr)<<endl;

        } else{// create listening sockets for CAM2

            for(int i=0; i< RPI_NO; i++){

                if(i==CAM_NO-1) i++; //skip own port no.

                if( (addr_status = getaddrinfo(GATHER_IP, portsCent[i], &hints, &addrInfo)) != 0) {

                    cout << "UDPSet:  Error in getting addr info from "<<REC_IP<< endl;
                    cout << gai_strerror(addr_status) << endl;
                    exit(1);
                }

                if( (socketFileDescrPiReceiver[i] = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol)) == -1){

                    cout<<"UDPSet: Error in socket creation "<< endl;
                    exit(1);

                }

                if( bind(socketFileDescrPiReceiver[i], addrInfo->ai_addr, addrInfo->ai_addrlen) == -1){
                    cout<<"UDPSet: Error in binding receiver socket "<< endl;
                    close(socketFileDescrPiReceiver[i]);
                    exit(1);
                }

                // convert fetched IP to a string and print it
                inet_ntop(addrInfo->ai_family, get_in_addr((struct sockaddr *)addrInfo->ai_addr), ipstr, sizeof ipstr);
                cout<<"UDPSet:  Created and bound listening socket for: "<<ipstr<<":"<<portsCent[i]<<endl;
            }

            // need to create extra socket for sending to central
            if( (addr_status = getaddrinfo(FINAL_IP, portPiConst, &hints, &addrInfo)) != 0) {

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
            cout<<"UDPSet:  Created sending socket for: "<<ipstr<<":"<<portPiConst<<endl;

            sendToFinalAddr = addrInfo->ai_addr;
            sendToFinalAddrLen = addrInfo->ai_addrlen;
            
            cout<<"UDPSet: sendToAddr set to "<< inet_ntop(addrInfo->ai_family, get_in_addr((struct sockaddr *)sendToFinalAddr), ipstr, sizeof ipstr)<<endl;

        }
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
        socketFileDescrPi,
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

void UDPSend(int markerID, int fixedMarkerID, Vec3d data, Vec3d rot){

    int send_status;
    Vec<double,9> send_data;
    char ipstr[INET6_ADDRSTRLEN];

    send_data = {
        double(CAM_NO),
        double(markerID),
        double(fixedMarkerID),
        data[0], 
        data[1],
        data[2], 
        rot[0], 
        rot[1], 
        rot[2]
    }; 
    
    //inet_ntop(AF_INET, get_in_addr((struct sockaddr *)sendToAddr), ipstr, sizeof ipstr);
    //if(print_flag) cout<<"UDPSet:  Sending to: "<<ipstr<<endl;
    
    // Transmit message
    send_status = sendto(
        socketFileDescrPi,
        &send_data,
        sizeof(send_data),
        0,
        sendToAddr, 
        sendToAddrLen
    );

    if (send_status == -1){

        cout<<"\nUDPSend: error sending data"<<"\n error code: "<< send_status << endl;

    }
}

void UDPRec(){

    int rec_status;
    sockaddr_in recv_addr;
    Vec<double,8> recv_data;
    char s[INET6_ADDRSTRLEN];
    socklen_t addr_len = sizeof(recv_addr);
    char recv_addr_string[INET_ADDRSTRLEN];
    int camera_no;

    if(print_flag) cout<<"\nUDPRec: waiting to receive message"<<endl;

    // Receive mesasge
    rec_status = recvfrom(
        socketFileDescrCent, 
        &recv_data,
        sizeof(recv_data),
        0,
        (struct sockaddr *)&recv_addr, 
        &addr_len
    );

    if (rec_status == -1){

        if(print_flag) cout<< "\nUDPRec: error receiving data"<<"\n error code: "<< rec_status << endl;

    }

    if(print_flag){
        cout<<"RECV: Received data for marker "<<recv_data[1]<<endl;
        cout<<"RECV: Coordinates received:\t["<<recv_data[2]<<","<<recv_data[3]<<","<<recv_data[4]<<"]"<<endl;
        cout<<"RECV: Angles received:\t\t["<<recv_data[5]<<","<<recv_data[6]<<","<<recv_data[7]<<"]"<< endl;
    }

}

void UDPRecAggr(int threadIndex){
    
    int rec_status;
    sockaddr_in recv_addr;
    char s[INET6_ADDRSTRLEN];
    Vec<double,9> recv_data;
    socklen_t addr_len = sizeof(recv_addr);
    char recv_addr_string[INET_ADDRSTRLEN];
    int camera_no;

    // Receive mesasge
    rec_status = recvfrom(
        socketFileDescrPiReceiver[threadIndex], 
        &recv_data,
        sizeof(recv_data),
        0,
        (struct sockaddr *)&recv_addr, 
        &addr_len
    );
    
    if (rec_status == -1){

        if(print_flag) cout<< "\nUDPRec: error receiving data"<<"\n error code: "<< rec_status << endl;

    }

    // get number of camera that sent
    camera_no = (int)recv_data[0];
    
    if(print_flag) cout<<"\nUDPRec: Message received from camera "<<camera_no<<endl;
    
    if(recv_data[1] != 0)
        recv_data_aggr[camera_no - 1] = recv_data;
    
}
