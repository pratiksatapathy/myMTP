#include "sgw_server.h"

int g_s11_server_threads_count;
int g_s1_server_threads_count;
int g_s5_server_threads_count;
vector<thread> g_s11_server_threads;
vector<thread> g_s1_server_threads;
vector<thread> g_s5_server_threads;
Sgw g_sgw;

void check_usage(int argc) {
	if (argc < 4) {
		TRACE(cout << "Usage: ./<sgw_server_exec> S11_SERVER_THREADS_COUNT S1_SERVER_THREADS_COUNT S5_SERVER_THREADS_COUNT" << endl;)
				g_utils.handle_type1_error(-1, "Invalid usage error: sgwserver_checkusage");
	}
}

void init(char *argv[]) {
	g_s11_server_threads_count = atoi(argv[1]);
	g_s1_server_threads_count = atoi(argv[2]);
	g_s5_server_threads_count = atoi(argv[3]);
	g_s11_server_threads.resize(g_s11_server_threads_count);
	g_s1_server_threads.resize(g_s1_server_threads_count);
	g_s5_server_threads.resize(g_s5_server_threads_count);
	g_sgw.initialize_kvstore_clients(g_s11_server_threads_count);//do need to change this for different threads
	signal(SIGPIPE, SIG_IGN);
}

void run() {
	int i;

	/* SGW S11 server */
	TRACE(cout << "SGW S11 server started" << endl;)
	g_sgw.s11_server.run(g_sgw_s11_ip_addr, g_sgw_s11_port);
	for (i = 0; i < g_s11_server_threads_count; i++) {
		g_s11_server_threads[i] = thread(handle_s11_traffic,i);
	}	

	/* SGW S1 server */
	TRACE(cout << "SGW S1 server started" << endl;)
	g_sgw.s1_server.run(g_sgw_s1_ip_addr, g_sgw_s1_port);
	for (i = 0; i < g_s1_server_threads_count; i++) {
		g_s1_server_threads[i] = thread(handle_s1_traffic,i);
	}

	/* SGW S5 server */
	TRACE(cout << "SGW S5 server started" << endl;)
	g_sgw.s5_server.run(g_sgw_s5_ip_addr, g_sgw_s5_port);
	for (i = 0; i < g_s5_server_threads_count; i++) {
		g_s5_server_threads[i] = thread(handle_s5_traffic,i);
	}

	/* Joining all threads */
	for (i = 0; i < g_s11_server_threads_count; i++) {
		if (g_s11_server_threads[i].joinable()) {
			g_s11_server_threads[i].join();
		}
	}
	for (i = 0; i < g_s1_server_threads_count; i++) {
		if (g_s1_server_threads[i].joinable()) {
			g_s1_server_threads[i].join();
		}
	}	
	for (i = 0; i < g_s5_server_threads_count; i++) {
		if (g_s5_server_threads[i].joinable()) {
			g_s5_server_threads[i].join();
		}
	}				
}

void handle_s11_traffic(int worker_id) {
	UdpClient pgw_s5_client;
	struct sockaddr_in src_sock_addr;
	Packet pkt;

	pgw_s5_client.set_client(g_sgw_s5_ip_addr);
	while (1) {
		g_sgw.s11_server.rcv(src_sock_addr, pkt);
		pkt.extract_gtp_hdr();
		switch(pkt.gtp_hdr.msg_type) {
		/* Create session */
		case 1:
			TRACE(cout << "sgwserver_handles11traffic:" << " case 1: create session" << endl;)
			g_sgw.handle_create_session(src_sock_addr, pkt, pgw_s5_client,worker_id);
			break;

			/* Modify bearer */
		case 2:
			TRACE(cout << "sgwserver_handles11traffic:" << " case 2: modify bearer" << endl;)
			g_sgw.handle_modify_bearer(src_sock_addr, pkt,worker_id);
			break;

			/* Detach */
		case 3:
			TRACE(cout << "sgwserver_handles11traffic:" << " case 3: detach" << endl;)
			g_sgw.handle_detach(src_sock_addr, pkt, pgw_s5_client,worker_id);
			break;

			/* For error handling */
		default:
			TRACE(cout << "sgwserver_handles11traffic:" << " default case:" << endl;)
		}		
	}
}

void handle_s1_traffic(int worker_id) {
	UdpClient pgw_s5_client;
	struct sockaddr_in src_sock_addr;
	Packet pkt;

	pgw_s5_client.set_client(g_sgw_s5_ip_addr);
	while (1) {
		g_sgw.s1_server.rcv(src_sock_addr, pkt);
		pkt.extract_gtp_hdr();
		switch(pkt.gtp_hdr.msg_type) {
		/* Uplink userplane data */
		case 1:
			TRACE(cout << "sgwserver_handles1traffic:" << " case 1: uplink udata" << endl;)
			g_sgw.handle_uplink_udata(pkt, pgw_s5_client,worker_id);
			break;

			/* For error handling */
		default:
			TRACE(cout << "sgwserver_handles1traffic:" << " default case:" << endl;)
		}		
	}		
}

void handle_s5_traffic(int worker_id) {
	UdpClient enodeb_client;
	struct sockaddr_in src_sock_addr;
	Packet pkt;

	enodeb_client.set_client(g_sgw_s1_ip_addr);
	while (1) {
		g_sgw.s5_server.rcv(src_sock_addr, pkt);
		pkt.extract_gtp_hdr();
		switch(pkt.gtp_hdr.msg_type) {
		/* Downlink userplane data */
		case 3:
			TRACE(cout << "sgwserver_handles5traffic:" << " case 3: downlink udata" << endl;)
			g_sgw.handle_downlink_udata(pkt, enodeb_client,worker_id);
			break;

			/* For error handling */
		default:
			TRACE(cout << "sgwserver_handles5traffic:" << " default case:" << endl;	)
		}		
	}			
}

int main(int argc, char *argv[]) {
	check_usage(argc);
	init(argv);cout<<"suc inint"<<endl;
	run();cout<<"suc inint2"<<endl;
	return 0;
}
