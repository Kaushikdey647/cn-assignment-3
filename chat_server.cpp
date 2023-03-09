#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <pthread.h>
#include <csignal>
using namespace std;
#define MAX 1000

queue<int> clients;

// client params
int client_status[MAX];
int client_pairups[MAX];

// 
pthread_mutex_t mylock = PTHREAD_MUTEX_INITIALIZER;

// 
pthread_mutex_t qlock = PTHREAD_MUTEX_INITIALIZER;

class server
{
    public:
	
		// Member variables

		int port; // port number

		int server_socket_fd; // server socket file descriptor

		int connection_id; // connection id

		int bindid; // bind id

		int listenid; // listen id

		int connfd; // connection file descriptor

		struct sockaddr_in server_addr, client_addr; // server and client address
		
		// Constructor
		server(char* argv[])
		{	
			// Get the port number
			port = atoi(argv[1]);

			// Initialize the client status and client pairups
			for(int i=0; i<MAX; i++){
				client_status[i] = -1;
				client_pairups[i] = -1;
			}
		}
		
		/**
		 * @brief Get the Socket Number object
		 * 
		 * @return void
		 */
		void initialize_socket_fd();
		
		/**
		 * @brief initialize the server address object and bind it to the socket
		 * 
		 * @return void
		 */
		void bind_socket();

		/**
		 * @brief Listen for the client connections
		 * 
		 * @return void
		 */
		void listen_for_connections();

		/**
		 * @brief Accept the client connection
		 * 
		 * @return void
		 */
		void accept_client_connection();

		/**
		 * @brief Read the client message
		 * 
		 * @param a : client id
		 * @return string 
		 */
		string read_from_client(int a);

		/**
		 * @brief Write the message to the client
		 * 
		 * @param s : message
		 * @param a : client id
		 * @return void 
		 */
		void write_to_client(string s, int a);

		/**
		 * @brief Close the server
		 * 
		 * @param a : client id
		 * @return void 
		 */
		void detach_client(int a)
		{
			close(a);
		}
} s; 

void signal_handler(int signum) 
{
	for(int i=0; i<MAX; i++)
	{
		if(client_status[i] != -1)
		{
			s.write_to_client("OUTAGE", i);
		}
	}
    exit(signum);  
}

string encoding(int connfd)
{
	string encode = "";
	for(int i=0; i<MAX; i++)
	{
		if(client_status[i] == 0)
			encode = encode + "0 "; // FREE
		else if(client_status[i] == 1)
			encode = encode + "1 "; // BUSY
		else if(client_status[i] == -1)
			encode = encode + "-1 "; // DISCONNECTED
	}
	return encode;
}

void logs(string s, int connfd)
{
	cout << "Client ID " << connfd << " requested for " << s << " command" << endl;
}

string parseMessage(string s)
{
	stringstream ss(s);
	char delim = ' ';
	string item;

	getline(ss, item, delim);
	string ans = "";
	while(getline(ss, item, delim))
		ans = ans + item;
	
	return ans;
}

