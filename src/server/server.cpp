#include "serverapp.h"


int main(int argc, char* argv[])
{
	will::ServerApp app(argc, argv);
	return app.run();
}
