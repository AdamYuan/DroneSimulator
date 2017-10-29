#include "Application.hpp"

int main(int argc, char **argv)
{
	try {
		Application App(argc, argv);
		App.Run();
	}
	catch (std::exception &e)
	{
		std::cout << e.what() << std::endl;
	}

	return 0;
}