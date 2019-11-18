#include "localize.h"

int socketFileDescr;

const struct sockaddr *sendToAddr;
socklen_t sendToAddrLen;
    
/*void UDPSetSender(const char *IP){

	// address family set to IPv4
    sendToAddr.sin_family = AF_INET;

    // convert IP addr from numbers-and-dots notation into binary
    // return 1 if succesful, 0 otherwise
    socketFileDescr = inet_aton(IP, &sendToAddr.sin_addr);

    if (socketFileDescr == 0){ 

        cout << "Error in coverting string address to binary number" << endl;
        exit(1);

    }

    // set port 
    sendToAddr.sin_port = htons(port);

    // create socket
    // PF_INET 		-> IP layer 3 protocol
    // SOCK_DGRAM	-> support datagrams
    // IPPROTO_UDP	-> UDP
    socketFileDescr = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (socketFileDescr == -1){

        cout<<"Error in socket creation "<< endl;

    }
}*/

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

    cout<<"\nUDPSet: Address info freed. UDP communication setup complete."<<endl;

}

void UDPSend(int markerID, Vec3d data, Vec3d rot){

    int send_status;

    t = time(0);
    now = static_cast<double> (t);
    //std::cout<<"stamps:"<<now<< std::endl;

    loc_data = {
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
    	socketFileDescr,
    	&loc_data,
    	sizeof(loc_data),
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
    sockaddr_in recAddr;
    char s[INET6_ADDRSTRLEN];

    socklen_t addr_len = sizeof(recAddr);

    if(print_flag) cout<<"\nUDPRec: waiting to receive message"<<endl;

    // Receive mesasge
    rec_status = recvfrom(
        socketFileDescr, 
        &loc_data,
        sizeof(loc_data),
        0,
        (struct sockaddr *)&recAddr, 
        &addr_len
    );

    if (rec_status == -1){

        if(print_flag) cout<< "\nUDPRec: error receiving data"<<"\n error code: "<< rec_status << endl;

    }

    if(print_flag) cout<<"\nUDPRec: Message received.."<<endl;
    inet_ntop(
        recAddr.sin_family,
        (struct sockaddr *)&recAddr.sin_addr,
        s, 
        sizeof s
    );
    if(print_flag) cout<<"\nUDPRec: received location data contains:"<<endl;
    if(print_flag) cout<<"UDPRec:  markerID: "<<loc_data[1]<<endl;

}

