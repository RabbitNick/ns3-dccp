#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <iostream>
#include <arpa/inet.h>
#include <linux/dccp.h>

#include <sys/time.h>

#include <stdio.h>


#define SERVER_PORT 2000
#define SOL_DCCP 269




int getCCID(int sock)
{
         int ccid;
         int len = sizeof(ccid);

         int ret = getsockopt(sock, SOL_DCCP, DCCP_SOCKOPT_RX_CCID,
                              &ccid, (socklen_t*)&len);
          //dccp_get_mtu_size(sock);
         printf("CCID: %d\n", ccid);

  uint8_t ccids[4] = {0,0,0,0};
  socklen_t  slen = sizeof(ccids);

  ret = getsockopt(sock, SOL_DCCP, DCCP_SOCKOPT_AVAILABLE_CCIDS,
                              &ccids, (socklen_t*)&slen);

  for (int i = 0; i < 4; ++i)
  {
    printf("CCID %d : %d\n",i, ccids[i] );
  }

  return 0;

}

int setCCID(int sock, unsigned char ccid)
{
  uint8_t CCID = ccid;
  int ret =  setsockopt(sock, SOL_DCCP, DCCP_SOCKOPT_CCID, &CCID, sizeof (CCID) );
  if (ret != 0)
  {
    std::cout << "Can not set CCID: " << ret << std::endl;
  }
  return ret;
}




int main (int argc, char *argv[])
{
  int sock;
  sock = socket (PF_INET, SOCK_DCCP, IPPROTO_DCCP);

  std::cout << " sock success "  << std::endl;


 // setCCID(sock, 3);
//  std::cout << " ccid setting success "  << std::endl;

  getCCID(sock);
 // getCCID(sock);

  std::cout << " get ccid "  << std::endl;


  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons (SERVER_PORT);
  addr.sin_addr.s_addr = INADDR_ANY;



  int status;
  status = bind (sock, (const struct sockaddr *) &addr, sizeof(addr));
  
  std::cout << " bing success "  << std::endl;


  status = listen (sock, 1);

  std::cout << " listen success "  << std::endl;




  int fd = accept (sock, 0, 0);
  std::cout << " accept -> " << fd << std::endl;




/*
  uint8_t buf[10240];

  memset (buf, 0, 10240);
  ssize_t tot = 0;
  double tot_time = 0;



 // std::cout << "here" << std::endl;

  while(1)
    {
      ssize_t n = 10240;
      struct  timeval  start;
      gettimeofday(&start,NULL);
      static ssize_t old_tot = 0;
      ssize_t incbytes = 0;      
      while (n > 0)
        {
          ssize_t bytes_read = recv(fd,&buf[10240 - n], n,0 );//read (fd, &buf[10240 - n], n);

          if (bytes_read > 0)
            {
              n -= bytes_read;

              //std::cout << "read:" << bytes_read << " n:" << n << std::endl;

              tot += bytes_read;
            }
          else
            {
              break;
            }

        }


        incbytes = tot - old_tot;
        old_tot = tot;
        struct  timeval stop;
        gettimeofday(&stop,NULL);
        double during =  1000000 * (stop.tv_sec-start.tv_sec) + stop.tv_usec-start.tv_usec;
        during = during / 1000000;
        tot_time = tot_time + during;

      // if (n <= 0)
      // {
      //   break;
      // }
  

     //   std::cout << "time: " << during << " s   rate: " << incbytes/during << " byte/s" << std::endl; 
     //   std::cout << "tot: " << tot << std::endl;
        if (tot >= 1024000)
        {

          break;
        }

      //   sleep (1);
    }

  std::cout << "did read all buffers tot:" << tot << std::endl;
  std::cout << "total time :" << tot_time << " s " << "  rate: " << tot / tot_time << " byte/s" << std::endl;
*/
  close (sock);
  close (fd);

  return 0;
}
