build:
	rpcgen -C application.x
	cat application_svc_modified.c > application_svc.c
	g++ -o server server_src/* application_svc.c application_xdr.c -I /usr/include/tirpc/ -ltirpc -std=c++11
	g++ -o client client_src/* application_clnt.c application_xdr.c  -I /usr/include/tirpc/ -ltirpc -std=c++11
	
clean:
	rm -f client server application_svc.c application_clnt.c application_xdr.c application.h


