//Author Paritosh Ramanan
#include <sys/signal.h>
#include "xbee.h"
#include "LogUtil.h"

char data[5]={0x48,0x45,0x4C,0x4C,0x4F};


int xbee_fd;
unsigned char stream_control;
pthread_t listener_thread,server_thread;
unsigned char dest_address[8];
 



int main(int argc, char *argv[])
{	
	 	
struct sigaction saio;
f_setowner_ex arg;
dest_address[0]=0x00;
dest_address[1]=0x13;
dest_address[2]=0xA2;
dest_address[3]=0x00;
dest_address[4]=0x40;
dest_address[5]=0xB2;
dest_address[6]=0x38;
dest_address[7]=0x6D;

	//FILE *station_file, *arrival_time_file;
   
	memset(stn_in_file, -1, STATION_NUM*sizeof(int));
  
    	
	if ( (arrival_time_file  = fopen(argv[2], "a+")) == NULL) 
	{
		error (LOG_APPL,"error opening file %s\n", argv[2]);
	}

	if ( (station_file  = fopen(argv[3], "a+")) == NULL) 
	{
		error (LOG_APPL,"error opening file %s\n", argv[3]);
	}
 	
	//if( access( argv[1], F_OK ) == -1 ) { 
	//	system("echo BB-UART2 > /sys/devices/bone_capemgr.*/slots");
	//}
	
	xbee_fd = open (argv[1], O_RDWR | O_NOCTTY | O_SYNC);
	if (xbee_fd < 0)
	{	printf("Error enabling serial port for XBee!\nFollow the steps to resolve the issue:\n1) In case of Desktop Linux please check access permissions to /dev/ttyUSB*\n2) In case of BeagleBone Black, execute \"echo BB-UART2 > /sys/devices/bone_capemgr.*/slots\"\n");
		error(LOG_APPL,"error %d opening serial port %s: %s\n", errno, argv[1], strerror (errno));
		return -1;
	}

	

	
	saio.sa_handler = signal_handler_IO;
	saio.sa_flags = 0;
	saio.sa_restorer = NULL; 
	sigaction(SIGIO,&saio,NULL);
	fcntl(xbee_fd, F_SETFL, FNDELAY);
	//fcntl(fd, F_SETOWN, getpid());
	fcntl(xbee_fd, F_SETFL,  O_ASYNC );	

	set_interface_attribs (xbee_fd, B9600, 0);  // set speed to 115,200 bps, 8n1 (no parity)
	set_blocking (xbee_fd, 0);                // set no blocking
	

//*******Sender specific routine**********	
	
	
        //int n;
       
        unsigned char einfo[52];
        //bzero(einfo,100);
//    //test an event
        EventInfo event;
        event.id = 0;
        event.sps=45;
        event.x = 2.0;
        event.y = 3.0;
        event.z = 4.0;
        event.ptime=5.0;
	event.stime=6.0;
	event.ptime=7.0;       
        memcpy(einfo, &event.id,2);
        memcpy(einfo+2,&event.sps,2);
        memcpy(einfo+4,&event.x,8);
        memcpy(einfo+12,&event.y,8);
        memcpy(einfo+20,&event.z,8);
        memcpy(einfo+28,&event.ptime,8);
	memcpy(einfo+36,&event.stime,8);
	memcpy(einfo+42,&event.amplitude,8);

	arg.type= 0;

	//to relay messages to GUI
	if(!pthread_create(&listener_thread, NULL, &XBeeListener, NULL))
		{
			
			info(LOG_APPL,"Listener/tcp client spawned successfully\n");
			arg.pid=listener_thread;
			fcntl(xbee_fd,15,&arg);

		}
	else
		{
			error(LOG_APPL, "Error creating server thread\n");
			return 1;
		}
	//Server
	if(!pthread_create(&server_thread, NULL, &tcp_socket_server, NULL))
		{
			info(LOG_APPL,"TCP server spawned successfully\n");

		}
	else
		{
			error(LOG_APPL, "Error creating server thread\n");
			return 1;
		}
	
	
	pthread_join(listener_thread,NULL);
	pthread_join(server_thread,NULL);
     
//*********Keep the following for both cases**********************************	
	fclose(arrival_time_file);
	fclose(station_file);

	return 0;

}
