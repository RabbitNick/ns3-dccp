#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <errno.h>


#include <syslog.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <linux/dccp.h>

#include <stdio.h>

#include <fcntl.h>

#include <stdlib.h>

struct tcp_info tcp_info;



#define SOL_DCCP 269



int dccp_get_mtu_size(int sock)
 {

  int new_size = 1440;
  int ret = 0;
  //ret = setsockopt(sock, SOL_DCCP, DCCP_SOCKOPT_PACKET_SIZE,
  //                         &new_size, sizeof(new_size));  
  if (ret >= 0)
  {
    std::cout << "Can set mtu_size: " << ret << std::endl;
  }

         int mtu_size = 0;
         int len = sizeof(mtu_size);

          ret = getsockopt(sock, SOL_DCCP, DCCP_SOCKOPT_GET_CUR_MPS,
                              &mtu_size, (socklen_t*)&len) ;
          printf("mtu_size: %d\n", mtu_size);
         return mtu_size;
 }


int getCCID(int sock)
{
         int ccid;
         int len = sizeof(ccid);

         int ret = getsockopt(sock, SOL_DCCP, DCCP_SOCKOPT_TX_CCID,
                              &ccid, (socklen_t*)&len);
          dccp_get_mtu_size(sock);
         //printf("CCID: %d\n", (uint8_t)ccid);
          std::cout << "CCID: " << ccid << std::endl;

  uint8_t ccids[4] = {0,0,0,0};
  socklen_t  slen = sizeof(ccids);

  ret = getsockopt(sock, SOL_DCCP, DCCP_SOCKOPT_AVAILABLE_CCIDS,
                              &ccids, (socklen_t*)&slen);

  for (int i = 0; i < 4; ++i)
  {
    printf("CCID %d : %d\n",i, ccids[i] );
  }

}

int setCCID(int sock, unsigned char ccid)
{

  //getCCID(sock);
  uint8_t CCID = ccid;
  int ret =  setsockopt(sock, SOL_DCCP, DCCP_SOCKOPT_CCID, &CCID, sizeof (CCID) );
  if (ret < 0)
  {
    std::cout << "Can not set CCID: " << ret << std::endl;
  }


  return ret;
}

int main (int argc, char *argv[])
{
  int sock;
  sock = socket (PF_INET, SOCK_DCCP, IPPROTO_DCCP);

  // getCCID(sock);

  //setCCID(sock, 3);
  // getCCID(sock);
  getCCID(sock);



  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons (2000);



  // int flag = fcntl(sock, F_GETFL, 0);
  // fcntl(sock, F_SETFL, flag & ~O_NONBLOCK);


  struct hostent *host = gethostbyname (argv[1]);
  memcpy (&addr.sin_addr.s_addr, host->h_addr_list[0], host->h_length);

  char *ip = inet_ntoa(addr.sin_addr);

  std::cout <<   "server IP : " << ip  << std::endl;

  int result;

  result = connect (sock, (const struct sockaddr *) &addr, sizeof (addr));
  if (result < 0)
    {
      printf ("connect errno=%d\n", errno);
      std::cout << strerror(errno) << std::endl;
      exit(-1);
      return -1;
    }



  uint8_t buf[1024];

  memset (buf, 0x66, 20);
  memset (buf + 20, 0x67, 1004);
  ssize_t tot = 0;
   fd_set w_fds;
   FD_ZERO (&w_fds);
   FD_SET (sock, &w_fds);



  //for (uint32_t i = 0; i < 100000; i++)
  
  while(1)
    {
      ssize_t n = 1024;
      while (n > 0)
        {
          result = select (sock + 1, NULL, &w_fds, NULL, NULL);
          if (result == 0)
            {
              std::cout << "timeout" << std::endl;
              continue;
            }
          if (result < 0)
            {
              if (errno == EINTR || errno == EAGAIN)
                {
                  std::cout << "result < 0: " << errno << std::endl;
                  continue;
                }
              perror ("select");
              break;
            }
          if (!FD_ISSET (sock, &w_fds))
            {
              std::cout << "fd isn't set" << std::endl;
              continue;
            }
         // std::cout << "MTU: " << dccp_get_mtu_size(sock) << std::endl;
          //ssize_t e  = write (sock, &(buf[1024 - n]), n);
            ssize_t e = send(sock, &(buf[1024 - n]), n, 0);
          if (e < 0)
            {

              if (errno == EINTR || errno == EAGAIN)
                {
                  std::cout << "result < 0: " << errno << std::endl;
                  usleep(1000);
                  continue;
                }              
              std::cout << "e < 0 : " << strerror (errno) << std::endl;
              break;
            }
          if (e < n)
            {
              //  sleep (1);
              std::cout << "e < n : " << e << "<" << n << std::endl;
            }
          n -= e;
          tot += e;
          
         // usleep(1000000);

        }

          std::cout << "write: " << n << "  total: " << tot <<std::endl;
    //   usleep(100000);
      //sleep (1);
        //usleep(100000);
      // if (tot >= 1024*1000)
      // {
      //   break;
      // }
    }

//   while(1)
//     {
//       ssize_t n = 1024;
//       while (n > 0)
//         {
//           ssize_t e  = write (sock, &(buf[1024 - n]), n);
//           if (e < 0)
//             {
//              std::cout << "e < 0 : " << strerror (errno) << std::endl;

//               break;
//             }
//           if (e < n)
//             {
//               //  sleep (1);
//               std::cout << "e < n : " << e << "<" << n << std::endl;
//             }
//           n -= e;
//           tot += e;
//          // usleep(1000000);
//         }
//        if (tot >= 1024*1000)
//        {
//          break;
//        }
// //      std::cout << "write: " << tot << std::endl;
//       std::cout << "write: " << n << "  total: " << tot <<std::endl;
//       //usleep(100000);
//      //  sleep (1);
//     }      

  std::cout << "did write all buffers total:" << tot << std::endl;

  //  close (sock);

  return 0;
}
