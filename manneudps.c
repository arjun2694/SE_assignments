

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>


struct IPnTime {
   unsigned int  ip[4];
   int  timed;
};

struct IncomingPacket {
   unsigned int  ip[4];
   int  transID;
};

struct DHCPSuggest {
   unsigned int  ip[4];
   int  timed;
};

struct DHCPACK {
   unsigned int  ip[4];
};

int isIpAlreadyInUse(unsigned int ipAddress[], struct IPnTime *temp, int count){
	for(int i=0;i<count;++i){
		int found=1;
		for(int k=0;k<4;++k){
			if(ipAddress[k]!=temp->ip[k]){
				found=0;
				break;
			}
		}
		if(found==1){
			return 0;
		}
		++temp;
	}

	return 1;
}

int* getNextIP (unsigned int hostS[], unsigned int network[], unsigned int currentIP[]) {
    static int nextIP[4];
	int found=0;
	for(int i=3;i>=0;--i){
		if(hostS[i]!=0 && hostS[i]<=(currentIP[i]-network[i]) && !found){
			nextIP[i]=0;
		}else if(hostS[i]!=0 && !found){
			nextIP[i]=currentIP[i]+1;
			found=1;
		}else{
			nextIP[i]=currentIP[i];
		}
	}

	return nextIP;
}

