#include "compare.h"

namespace fs = boost::filesystem;

bool HashBlock::operator==(const HashBlock& other) const 
{
	return data == other.data;
}
bool HashBlock::operator!=(const HashBlock& other) const 
{
	return !(*this == other);
}

bool CompareFileHash::operator==(const CompareFileHash& other) const
{
	return fs::equivalent(path, other.path);
}

size_t CompareFileHash::get_hash_data_size() const
{
	return hash_data.size();
}

uintmax_t CompareFileHash::get_file_size() const
{
	return file_size;
}

fs::path CompareFileHash::get_path() const
{
	return path;
}

void CompareFileHash::open_handle()
{
	if(!file_handle) 
	{
		file_handle = std::make_unique<std::ifstream>(get_path().string());
		file_handle.get()->seekg(hash_data.size() * block_size);
	}
}

void CompareFileHash::close_handle()
{
	if(file_handle != nullptr) 
	{
		file_handle->close();
		delete file_handle.release();
	}
}

std::unique_ptr<char[]> CompareFileHash::get_next_block()
{
	open_handle();
	auto buffer = std::make_unique<char[]>(block_size);
	file_handle->read(buffer.get(), block_size);
	return buffer;
}

void CompareFileHash::calc_next_hash()
{
	hash_data.emplace_back(HashBlock{hasher->Hash(get_next_block().get(), block_size)});
}

HashBlock CompareFileHash::calc_hash_block(size_t addr)
{
	while(hash_data.size() <= addr) 
		calc_next_hash();	
	return hash_data.back();
}

HashBlock CompareFileHash::get_hash_block(size_t addr)
{
	if(addr >= block_count)
		throw;
	if(addr < hash_data.size())
		return hash_data[addr];
	else
		return calc_hash_block(addr);
}

bool CompareFileHash::equal_hash_file(CompareFileHash& other)
{
	if(this->get_file_size() != other.get_file_size())
		return false;
	for(size_t i = 0; i < block_count; ++i) 
	{
		if(get_hash_block(i) != other.get_hash_block(i)) 
		{
			close_handle();
			other.close_handle();
			return false;
		}
	}
	close_handle();
	other.close_handle();
	return true;
}

namespace std
{
	template <>
	struct hash<CompareFileHash>
	{
		size_t operator()(const CompareFileHash& obj) const {
			return hash<string>()(obj.get_path().string());
		}
	};
} 
