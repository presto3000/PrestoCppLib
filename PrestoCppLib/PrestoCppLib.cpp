// PrestoCppLib.cpp : Defines the entry point for the application.
//

#include "PrestoCppLib.h"
#include <presto/hashing/file_hash.hpp>

using namespace std;

int main()
{
	cout << "Hello PrestoLib." << endl;

	std::string hash = hashing::sha256File("TestFile1.txt");
	std::string hash2 = hashing::sha256File("TestFile1_copy.txt");
	std::cout << "SHA-256: " << hash << "\n";
	std::cout << "SHA-256 (copy): " << hash2 << "\n";

	return 0;
}