int main(int argc, char *argv[]){
	if (argc != 4) {
		printf("Usage: ./Server <PORT_NUMBER> <GATEWAY_IP> <SUBNET_MASK>\n");
		return 1;
	}
	int portNo =atoi(argv[1]);
	char temp[20];
	strcpy(temp,argv[3]);
	char *p= strtok (temp, ".");
	int sCount,i=0;
	int iSubnet[4];
    while (p != NULL)
    {
		char tt[4];
		strcpy(tt,p);
		iSubnet[i]=atoi(tt);
		if(iSubnet[i]<0||iSubnet[i]>255){
			printf("Subnet Given is Wrong\n");
			return 1;
		}
		++sCount;
		++i;
		p = strtok (NULL, ".");
    }

	if(sCount!=4){
		printf("Subnet Given is Wrong\n");
		return 1;
	}
	strcpy(temp,argv[2]);    //To get gateway
	p= strtok (temp, ".");
	i=0;
	sCount=0;
	int iGateway[4];
    while (p != NULL)
    {
        char tt[4];
		strcpy(tt,p);
		iGateway[i]=atoi(tt);
		if(iGateway[i]<0||iGateway[i]>255){
			printf("Gateway Given is Wrong\n");
			return 1;
		}
		++sCount;
		++i;
		p = strtok (NULL, ".");
    }

	if(sCount!=4){
		printf("Gateway Given is Wrong\n");
		return 1;
	}

	unsigned int hostSize[4];
	unsigned int networkIP[4];
	for (i = 0; i < sCount; ++i){
       hostSize[i]=iSubnet[i]^255;
	   networkIP[i]=iSubnet[i]&iGateway[i];
	}

	int udpSocket, nBytes;
	struct sockaddr_in serverAddr, clientAddr;
	struct sockaddr_storage serverStorage;
	socklen_t address_size, client_addr_size;
	udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	memset((char *) &serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(portNo);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
	address_size = sizeof serverStorage;
	struct IncomingPacket *inData= malloc(sizeof(struct IncomingPacket));

	int k;
	int currentIP[4];
	for(k=0;k<4;++k){
		currentIP[k]=networkIP[k];
	}
	int *pp;

	int mSize=(hostSize[0]>0?hostSize[0]:1)*(hostSize[1]>0?hostSize[1]:1)*(hostSize[2]>0?hostSize[2]:1)*(hostSize[3]>0?hostSize[3]:1);
	struct IPnTime *root=malloc(mSize*sizeof(struct IPnTime));
	int reservedCount=0;

	for(k=0;k<4;++k){
		root[reservedCount].ip[k]=iGateway[k];
	}
	root[reservedCount].timed=-1;
	++reservedCount;

	while(1){
		nBytes = recvfrom(udpSocket,inData,sizeof(*inData),0,(struct sockaddr *)&serverStorage, &address_size);
		if(inData->ip[0]==0&&inData->ip[1]==0&&inData->ip[2]==0&&inData->ip[3]==0){
			printf("\nRequest For New IP Received %d.%d.%d.%d\n",inData->ip[0],inData->ip[1],inData->ip[2],inData->ip[3]);
			printf("Client Transaction ID is %d\n",inData->transID);
			while(!isIpAlreadyInUse(currentIP,root,reservedCount)){
				pp=getNextIP(hostSize,networkIP,currentIP);
				for(k=0;k<4;++k){
					currentIP[k]=*(pp+k);
				}
			}
			struct DHCPSuggest *s1=malloc(sizeof(struct DHCPSuggest));
			for(k=0;k<4;++k){
				s1->ip[k]=currentIP[k];
			}
			s1->timed=3600;
			sendto(udpSocket,s1,sizeof(struct DHCPSuggest),0,(struct sockaddr *)&serverStorage,address_size);
			printf("IP Address %d.%d.%d.%d sent\n",s1->ip[0],s1->ip[1],s1->ip[2],s1->ip[3]);
			pp=getNextIP(hostSize,networkIP,currentIP);
			for(k=0;k<4;++k){
				currentIP[k]=*(pp+k);
			}
			while(!isIpAlreadyInUse(currentIP,root,reservedCount)){
				pp=getNextIP(hostSize,networkIP,currentIP);
				for(k=0;k<4;++k){
					currentIP[k]=*(pp+k);
				}
			}
			struct DHCPSuggest *s2=malloc(sizeof(struct DHCPSuggest));
			for(k=0;k<4;++k){
				s2->ip[k]=currentIP[k];
			}
			s2->timed=3600;
			sendto(udpSocket,s2,sizeof(struct DHCPSuggest),0,(struct sockaddr *)&serverStorage,address_size);
			printf("IP Address %d.%d.%d.%d sent\n",s2->ip[0],s2->ip[1],s2->ip[2],s2->ip[3]);
			pp=getNextIP(hostSize,networkIP,currentIP);
			for(k=0;k<4;++k){
				currentIP[k]=*(pp+k);
			}
			while(!isIpAlreadyInUse(currentIP,root,reservedCount)){
				pp=getNextIP(hostSize,networkIP,currentIP);
				for(k=0;k<4;++k){
					currentIP[k]=*(pp+k);
				}
			}
			struct DHCPSuggest *s3=malloc(sizeof(struct DHCPSuggest));
			for(k=0;k<4;++k){
				s3->ip[k]=currentIP[k];
			}
			s3->timed=3600;
			sendto(udpSocket,s3,sizeof(struct DHCPSuggest),0,(struct sockaddr *)&serverStorage,address_size);
			printf("IP Address %d.%d.%d.%d sent\n",s3->ip[0],s3->ip[1],s3->ip[2],s3->ip[3]);
			pp=getNextIP(hostSize,networkIP,currentIP);
			for(k=0;k<4;++k){
				currentIP[k]=*(pp+k);
			}
		}else{
			printf("\nRegistering IP Address %d.%d.%d.%d is sent\n",inData->ip[0],inData->ip[1],inData->ip[2],inData->ip[3]);
			printf("Client Transaction ID is %d\n",inData->transID);
			struct DHCPACK *ackMsg=malloc(sizeof(struct DHCPACK));
			for(k=0;k<4;++k){
				root[reservedCount].ip[k]=inData->ip[k];
				ackMsg->ip[k]=inData->ip[k];
			}
			root[reservedCount].timed=-1;
			++reservedCount;
			sendto(udpSocket,ackMsg,sizeof(struct DHCPACK),0,(struct sockaddr *)&serverStorage,address_size);
		}

	}

	return 0;
}