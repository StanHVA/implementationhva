#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <iostream>    // using IO functions
#include <string>      // using string
#include <string.h>
#include <experimental/filesystem>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <cinttypes>
#include <openssl/md5.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm> //find_if
#include <curl/curl.h>
#include <json-c/json.h>

namespace fs = std::experimental::filesystem;

using namespace std;

string fsToString(const std::experimental::filesystem::v1::directory_entry e) {
	std::ostringstream oss;
	oss << e;
	return (oss.str());
}

class Scanner {
private:
	string directory;       // Data member (Variable)
	int fileCount;			// Aantal files in de dir
	std::vector<std::string> files;					// bestanden in de dir. Gaan later door de hash generator
	std::vector<std::string> recursiveDirectories;	// Program scant recusively, dus er moet een vector komen met
													// directories in de scanner directory
    std::vector<char> charFiles;
    std::vector<char> charDirectories;

public:
	string getDirectory() {
		return directory;
	}

	int getFileCount() {
		return fileCount;
	}

	std::vector<std::string> getFiles() {
		return files;
	}

	void printFiles() {
		std::string nothingness;

	}

	void printDirectories() {
		for (std::vector<string>::const_iterator i = recursiveDirectories.begin(); i != recursiveDirectories.end(); ++i)
			std::cout << *i << endl;
	}

	int countFiles() {
		int cnt = std::count_if(
			fs::recursive_directory_iterator(directory),
			fs::recursive_directory_iterator(),
			static_cast<bool(*)(const fs::path&)>(fs::is_regular_file)
		);
		return cnt;
	}


	void indexFiles() {
		for (auto & p : fs::recursive_directory_iterator(directory)) {		//create read only access 'p' to use in the loop.
			if (is_regular_file(p)) {	// only shows regular files and no directories.
				string converted = fsToString(p);
                converted.erase(remove(converted.begin(), converted.end(), '"'), converted.end());

				files.push_back(converted);
			}
			if (is_directory(p)) {
				recursiveDirectories.push_back(fsToString(p));
			}
		}
	}

	// Constructor
	Scanner(string dir = "/root/testdir") {
		directory = dir;			// Creates a scanner with a directory.
		fileCount = countFiles();	// Counts files in dirs
		indexFiles();				// Recursively scans dirs and files. Dirs are saved but not implemented to the API.
	}
};

string get_md5_sum(unsigned char* md) {
    int i;
    string result;
    result.reserve(32);
    char buf[32];
    /*
    for(i=0; i <MD5_DIGEST_LENGTH; i++) {

        printf("%02x",md[i]);
        //snprintf(result, 2, "%02x",md[i]);
        //sprintf(buf, "%02x",md[i]);
        //result.append(result);


        //return md[i];
    }
    */

    for(size_t i = 0; i != 16; ++i){
        result += "0123456789abcdef" [md[i] / 16];
        result += "0123456789abcdef" [md[i] % 16];
    }


    return result;
}

// Get the size of the file by its file descriptor
unsigned long get_size_by_fd(int fd) {
    struct stat statbuf;
    if(fstat(fd, &statbuf) < 0) exit(-1);
    return statbuf.st_size;
}

string get_md5_data(string path) {

    unsigned char result[MD5_DIGEST_LENGTH];

    char charPath[path.size() + 1];
    strcpy(charPath, path.c_str());
    char* file_buffer;
    int file_descript = open(charPath, O_RDONLY);

    if(file_descript < 0) exit(-1);

    unsigned long file_size = get_size_by_fd(file_descript);

    file_buffer = (char*)mmap(0, file_size, PROT_READ, MAP_SHARED, file_descript, 0);
    MD5((unsigned char*) file_buffer, file_size, result);
    munmap(file_buffer, file_size);

    string md5Sum = get_md5_sum(result);
    cout << charPath << "     " << md5Sum << endl;
    return md5Sum;
    //printf("  %s\n", charPath);
}

void perform_curl(string path, string md5hash){
    json_object *json;
    json = json_object_new_object();
    json_object_object_add(json, "id", json_object_new_int(1));
    json_object_object_add(json, "path", json_object_new_string(path.c_str()));
    json_object_object_add(json, "hash", json_object_new_string(md5hash.c_str()));

    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl == NULL) {
        return;
    }
    struct curl_slist *headers = NULL;
    char *url = "http://localhost/api/hashapi/md5";
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: application/json");

    if (headers == NULL) {
        return; //preven null pointer exc
    }

    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost/api/hashapi/md5");

    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_object_to_json_string(json));
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.64.0");

    res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);
    curl_global_cleanup();
}

int main() {

    unsigned char result[MD5_DIGEST_LENGTH];

	Scanner scanner1("/root/wpscan/bin");
	scanner1.printFiles();
    vector<string> outfiles = scanner1.getFiles();
    vector<string> md5files;
    for(int i=0; i < outfiles.size(); i++){
        //get_md5_data(outfiles[i]);
        md5files.push_back(get_md5_data(outfiles[i]));
    }

    for(int i=0; i < outfiles.size(); i++){
        perform_curl(outfiles[i], md5files[i]);
    }

    return 0;
}