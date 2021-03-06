#include "splay_tree.hpp"
#include <cstdlib>
#include <thread>
#include <mutex>
#include <atomic>
extern "C" {
#include <unistd.h>
#include <fcntl.h>
}

int french_tree(void)
{
	SplayTree<std::string> arbre;
	arbre.add("const mot_simple<char> &valeur");
	arbre.add("banane");
	arbre.add("fraise");
	arbre.add("kiwi");
	arbre.add("sirop d'érable");
	arbre.add("crêpe");
	arbre.structure_print();
	return EXIT_SUCCESS;
}

int main(void)
{
	SplayTree<std::int32_t> bytes;
	std::mutex bytes_lock;
	std::atomic<bool> shutdown(false);
	std::atomic<std::uint8_t> some_byte(0);
	std::thread t1([&]() -> int {
		// Open up urandom
		int handle = open("/dev/urandom", O_RDONLY);
		if (handle == -1) {
			std::cerr
				<< "Error. Unable to open urandom for reading.\n";
			return EXIT_FAILURE;
		}
		// Keep adding bytes to the splay tree until we are told to shutdown.
		while (!(shutdown.load())) {
			std::uint8_t read_byte;
			if (read(handle, &read_byte, 1) < 0) {
				std::cerr << "Error reading from urandom.\n";
				return EXIT_FAILURE;
			}
			some_byte.store(read_byte);
			{ // Mutex locked scope
				std::lock_guard<std::mutex> lock(bytes_lock);
				bytes.add(read_byte);
			}
		}
		close(handle);
		return EXIT_SUCCESS;
	});
	while (true) {
		bool has;
		{ // Mutex locked scope
			std::lock_guard<std::mutex> lock(bytes_lock);
			has = bytes.contains(42);
		}
		if (has) {
			shutdown.store(true);
			break;
		} else {
			{ // Mutex locked scope
				std::lock_guard<std::mutex> lock(bytes_lock);
				bytes.remove(some_byte.load());
			}
		}
	}
	t1.join();
	bytes.structure_print();
	std::cout << "Ending Tree size (maximum tree size is 256): "
		  << bytes.get_size() << "\n";
	return french_tree();
}
