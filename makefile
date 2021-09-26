UDP_SRCS := \
	Encoder.cpp \
	UdpPeer.cpp

TCP_SERVER_SRCS := \
	Encoder.cpp \
	TcpServer.cpp

TCP_Client_SRCS := \
	Encoder.cpp \
	TcpClient.cpp

all: peer0 peer1 server client

peer0: ut_peer0.cpp test_vectors.cpp $(UDP_SRCS) *.hpp
	g++ -Wall -o peer0 ut_peer0.cpp test_vectors.cpp $(UDP_SRCS) -lpthread

peer1: ut_peer1.cpp test_vectors.cpp $(UDP_SRCS) *.hpp
	g++ -Wall -o peer1 ut_peer1.cpp test_vectors.cpp $(UDP_SRCS) -lpthread

server: ut_tcp_server.cpp test_vectors.cpp $(TCP_SERVER_SRCS) *.hpp
	g++ -Wall -o server ut_tcp_server.cpp test_vectors.cpp $(TCP_SERVER_SRCS) -lpthread

client: ut_tcp_client.cpp test_vectors.cpp $(TCP_Client_SRCS) *.hpp
	g++ -Wall -o client ut_tcp_client.cpp test_vectors.cpp $(TCP_Client_SRCS) -lpthread
