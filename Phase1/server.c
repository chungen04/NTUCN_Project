#include <unistd.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <fcntl.h>

#define ERR_EXIT(a) do { perror(a); exit(1); } while(0)

typedef struct {
    char hostname[512];  // server's hostname
    unsigned short port;  // port to listen
    int listen_fd;  // fd to wait for a new connection
} server;

typedef struct request {
    char host[512];  // client's host
    int conn_fd;  // fd to talk with client
    char buf[512];  // data sent by/to client
    size_t buf_len;  // bytes used by buf
    int id;
    int wait_for_write;  // used by handle_read to know if the header is read or not.
} request;

server svr;  // server
request* requestP = NULL;  // point to a list of requests
int maxfd;  // size of open file descriptor table, size of request list

const char* ask_for_id = "[server] Please enter text:\n";
const unsigned char IAC_IP[3] = "\xff\xf4";

static void init_server(unsigned short port);
// initailize a server, exit for error

static void init_request(request* reqP);
// initailize a request instance

static void free_request(request* reqP);
// free resources used by a request instance

int handle_read(request* reqP) {
    /*  Return value:
     *      1: read successfully
     *      0: read EOF (client down)
     *     -1: read failed
     */
    int r;
    char buf[512] = {};

    // Read in request from client
    r = read(reqP->conn_fd, buf, sizeof(buf));
    // fprintf(stderr, "buffer: %s\n", buf);
    if (r < 0) return -1;
    if (r == 0) return 0;
    char* p1 = strstr(buf, "\015\012");
    int newline_len = 2;
    if (p1 == NULL) {
       p1 = strstr(buf, "\012");
        if (p1 == NULL) {
            if (!strncmp(buf, IAC_IP, 2)) {
                // Client presses ctrl+C, regard as disconnection
                fprintf(stderr, "Client presses ctrl+C....\n");
                return 0;
            }
            ERR_EXIT("this really should not happen...");
        }
    }
    size_t len = p1 - buf + 1;
    memmove(reqP->buf, buf, len);
    reqP->buf[len - 1] = '\0';
    reqP->buf_len = len-1;
    return 1;
}

int is_int(char* p){
    if(strcmp(p, "-0") == 0) return 1;
    int num = atoi(p);
    char str[32];
    sprintf(str, "%d", num);
    return strcmp(str, p) == 0;
}

int main(int argc, char** argv) {

    // Parse args.
    if (argc != 2) {
        fprintf(stderr, "usage: %s [port]\n", argv[0]);
        exit(1);
    }

    struct sockaddr_in cliaddr;  // used by accept()
    int clilen;

    int conn_fd;  // fd for a new connection with client
    int file_fd;  // fd for file that we open for reading
    char buf[512] = {};
    
    // Initialize server
    init_server((unsigned short) atoi(argv[1]));

    // Loop for handling connections
    fprintf(stderr, "\nstarting on %.80s, port %d, fd %d, maxconn %d...\n", svr.hostname, svr.port, svr.listen_fd, maxfd);

    fd_set master_fds; 
    fd_set read_fds;
    FD_ZERO(&master_fds); // init
    FD_ZERO(&read_fds); // init

    FD_SET(svr.listen_fd, &master_fds); // add server.listen_fd to master

    maxfd = svr.listen_fd + 1;
    while (1) {
        // TODO: Add IO multiplexing
        
        memcpy(&read_fds, &master_fds, sizeof(master_fds));
        if(select(maxfd, &read_fds, NULL, NULL, NULL) < 0){
            fprintf(stderr, "exit due to select error");
            break;
        }
        
        //check every fds listening to
        for(int i = 0; i<maxfd; i++){
	        if(FD_ISSET(i, &read_fds)){ // select detects event
                if(i == svr.listen_fd){ // new connection
                    clilen = sizeof(cliaddr);
                    conn_fd = accept(svr.listen_fd, (struct sockaddr*)&cliaddr, (socklen_t*)&clilen);
                    if (conn_fd < 0) {
                        if (errno == EINTR || errno == EAGAIN) continue;  // try again
                        if (errno == ENFILE) {
                            (void) fprintf(stderr, "out of file descriptor table ... (maxconn %d)\n", maxfd);
                            continue;
                        }
                        ERR_EXIT("accept");
                    }
                    // start handle client
                    requestP[conn_fd].conn_fd = conn_fd;
                    if(conn_fd >= maxfd){
			            maxfd = conn_fd + 1;
                    }
                    write(conn_fd, ask_for_id, strlen(ask_for_id)); 
                    strcpy(requestP[conn_fd].host, inet_ntoa(cliaddr.sin_addr));
                    fprintf(stderr, "getting a new request... fd %d from %s\n", conn_fd, requestP[conn_fd].host);
                    FD_SET(conn_fd, &master_fds);
                }else{ // previous connection
                    int ret = handle_read(&requestP[i]); // parse data from client to requestP[conn_fd].buf
                    fprintf(stderr, "ret = %d\n", ret);
                    if (ret < 0) {
                        fprintf(stderr, "bad request from %s\n", requestP[i].host);
                        FD_CLR(requestP[i].conn_fd, &master_fds);
                        close(requestP[i].conn_fd);
                        free_request(&requestP[i]);
                        continue;
                    }
                    else if (ret == 0){
                        fprintf(stderr, "disconnected from %s\n", requestP[i].host);
                        FD_CLR(requestP[i].conn_fd, &master_fds);
                        close(requestP[i].conn_fd);
                        free_request(&requestP[i]);
                        continue;
                    }else{
                        fprintf(stderr, "[client]: %s\n[info]: fd %d from %s\n", requestP[i].buf, conn_fd, requestP[conn_fd].host);
                        strcpy(buf, requestP[i].buf);
                        sprintf(buf, "[server response] %s\n", requestP[i].buf);
                        write(requestP[i].conn_fd, buf, strlen(buf));
                        FD_CLR(requestP[i].conn_fd, &master_fds);
                        close(requestP[i].conn_fd);
                        free_request(&requestP[i]);
                        continue;
                    }
                }
            }
        }
    }
    free(requestP);
    close(file_fd);
    return 0;
}

// ======================================================================================================
// You don't need to know how the following codes are working

static void init_request(request* reqP) {
    reqP->conn_fd = -1;
    reqP->buf_len = 0;
    reqP->id = 0;
    reqP->wait_for_write = 0;
}

static void free_request(request* reqP) {
    init_request(reqP);
}

static void init_server(unsigned short port) {
    struct sockaddr_in servaddr;
    int tmp;

    gethostname(svr.hostname, sizeof(svr.hostname));
    svr.port = port;

    svr.listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (svr.listen_fd < 0) ERR_EXIT("socket");

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
    tmp = 1;
    if (setsockopt(svr.listen_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&tmp, sizeof(tmp)) < 0) {
        ERR_EXIT("setsockopt");
    }
    if (bind(svr.listen_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        ERR_EXIT("bind");
    }
    if (listen(svr.listen_fd, 1024) < 0) {
        ERR_EXIT("listen");
    }

    // Get file descripter table size and initialize request table
    maxfd = getdtablesize();
    requestP = (request*) malloc(sizeof(request) * maxfd);
    if (requestP == NULL) {
        ERR_EXIT("out of memory allocating all requests");
    }
    for (int i = 0; i < maxfd; i++) {
        init_request(&requestP[i]);
    }
    requestP[svr.listen_fd].conn_fd = svr.listen_fd;
    strcpy(requestP[svr.listen_fd].host, svr.hostname);

    return;
}