void *handler(void *arg)
{
	pthread_mutex_lock(&qlock);
	int connfd = clients.front();
	clients.pop();
	pthread_mutex_unlock(&qlock);
	
	int flag = 0;
	
	while(1)
	{
		if(flag == 0)
		{
			cout << "Client ID " << connfd << " joined" << endl;
			cout << string(50, '-') << endl;
			flag = 1;
			client_status[connfd] = 0; // FREE and connected to the server
		}
		

        string client = s.read_from_client(connfd);

		pthread_mutex_lock(&mylock);
		cout << "Handler Locked using Mutex" << endl;
		if(client.compare("#CLOSE#") == 0)
		{
			s.write_to_client("ABORT", connfd);
			cout << "Client ID " << connfd <<" exits" << endl;
			
			// When does the disconnection happens ?

			if(client_status[connfd] == 1)
			{
				// Disconnect the client (Critical Section?)
				int connfd2 = client_pairups[connfd];
				client_status[connfd] = -1;
				client_status[connfd2] = 0;
				client_pairups[connfd2] = -1;
				client_pairups[connfd] = -1;
				s.write_to_client("TERMINATED", connfd2);
			}
			else 
			{
				client_status[connfd] = -1; // Disconnected from the server
				client_pairups[connfd] = -1;
			}
			s.detach_client(connfd);
			cout << "Handler unlocked using Mutex" << endl;
			cout << string(50, '-') << endl;
			pthread_mutex_unlock(&mylock);
			pthread_exit(NULL);
			return NULL;
		}

		if(client_status[connfd] == 1)
		{
			if(client == "#GOODBYE#")
			{
				// Disconnect the client (Critical Section?)
				
				int connfd2 = client_pairups[connfd];
				cout << "Client ID " << connfd << " and Client ID " << connfd2 << " are now disconnected" << endl;
				client_status[connfd] = 0;
				client_status[connfd2] = 0;
				client_pairups[connfd2] = -1;
				client_pairups[connfd] = -1;

				s.write_to_client("TERMINATED", connfd);
				s.write_to_client("TERMINATED", connfd2);
			}
			else 
			{
				logs("SEND MESSAGE", connfd);
				s.write_to_client("SEND " + client, client_pairups[connfd]);
			}
			cout << "Handler unlocked using Mutex" << endl;
			cout << string(50, '-') << endl;
			pthread_mutex_unlock(&mylock);
			continue;
		}

		// Parsing of the string for command - GET, CONNECT, SEND
		vector<string> process;
		stringstream ss(client);
		char delim = ' ';
		string item, output, command;

		getline(ss, item, delim);
		command = item;

		if(command == "GET")
		{
			// Format: GET
			logs(command, connfd);

			// Encoding of the message from server to client
			string encode = encoding(connfd);
			s.write_to_client("GET " + encode, connfd);
		}
		else if(command == "CONNECT")
		{
			// Format: CONNECT <partner id>
			logs(command, connfd);

			getline(ss, item, delim);
			int connfd2 = stoi(item);
			
			if(client_status[connfd2] == -1)
				s.write_to_client("CONNECT OFFLINE", connfd);
			else if(connfd2 == connfd)
				s.write_to_client("CONNECT SELF", connfd);
			else if(client_status[connfd2] == 1 && client_pairups[connfd2] != connfd)
				s.write_to_client("CONNECT BUSY", connfd);
			else if(client_status[connfd2] == 1 && client_pairups[connfd2] == connfd)
				s.write_to_client("CONNECT TALK", connfd);
			else
			{
				// Make the connection between the two (Critical Section ?)
				// This means that I have to inform the other client and make it listen
				client_status[connfd] = 1;
				client_status[connfd2] = 1;
				client_pairups[connfd] = connfd2;
				client_pairups[connfd2] = connfd;

				cout << "Client " << connfd << " and Client " << connfd2 << " are connected" << endl;

				string a = "CONNECT SUCCESS " + to_string(connfd2);
				string b = "CONNECT SUCCESS% " + to_string(connfd);
				s.write_to_client(a, connfd);
				s.write_to_client(b, connfd2);
			}
		}
		else
		{
			// Invalid command
			s.write_to_client("INVALID COMMAND", connfd);
		}
		cout << "Handler unlocked using Mutex" << endl;
		cout << string(50, '-') << endl;
		pthread_mutex_unlock(&mylock);

	}
	s.detach_client(connfd);
	pthread_exit(NULL);
} 



int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        cout << "Format: ./chat_server <PORT NUMBER>" << endl;
        exit(0);
    } 

	// signal handling
    signal(SIGINT, signal_handler);

	server s(argv);

    s.initialize_socket_fd();
    if(s.server_socket_fd < 0)
	    exit(0);   
    s.bind_socket();
    if(s.bindid < 0)
        exit(0);
    s.listen_for_connections();
    if(s.listenid != 0)
        exit(0);
	cout << string(50, '-') << endl;

    while(1)
    {
    	// accepting the client
        s.accept_client_connection();

    	if (s.connfd < 0) 
        	continue;
        
        clients.push(s.connfd);
    	pthread_t t; 
    	pthread_create(&t, NULL, handler, NULL);
    }

    s.detach_client(s.server_socket_fd);
}

void server::initialize_socket_fd()
{
	server_socket_fd = socket(
		AF_INET, // IPv4
		SOCK_STREAM, // TCP
		0 // Default protocol
	);

	if(server_socket_fd < 0) 
	{
		cout << "[ERROR]: UNABLE TO CREATE SOCKET, EXITING....." << endl;
		exit(0);
	}
}

void server::bind_socket()
{
	//clear the serv_addr
	bzero(
		(sockaddr_in *) &server_addr, // cast to sockaddr_in
		sizeof(server_addr) // size of serv_addr
	);

	server_addr.sin_family = AF_INET ;
	server_addr.sin_addr.s_addr = htons(INADDR_ANY);
	server_addr.sin_port = htons(port);
	
	bindid = bind(server_socket_fd, (struct sockaddr*) &server_addr, sizeof(server_addr));
	if(bindid < 0)
	{ 
		cout << "[ERROR]: UNABLE TO BIND SOCKET, EXITING....." << endl;
		exit(0);
	}
}


void server::listen_for_connections()
{
	listenid = listen(server_socket_fd, 20);
	if(listenid != 0)
	{
		cout << "server listen failed." << endl;
		exit(0);
	}
	else 
		cout << "server is listening." << endl;
}

void server::accept_client_connection()
{
	socklen_t clilen = sizeof(client_addr);
	connfd = accept(server_socket_fd, (struct sockaddr *) &client_addr, &clilen);
	if (connfd < 0) 
	{
		printf("server accept failed...\n");
		exit(0);
	}
	else
		printf("server-client connection established. !!\n");
}

string server::read_from_client(int a)
{	
	// Read from the server 
	char ip[MAX];
	bzero(ip, sizeof(ip));
	
	read(a, ip, 8 * sizeof(ip));
	
	string ans(ip);
	return ans;
}

void server::write_to_client(string s, int a)
{
	// Write back to the server
	char *ptr = &s[0];
	write(a, ptr, 8 * sizeof(s));
}