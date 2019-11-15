#include "udp.cpp"

int main(){

	UDPSet(receiverIP);

	if(!STATIC_OUTPUT) print_flag = true;

	while(1){

		if(print_flag) cout<<"\n---------- RECEIVE LOOP START ----------"<<endl;


		UDPRec();

		cout<<endl<<loc_data<<endl;

		char c=(char)waitKey(25);
		if(c==27)
			break;

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
		
	}

	return 0;

}