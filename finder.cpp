#include "finder.h"

namespace fs = boost::filesystem;

std::vector<fs::path> vec_str_to_path(const std::vector<std::string>& other) 
{
	std::vector<fs::path> result;
	result.reserve(other.size());

	for(const auto& i : other) 
		result.emplace_back(fs::path(i));
	
	return result;
}

void FindDuplicate::set_hasher(std::string hash_str)
{
	if(hash_str == "md5") 
		hasher = new MD5Hasher();	
	else if(hash_str == "sha1") 
		hasher = new SHA1Hasher();	
	else 
		hasher = new CRC32Hasher();	
}

bool FindDuplicate::excluded_path(fs::path p)
{
	for(const auto& i : skip_dirs)
	{
		if(fs::equivalent(p, i)) 
			return true;
		
	}
	return false;
}

bool FindDuplicate::filename_with_filemask(const std::string& filename) 
{
	if(vec_regex_filemasks.size() == 0)
		return true;
	
	for(const auto& i : vec_regex_filemasks)
	{
		if(boost::regex_match(filename, i)) 
			return true;
		
	}
	return false;
}

void FindDuplicate::add_file(fs::path p) 
{
	if(fs::exists(p) && fs::is_regular_file(p)) 
	{
		if(files_canonical.find(fs::weakly_canonical(p).string()) == files_canonical.end()) 
		{
			files_canonical.insert(fs::weakly_canonical(p).string());
            info_scan_files.emplace_back(CompareFileHash(p.string(), fs::file_size(p), block_size, hasher));
		};
	}
}

void FindDuplicate::add_filtered_file(fs::path p) 
{
	if(filename_with_filemask(p.filename().string())) 
		add_file(p);
	
}

void FindDuplicate::scan_path(fs::path p, size_t depth) 
{
	if(fs::exists(p) && !excluded_path(p)) 
	{
		if(fs::is_regular_file(p)) 
		{
			if(fs::file_size(p) >= min_file_size) 
				add_filtered_file(p);
			
		}
		else if(fs::is_directory(p) && depth > 0) 
		{
			for(const fs::directory_entry& x : fs::directory_iterator(p)) 
				scan_path(x.path(), depth - 1);
			
		}
	}
}

FindDuplicate::FindDuplicate(std::vector<std::string> dirs, 
							 std::vector<std::string> skip_dirs,
							 std::vector<std::string> vec_file_mask,
							 size_t block_size, size_t scan_depth, 
							 uintmax_t min_file_size, 
							 std::string hasher) :
							 dirs(vec_str_to_path(dirs)), 
							 skip_dirs(vec_str_to_path(skip_dirs)),
							 block_size(block_size),
							 scan_depth(scan_depth), 
							 min_file_size(min_file_size) 
{
	set_hasher(hasher);
	for(const auto& i : vec_file_mask) 
		vec_regex_filemasks.emplace_back(boost::regex(i));
}

std::unordered_map<std::string, std::vector<std::string>> FindDuplicate::map_find_duplicates() 
{
	search();
	std::unordered_map<std::string, std::vector<std::string>> result;

	if(info_scan_files.size() < 2)
		return result;	
	
	for(auto first = std::begin(info_scan_files); first != end(info_scan_files); ++first)
	{
		if(!first->in_result) 
		{
			for(auto second = std::next(first); second != end(info_scan_files); ++second)
			{
				if(!second->in_result && first->equal_hash_file(*second)) 
				{
					result[first->get_path().string()].push_back(second->get_path().string());
					second->in_result = true;
				}
			}
		}
	}
	return result;
}

void FindDuplicate::set_dirs(std::vector<std::string> _dirs) 
{
	dirs = vec_str_to_path(_dirs);
}

void FindDuplicate::set_skip_dirs(std::vector<std::string> _dirs)
{
	skip_dirs = vec_str_to_path(_dirs);
}

void FindDuplicate::search() 
{
	for(const auto& i : dirs) 
		scan_path(i, scan_depth);	
}
