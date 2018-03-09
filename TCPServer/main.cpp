#include <stdio.h>
#include "TestServer.h"

int main(int argc, char *argv[])
{
	TestServer *Srv = NULL;

	Srv = new TestServer(44444);
	Srv->ServerLoop();

	delete Srv;

	return 0;
}