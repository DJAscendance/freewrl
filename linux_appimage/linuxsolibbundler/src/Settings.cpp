/*
The MIT License (MIT)

Copyright (c) 2014 Marianne Gagnon

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
 */

#include "Settings.h"
#include <vector>
#include <iostream>
#include "Utils.h"

namespace Settings
{

bool overwrite_files = false;
bool overwrite_dir = false;
bool create_dir = false;

bool canOverwriteFiles(){ return overwrite_files; }
bool canOverwriteDir(){ return overwrite_dir; }
bool canCreateDir(){ return create_dir; }

void canOverwriteFiles(bool permission){ overwrite_files = permission; }
void canOverwriteDir(bool permission){ overwrite_dir = permission; }
void canCreateDir(bool permission){ create_dir = permission; }


bool bundleLibs_bool = false;
bool bundleLibs(){ return bundleLibs_bool; }
void bundleLibs(bool on){ bundleLibs_bool = on; }


std::string dest_folder_str = "./lib/";
std::string destFolder(){ return dest_folder_str; }
void destFolder(std::string path)
{
    dest_folder_str = path;
    // fix path if needed so it ends with '/'
    if( dest_folder_str[ dest_folder_str.size()-1 ] != '/' ) dest_folder_str += "/";
}

std::vector<std::string> files;
void addFileToFix(std::string path){ files.push_back(path); }
int fileToFixAmount(){ return files.size(); }
std::string fileToFix(const int n){ return files[n]; }

std::string inside_path_str = "@executable_path/../lib/";
std::string inside_lib_path(){ return inside_path_str; }
void inside_lib_path(std::string p)
{
    inside_path_str = p;
    // fix path if needed so it ends with '/'
    if( inside_path_str[ inside_path_str.size()-1 ] != '/' ) inside_path_str += "/";
}

std::vector<std::string> prefixes_to_ignore;
void ignore_prefix(std::string prefix)
{
    if( prefix[ prefix.size()-1 ] != '/' ) prefix += "/";
    prefixes_to_ignore.push_back(prefix);
}

bool isPrefixIgnored(std::string prefix)
{
    const int prefix_amount = prefixes_to_ignore.size();
    for(int n=0; n<prefix_amount; n++)
    {
        if(prefix.compare(0,prefixes_to_ignore[n].length(),prefixes_to_ignore[n]) == 0) return true;
    }

    return false;
}

bool isPrefixBundled(std::string prefix)
{
    std::cout << "\nprefix" << prefix;
    if(prefix.find(".framework") != std::string::npos) return false;
    if(prefix.find("@executable_path") != std::string::npos) return false;
    if(prefix.compare(0,5,"ib64/") == 0) return false;
    //if(prefix.compare(0,5,"/lib/") == 0) return false; //theres a libpng12.so.1 I need 
    //if(prefix.compare(0,9,"/usr/lib/") == 0) return false; //on linux ths is where our 3rd party libs are.
    // should leave /usr/local/lib 
    if(isPrefixIgnored(prefix)) return false;
    std::cout << " - bundled";
    return true;
}

bool doRpaths_bool = false;
bool doRpaths() { return doRpaths_bool;}
void doRpaths(bool permission) {doRpaths_bool = permission;}


std::vector<std::string> exclusions;
bool exclusions_loaded = false;
void exclusions_file(std::string efilename){
    if (!fileExists(efilename))
    {
        std::cerr << "\n/!\\ WARNING : can't read exclusions file '" << efilename << "'\n";
        return;
    }
	std::string cmd = "cat " + efilename + " | grep -v -e '^#' | grep -v -e '^$'";
    std::string output = system_get_output(cmd);

    std::vector<std::string> lc_lines;
    tokenize(output, "\n", &lc_lines);

    size_t pos = 0;
    bool read_rpath = false;
    while (pos < lc_lines.size())
    {
        std::string line = lc_lines[pos];
        pos++;
		exclusions.push_back(line);
	}
    std::cout << "Exclusions:" << exclusions.size() << std::endl;
	for(unsigned int i=0;i<exclusions.size();i++){
		std::cout << exclusions[i] << std::endl;
	}

	exclusions_loaded = true;
}
std::string soname(std::string filename){
    std::string rootname = filename.substr(0,filename.find(".so."));
    std::string version = filename.substr(filename.find(".so.")+4);
    std::string interface_version = version;
    size_t dot = version.find(".");
    if( dot != std::string::npos)
         interface_version = version.substr(0,dot);
    std::string so_name = rootname + ".so." + interface_version;
    //std::cout << " " << filename << " > " << so_name << std::endl;
    return so_name;
}
bool isFilenameBundled(std::string filename)
{
	if(!exclusions_loaded) return true;
    std::cout << "\nfilename " << filename;
    /*
	if(std::find(exclusions.begin(), exclusions.end(), filename) == exclusions.end()){
	    std::cout << " - bundled";
		return true;
	}
    return false;
    */
    for(unsigned int i=0;i<exclusions.size();i++){

        if(exclusions[i].compare(soname(filename)) == 0){
            std::cout << " - excluded";
            return false;
        }
    }
    std::cout << " - bundled";
    return true;
}

}