#  Implementation Of File Sharing Over NFS

Video Blogging & Implementation Of File Sharing Over NFS

Software Requirements:
Operating System: Linux ( Ubuntu )
Language Used: C  


INSTALLATION PROCEDURE:

STEP 1.

    1. In this project we uses 5 computers.
                 
        a. 2 computers for client
        b. 2 computers for server
        c. 1 computer for Sharedstorage

    2. All the computers should be connected on a common network.


STEP 2.

    { This process will be on the only that computer which u want to make as a common storage }

                                          
    ------------------------------ NFS SERVER----------------------------------

    1. Check the ipaddress of all computers by using command
       ifconfig

    2. Create the NFS Enviornment between 3 computers (one for Sharedstorage & 2 server)
                  
        a. Open the terminal.

        b. Check if NFS is already installed on the three computers or not using the command 
          /etc/init.d/nfs start
                        

          if (NFS is not installed){
            
            install the NFS by internet then proceeed further
          
          }
       
        c.  Make a directory on the comman storage by using command 
            mkdir sharedstorage
                 
        d.  create a file.
            vim  /etc/exports
                     
        e.  Inside file write
            /root/sharedstorage *(rw,sync)
             
        f.  Save this file and quit
            :wq

        g.  Restart nfs
            /etc/init.d/nfs restart
                  
        h.  NFS configration is complete.

        i.  Stop the  firewalls by using the command
            service iptables stop

            //ubuntu
            sudo ufw disable

STEP 3.               

      [ Instructions for Server computers]

        ----------------NFS CLIENT ------------------

      1. Sharedstorage is NFS server & 2 servers are NFS client.

      2. Mount the NFS client computer to NFS server.

          a.  First stop the firewall by using the command
              service iptables stop

          b.  Mount the computers by using command
              mount -t nfs -o nfsvers=3 ipaddress :/root/sharedstorage /root/temp
                  
          c.  Use the hard disk of a NFS server as a common storage.

STEP 4.

      1.  Compile and Run the server program on the NFS client computer                   
          gcc server.c         
          ./a.out

      2.  Comiple and Run the client program on the client computer 
          gcc client.c
          ./a.out ipadress  (EX- ./a.out 127.0.0.1)


      3.  These option will be display on client side.
                    
          a. Uploading
          b. Downloading
          c. Delete
          d. Exit
          printf(enter the choice);

      4.  According to the Client Program will Execute :
          
          a.  Video will be stored on the common storage.
          b.  Client can download the video from the common storage.
          c.  Multiple user can upload and download the videos at the same time.
          d.  Self user can delete the video.
          e.  If enter 4 by the user will be disconnect from the server.
