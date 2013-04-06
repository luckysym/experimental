#include "../sockdef.h"
#include "../localchannel.h"
#include <iostream>
#include <unistd.h>

#define  SERVER_PATH  "/tmp/localchannel_server"
#define  CLIENT_PATH  "/tmp/localchannel_client"


int child_main(int argc, char *argv[])
{

	using namespace std;
	using namespace sym;

	CLocalChannel chc;
	int ret = 0;

	::sleep(1);
	chc.Bind(CLIENT_PATH);
	if ( ret ) {
		cout<<"error: chc.Bind() = "<<strerror(ret)<<endl;
		return 0;
	}
	chc.Connect(SERVER_PATH);
	ret = chc.Send("Hello, Server", 14);
	if ( ret < 0 ) {
		cout<<"Error: chc.Send() = "<<strerror(ret)<<endl;
		return 0;
	} else if ( ret == 0 ) {
		cout<<"INFO: CHC nothing sent"<<endl;
		return 0;
	}
	char buf[64];
	ret = chc.Recv(buf, 64);
	if ( ret < 0) {
		cout<<"error: chc.Recv() = "<<strerror(ret)<<endl;
		return 0;
	} else if (ret == 0) {
		cout<<"info: chc: nothing received"<<endl;
		return 0;
	}
	cout<<"chc recv: "<<buf<<endl;

	chc.Close();


	return 0;
}

int main(int argc, char *argv[])
{
	using namespace sym;
	using namespace std;

	int ret = fork();
	if ( ret == 0 ) {
		child_main(argc, argv);
		unlink(CLIENT_PATH);
	} else {
		CLocalChannel chs;
		ret = chs.Bind("/tmp/localchannel_server");
		::sleep(1);
		if ( ret ) {
			cout<<"error: chs.Bind() = "<<strerror(ret)<<endl;
			return 0;
		}
		char buf[64];
		ret = chs.Recv(buf, 64);
		if ( ret < 0) {
			cout<<"error: chs.Recv() = "<<strerror(ret)<<endl;
			unlink(SERVER_PATH);
			return 0;
		} else if (ret == 0) {
			cout<<"info: chs: nothing received"<<endl;
			unlink(SERVER_PATH);
			return 0;
		}
		cout<<"CHS: Recv messge: "<<buf<<endl;
		chs.Connect("/tmp/localchannel_client");
		ret = chs.Send("Hello, client", 14);
		if ( ret < 0 ) {
			cout<<"Error: chs.Send() = "<<strerror(ret)<<endl;
			unlink(SERVER_PATH);
			return 0;
		} else if ( ret == 0 ) {
			cout<<"INFO: CHS nothing sent"<<endl;
			unlink(SERVER_PATH);
			return 0;
		}
		chs.Close();
		unlink(SERVER_PATH);
	}
	return 0;
}

