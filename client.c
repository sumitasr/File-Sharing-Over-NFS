
/*****************************************************************************************************************************************************/
#include <stdio.h>       
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <time.h> 
#include <sys/stat.h>
#include <fcntl.h>

#define size 1024
#define PORT "3490"

//   *****************************************************************************************************************************************************************/
char buffer[size];
int sockfd;
void video_uploading(int,FILE *fp);
void video_downloading(int,FILE *fo);
void video_delete(int,FILE *fp); 
void termination(int sockfd,char s[30]);

/****************************************************************************************************************************************************************/
struct stat status;
void *get_in_addr(struct sockaddr *sa)
{
	    if (sa->sa_family == AF_INET) 
    	    {
        		return &(((struct sockaddr_in*)sa)->sin_addr);
            }
    			return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
/***************************************************************************************************************************************************/
int main(int argc, char *argv[])
{

	    int sockfd,numbytes;
	    struct addrinfo hints, *servinfo, *p;  
	    int rv,choice,len;     
	    char ch;
            FILE *fp,*fo;
	    char s[INET6_ADDRSTRLEN];

	    if (argc != 2) 
	    {
		        fprintf(stderr,"usage: client hostname\n");
		        exit(1);
	    }

	    memset(&hints, 0, sizeof hints);
	    hints.ai_family = AF_UNSPEC;
	    hints.ai_socktype = SOCK_STREAM;

/*******************************************************************************************************************************************************************/	   

         if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) 
	     {
		        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		        return 1;
	     }

/*****************************************************************************************************************************************************************/

            if (sockfd = socket(servinfo->ai_family, servinfo->ai_socktype,servinfo->ai_protocol))
	    {
		    perror("client: socket");

	    }
/************************************************************************************************************************************************************/

	    inet_ntop(servinfo->ai_family, get_in_addr((struct sockaddr *)servinfo->ai_addr),s, sizeof(s));
	    printf("client: connecting to: %s\n", s);


/**************************************************************************************************************************************************************/ 	    
           if (connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) 
	    {
		    close(sockfd); 
		    perror("client: connect");
		    return 0;            
	    }

            freeaddrinfo(servinfo);
       
           while(1)
	    {                            
		  printf("\n *****************************************************\n");
		  printf(" Enter your choice:\n");
		  printf(" For uploading: 1\n For downloading: 2\n For delete: 3\n For exit: 4\n ");
		  scanf("%d",&choice);
     		  switch(choice)
	 	  {
			   	case 1:          
					send(sockfd,"1",1,0);
                                        video_uploading(sockfd,fp);        
					break;
				case 2:
                         		send(sockfd,"2",1,0);
                                        video_downloading(sockfd,fo);
                                                        
					break;
				case 3:           
					send(sockfd,"3",1,0);
					video_delete(sockfd,fp);
                                        break;
				case 4:
					send(sockfd,"4",1,0);
					termination(sockfd,s);
                                        exit(1);
                                        break;
			        default:
					printf("wrong choice");
                                        break;
                                                       
		 } 		
	   }			
          close(sockfd);  
    	   return 0;
} 

/***********************************************************************************************************************************************************/
  

void  video_uploading(int sockfd,FILE *fp)
{
	      char path[100],result[size],username[32],passwd[32],video_name[32];
	      int flag=0;
	      int len1,len;
	      double byte_to_send,m;
	      printf("\n*************************Upload****************************\n");
	      printf("\nclient want to upload:\n");               	                                                                                         
	      printf("Enter user name: ");
	      scanf("%s",username);
	      send(sockfd,username,32,0);
	      printf("Enter password: ");
	      scanf("%s",passwd);
	      send(sockfd,passwd,32,0);
              do
              {
	      		printf("Enter the video-file name: ");
              		scanf("%s",video_name);
              		send(sockfd,video_name,sizeof(video_name),0);
             	        recv(sockfd,buffer,32,0);
              		if(strcmp(buffer,"Video Exists")==0)
               		{
              		   printf("Video alredy exists on server");
                  		flag=1;
               	        }
               		else
               		{
                   		flag=0;
                	}
               }while(flag==1);
              while(1)
	      {  
	      		printf("Enter the path of video-file:\n ");
	      		scanf("%s",path);
              		fp=fopen(path,"r"); 

              		if(fp==NULL)
              		{
                        	  printf("File does not exist  \n");  
              		} 
              		else
              		{     
             			 if(fstat(fileno(fp),&status)==0)
                	  	 byte_to_send=(double)(status.st_size);
               		  	 sprintf(result,"%lf",byte_to_send);
			         send(sockfd,result,sizeof(result),0);   
                                 while(byte_to_send>0) 
                         	{             
                                	       len=read(fileno(fp),buffer,sizeof(buffer));
                                	       len1=send(sockfd,buffer,len,0);
					       strcpy(buffer,"\0");
                                	       m=(double)(len1);
                                	       byte_to_send-=m;
                                	       printf("byte_to_send =%lf\t",byte_to_send); 
                         	}
                  	break;         
                }
                
         }
	      fclose(fp);
}

/************************************************************************************************************************************************************/







     void video_downloading(int sockfd,FILE *fo)
     {
       		double converter,remainder,temp=0;
                int data_recv;
                char *leftover,path_name[100],buf1[30],buffer1[size],video_name[32];
                printf("\n*********************Download********************\n");
		printf("\nclient want to download:\n");
		printf("list of the videos :\n");		
		recv(sockfd,buffer,sizeof(buffer),0);
		remainder = strtod(buffer, &leftover);
				
		do
		{
                          if(data_recv=recv(sockfd,buf1,sizeof(buf1),0))
                          { 	
				if(data_recv>0)
				{
					printf("%s\n",buf1); 
					converter=(double)(data_recv);
		                   	remainder-=converter;
				}
			  }
		}while(remainder>0);                                               
	      	printf("Enter the video-file name from the list: ");
       		scanf("%s",video_name);
		printf("Enter the path, where to save the video-file: "); 
        	scanf("%s",path_name);
                send(sockfd,video_name,sizeof(video_name),0); 	
        	fo=fopen(path_name,"w");        
         	if(fo==NULL)
         	{                                   
        		      printf("file does not exist");
           		      exit(1);
         	}
         	else
       		{      
			int numTry = 20;
			fcntl(sockfd, F_SETFL, O_NONBLOCK); 
			while(numTry)
			{
			      if(data_recv=recv(sockfd,buffer1,sizeof(buffer1),0))
		       	      {
					if(data_recv>=0)
					{
						fwrite(buffer1,data_recv,1,fo);
					}
					else 
					{
						printf("waiting for 3 seconds\n");
						sleep(5);
						numTry--;						
					}
			      }
			}
		}
         		fclose(fo);
}


/********************************************************************************************************************************************************/
void video_delete(int sockfd,FILE *fp)
{
             char delete_videoname[32],user_name[32],passwd[32];
             printf("\n***********************Delete**********************\n");
             printf("\nuser want to delete a video\n");
	     printf("enter the user name:");
             scanf("%s",user_name);
	     send(sockfd,user_name,32,0);
	     printf("enter the passwd:");
             scanf("%s",passwd);
	     send(sockfd,passwd,32,0);
             printf("enter the video name which user want delete\n");
             scanf("%s",delete_videoname);
             send(sockfd,delete_videoname,32,0);
             recv(sockfd,buffer,sizeof(buffer),0);
	     printf("%s",buffer);
}

/***************************************************************************************************************************************/
void termination(int sockfd,char s[INET6_ADDRSTRLEN])
{
	     strcat(s,"client want to terminate\n");
             printf("%s",s);					
}