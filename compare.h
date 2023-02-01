#pragma once
#include <boost/filesystem.hpp>
#include <iostream>
#include <vector>
#include <fstream>
#include "hash.h"

namespace fs = boost::filesystem;

struct HashBlock	// Single hash block
{
	std::vector<unsigned int> data;
	bool operator==(const HashBlock& other) const;
	bool operator!=(const HashBlock& other) const;
};

class CompareFileHash
{
public:
	bool in_result = false;	
	//const size_t block_count;						// Number of hash blocks of the file
	size_t block_count;						// Number of hash blocks of the file

	CompareFileHash(fs::path path, uintmax_t size, uintmax_t hash_blocksize, IHasher* hasher) :
		path(path), 
		file_size(size), 
		block_size(hash_blocksize), 
		hasher(hasher),
		block_count((size + hash_blocksize - 1) / hash_blocksize) {};

	bool operator==(const CompareFileHash& other) const;	// const comparsion for unordered_set
	size_t get_hash_data_size() const;				// getter for size of calculated hash
	uintmax_t get_file_size() const;				// getter for filesize
	fs::path get_path() const;						// getter for path
	void open_handle();								// Open a file and jump to the location of the last uncommitted block	
	void close_handle();							// Close file
	std::unique_ptr<char[]> get_next_block();		// Get the next block from a file
	void calc_next_hash();							// Calculate the next hash block
	HashBlock calc_hash_block(size_t addr);			// Calculate hash up to block N inclusive	
	HashBlock get_hash_block(size_t addr);			// get N-th HashBlock	
	bool equal_hash_file(CompareFileHash& other);	// Compare by hash with other File
private:
	fs::path path;
	uintmax_t file_size;	
	std::vector<HashBlock> hash_data;				// Вектор с информацией по рассчитанным хэшам 
	uintmax_t block_size;
	IHasher* hasher;
	std::unique_ptr<std::ifstream> file_handle;
};
