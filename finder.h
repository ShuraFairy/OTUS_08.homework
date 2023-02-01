#pragma once

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/unordered_set.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "compare.h"
#include "hash.h"

namespace fs = boost::filesystem;

class FindDuplicate
{
public:
	FindDuplicate(std::vector<std::string> dirs,
				  std::vector<std::string> skip_dirs,
				  std::vector<std::string> vec_file_mask,
				  size_t block_size, 
				  size_t scan_depth, 
				  uintmax_t min_file_size, 
				  std::string hasher);
	std::unordered_map<std::string, std::vector<std::string>> map_find_duplicates();
	void set_dirs(std::vector<std::string> _dirs);
	void set_skip_dirs(std::vector<std::string> _dirs);
private:
  
	std::vector<fs::path> dirs;				// directories to scan
	std::vector<fs::path> skip_dirs;		// exclusion Directories	
	std::vector<boost::regex> vec_regex_filemasks;	// scan masks	
	size_t block_size;						// hash block size to compare
	size_t scan_depth;						// maximum scanning depth	
	uintmax_t min_file_size;				// minimum file size to scan	
	std::vector<CompareFileHash> info_scan_files;		// information on scanned files	
	std::unordered_set<std::string> files_canonical;	// duplicate Information	
	IHasher* hasher;						// class for caching files	
	void set_hasher(std::string hash_str);	// setting for a class objec	
	bool excluded_path(fs::path p);			// checking for Excluded Paths	
	bool filename_with_filemask(const std::string& filename);	// mask check	
	void add_file(fs::path p);				// adding a file to scan	
	void add_filtered_file(fs::path p);		// adding a file to scan with a mask
	void scan_path(fs::path p, size_t depth);	// recursive Scan
	void search();							// starting a scan
};
