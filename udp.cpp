#include "localize.h"
#include "cam_no.h"

#define PORT      "3000"
#define REC_IP    "192.168.0.4"
#define FINAL_IP  "192.168.0.3"

int socketFileDescr, socketFileDescrFinal;

const struct sockaddr *sendToAddr, *sendToFinalAddr;
socklen_t sendToAddrLen, sendToFinalAddrLen;

void UDPSet(bool isReceiver){

    int addr_status;
    int send_status, rec_status;
    struct addrinfo hints;
    struct addrinfo *addrInfo, *p;
    char ipstr[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // AF_INET or AF_INET6 to force version
    hints.ai_socktype = SOCK_DGRAM;

    cout<<"\nUDPSet: Fetching addr info for specified receiver IP.."<<endl;
    
    if ( (addr_status = getaddrinfo(REC_IP, PORT, &hints, &addrInfo)) != 0) {

        cout << "UDPSet:  Error in getting addr info from "<<REC_IP<< endl;
        cout << gai_strerror(addr_status) << endl;
        exit(1);

    }

    cout<<"\nUDPSet: Address Information fetched."<<endl;
    cout<<"\nUDPSet: Creating sockets.."<<endl;

    // can be used for multiple receivers
    for(p = addrInfo; p!= NULL; p = p->ai_next){

        void *addr;
        char *ipver;

        if ( (socketFileDescr = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){

            cout<<"\nUDPSet: Error in socket creation "<< endl;
            continue;

        }

        // get the pointer to the address itself,
        // different fields in IPv4 and IPv6:
        if (p->ai_family == AF_INET) { // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        } else { // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        // convert fetched IP to a string and print it
        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        cout<<"UDPSet:  Created socket for "<<ipver<<": "<<ipstr<<endl;

        if(isReceiver){
            cout<<"UDPSet:  Binding socket.."<<endl;
            if( bind(socketFileDescr, p->ai_addr, p->ai_addrlen) == -1){
                cout<<"\nUDPSet: Error in binding receiver socket "<< endl;
                close(socketFileDescr);
                continue;
            }
        } else {
            sendToAddr = p->ai_addr;
            sendToAddrLen = p->ai_addrlen;
        }

        break;
    }

    if (p == NULL) {
        cout<< "\nUDPSet: Failed to bind/create socket"<<endl;
        exit(1);
    }

    //freeaddrinfo(addrInfo);

    // need to create extra socket for final sending
    if(isReceiver){

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET; // AF_INET or AF_INET6 to force version
        hints.ai_socktype = SOCK_DGRAM;

        if ( (addr_status = getaddrinfo(FINAL_IP, PORT, &hints, &addrInfo)) != 0) {

            cout << "UDPSet:  Error in getting addr info from "<<FINAL_IP<< endl;
            cout << gai_strerror(addr_status) << endl;
            exit(1);

        }

        // can be used for multiple final receivers
        for(p = addrInfo; p!= NULL; p = p->ai_next){

            void *addr;
            char *ipver;

            if ( (socketFileDescrFinal = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){

                cout<<"\nUDPSet: Error in socket creation for final sending "<< endl;
                continue;

            }

            // get the pointer to the address itself,
            // different fields in IPv4 and IPv6:
            if (p->ai_family == AF_INET) { // IPv4
                struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
                addr = &(ipv4->sin_addr);
                ipver = "IPv4";
            } else { // IPv6
                struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
                addr = &(ipv6->sin6_addr);
                ipver = "IPv6";
            }

            // convert fetched IP to a string and print it
            inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
            cout<<"UDPSet:  Created socket for "<<ipver<<": "<<ipstr<<endl;

            sendToFinalAddr = p->ai_addr;
            sendToFinalAddrLen = p->ai_addrlen;

            break;
        }

    }

    cout<<"\nUDPSet: UDP communication setup complete."<<endl;

}

void UDPFinalSend(int markerID, Vec3d data, Vec3d rot){

    int send_status;
    Vec<double,8> send_data;

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

void UDPSend(int markerID, int fixedMarkerID, Vec3d data, Vec3d rot){

    int send_status;
    Vec<double,9> send_data;

    t = time(0);
    now = static_cast<double> (t);
    //std::cout<<"stamps:"<<now<< std::endl;

    send_data = {
        now,
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
    
    // Transmit message
    send_status = sendto(
    	socketFileDescr,
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

void UDPRec(int threadIndex){

    int rec_status;
    sockaddr_in recv_addr;
    char s[INET6_ADDRSTRLEN];
    Vec<double,10> recv_data;
    socklen_t addr_len = sizeof(recv_addr);
    char recv_addr_string[INET_ADDRSTRLEN];
    int camera_no;

    if(print_flag) cout<<"\nUDPRec: waiting to receive message"<<endl;

    bool received_from_thread_cam = false;

    while(!received_from_thread_cam){

        // Receive mesasge
        rec_status = recvfrom(
            socketFileDescr, 
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
        camera_no = (int)recv_data[1];

        // convert sender addr to string
        // inet_ntop(recv_addr.sin_family, &recv_addr.sin_addr, recv_addr_string, sizeof(recv_addr_string));

        // if received from cam1 else cam2 etc.
        if(threadIndex == 0 && camera_no == 3){
            received_from_thread_cam = true;
        } else if(threadIndex == 1 && camera_no == 4){
            received_from_thread_cam = true;
        }

    }

    if(print_flag) cout<<"\nUDPRec: Message received from camera "<<camera_no<<endl;

    int markerID = recv_data[2];
    int camera_index = camera_no - 1;

    dataToProcess[markerID][camera_index].fixedMarker = recv_data[3];
    dataToProcess[markerID][camera_index].coords[0] = recv_data[4];
    dataToProcess[markerID][camera_index].coords[1] = recv_data[5];
    dataToProcess[markerID][camera_index].coords[2] = recv_data[6];
    dataToProcess[markerID][camera_index].angles[0] = recv_data[7];
    dataToProcess[markerID][camera_index].angles[1] = recv_data[8];
    dataToProcess[markerID][camera_index].angles[2] = recv_data[9];
    dataToProcess[markerID][camera_index].valuesStored = true;
    received_data[markerID] = true;

    if(print_flag){
        cout<<"\nUDPRec:  - markerID: "<<markerID<<endl;
        cout<<"UDPRec:  - coordinates: ["
            <<dataToProcess[markerID][camera_index].coords[0]<<" , "
            <<dataToProcess[markerID][camera_index].coords[1]<<" , "
            <<dataToProcess[markerID][camera_index].coords[2]<<"]"<<endl;
        cout<<"UDPRec:  - rotation: ["
            <<dataToProcess[markerID][camera_index].angles[0]<<" , "
            <<dataToProcess[markerID][camera_index].angles[1]<<" , "
            <<dataToProcess[markerID][camera_index].angles[2]<<"]"<<endl;
    }
    


}

