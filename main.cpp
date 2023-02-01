#include <boost/program_options.hpp>
#include <exception>
#include <iostream>
#include "finder.h"


void to_cout(const std::vector<std::string>& v)
{
    std::copy(v.begin(), v.end(), std::ostream_iterator<std::string>{
        std::cout, "\n"});
}

namespace std
{
	std::ostream& operator<<(std::ostream& os, const std::vector<std::string>& vec) 
	{
		for (const auto& elem : vec) 
			os << elem << " ";
		
		return os;
	}
}

namespace bpo = boost::program_options;

int main([[maybe_unused]] int argc, [[maybe_unused]] char const* argv[])
{
	try 
	{
		size_t depth			= 2;    // how deep to search
		size_t block_size       = 512;  // block size for hash calculation
		size_t min_file_size    = 1;    // minimum file size to be processed

		bpo::options_description desc_opt{ "Options" };
		desc_opt.add_options()
			("help,h", "Help screen")
			("dirs,D", bpo::value<std::vector<std::string>>()->multitoken(), "Directories to scan")
			("skip_dirs,S", bpo::value<std::vector<std::string>>()->multitoken()->default_value(std::vector<std::string>{}), "Skip directories to scan")
			("depth,d", bpo::value<size_t>(&depth), "Depth to scan")
			("min_file_size,M", bpo::value<size_t>(&min_file_size), "Minimum file size")
			("hash,H", bpo::value<std::string>()->default_value("crc32"), "A hashing algorithm crc32/md5/sha1")
			("file_masks", bpo::value<std::vector<std::string>>()->multitoken()->default_value(std::vector<std::string>{}), "Mask for files to scans")
			("block_size", bpo::value<size_t>(&block_size), "Block size in bytes")
			("unreg", "Unrecognized options");

		bpo::command_line_parser parser{ argc, argv };
		parser.options(desc_opt).allow_unregistered().style(
		bpo::command_line_style::default_style | bpo::command_line_style::allow_slash_for_short);
		bpo::parsed_options parsed_options = parser.run();
		
		bpo::variables_map vm;
		bpo::store(parsed_options, vm);
		bpo::notify(vm);

		if (vm.count("help"))
			std::cout << desc_opt << '\n';
		else if (vm.count("unreg"))
			to_cout(collect_unrecognized(parsed_options.options, bpo::exclude_positional)); 
		else if (vm.count("include_dir")) 
		{
			FindDuplicate findDuplicate(			
				vm["dirs"].as<std::vector<std::string> >(),
				vm["skip_dirs"].as<std::vector<std::string> >(),
				vm["file_masks"].as<std::vector<std::string> >(),
				vm["block_size"].as<size_t>(),
				vm["depth"].as<size_t>() + 1,
				vm["min_file_size"].as<uintmax_t>(),
				vm["hash"].as<std::string>());

			auto duplicates = findDuplicate.map_find_duplicates();
			for (const auto& i: duplicates)
			{
				std::cout << i.first << '\n';
				for (const auto& j : i.second) 
				{
					std::cout << j << '\n';
				}
				std::cout << std::endl;
			}
		}
	}
	catch (const bpo::error& ex) 
	{
		std::cerr << ex.what() << '\n';
	}

	return EXIT_SUCCESS;
}


