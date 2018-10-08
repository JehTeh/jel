#include <windows.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>

const char cflags_line_identifer[] = "CFLAGS";
const char cppflags_line_identifer[] = "CPPFLAGS";

int main(int argc, char** argv)
{
    std::cout << std::endl << "======== compile_commands.json generator ========" << std::endl << std::endl;
    std::vector<std::string> iargs; 
    std::vector<std::string> dirs;
    for(size_t i = 1; i < argc; i++)
    {
        iargs.push_back(argv[i]);
    }
    if(iargs.size() == 0)
    {
        std::cout << "No '.flags' file specified! Generator aborting..." << std::endl;
        return 1;
    }
    std::ifstream flagsFile(iargs[0]);
    //Erase first argument. Whats left should be subdirectories
    iargs.erase(iargs.begin());
    if(!flagsFile.is_open())
    {
        std::cout << "Failed to open flags file '" << iargs[0] << "'. Generator aborting..." << std::endl;
        return 1;
    }
    //Get current directory 
    constexpr size_t temp_buf_len = 1024;
    char temp_buf[temp_buf_len];
    if(GetModuleFileName(NULL, temp_buf, temp_buf_len) == 0)
    {
        std::cout << "Failed to get current directory! Generator aborting..." << std::endl;
        return 1;
    }
    std::string rootdir(temp_buf);
    rootdir.erase(rootdir.find_last_of("\\") + 1);
    std::string cflags;
    std::string cppflags;
    while(!flagsFile.eof())
    {
        std::string line;
        std::getline(flagsFile, line);
        if(line.find(cflags_line_identifer) != std::string::npos)
        {
            line.erase(line.find(cflags_line_identifer), sizeof(cflags_line_identifer));
            cflags = line;
        }
        else if(line.find(cppflags_line_identifer) != std::string::npos)
        {
            line.erase(line.find(cppflags_line_identifer), sizeof(cppflags_line_identifer));
            cppflags = line;
        }
        else
        {
            if(line.length() != 0)
            {
                dirs.push_back(line);
            }
        }
    }
    std::cout << ".c/.h flags: '" << cflags << "'" << std::endl;
    std::cout << ".cpp/.hpp flags: '" << cppflags << "'" << std::endl;
    std::vector<std::string> cfileNames;
    std::vector<std::string> cppfileNames;
    auto addFilesWithSuffixToList = [](const std::string& dir, const char* suffix, std::vector<std::string>& list)
    {
        HANDLE hFile;
        WIN32_FIND_DATAA fData;
        std::string fStr = dir + suffix;
        hFile = FindFirstFileA(fStr.c_str(), &fData);
        if(hFile != INVALID_HANDLE_VALUE)
        {
            size_t count = 0;
            do
            {
                count++;
                list.push_back(fData.cFileName);
            }
            while(FindNextFileA(hFile, &fData));
            std::cout << "Added " << count << " entries for " << suffix << " files found in " << dir << std::endl;
        }
        else
        {
            std::cout << "No " << suffix << " files found in " << dir << std::endl;
        }
        return;
    };
    std::ofstream jsonOut("./compile_commands.json");
    std::vector<std::string> jsonOut_vec;
    if(!jsonOut.is_open())
    {
        std::cout << "Failed to create/overwrite compile_commands.json" << std::endl;
        return 1;
    }
    jsonOut_vec.push_back("[\n");
    for(auto&& subdir : dirs)
    {
        auto printJsonBlocks = [&](std::vector<std::string>& outLines, const std::vector<std::string>& fileNames, const std::string& flags)
        {
            for(auto& fn : fileNames)
            {
                std::string lbuf = "";
                std::string dirPlusFname = subdir + "/" + fn;
                lbuf += "  {\n";
                lbuf += "    \"directory\": \"" + rootdir + "\",\n";
                lbuf += "    \"command\": \"" + cflags + " "+ dirPlusFname + "\",\n";
                lbuf += "    \"file\": \"" + dirPlusFname + "\"\n";
                lbuf += "  },\n";
                while(lbuf.find("\\") != std::string::npos)
                {
                    lbuf.replace(lbuf.find("\\"), 1, "/");
                }
                outLines.push_back(lbuf);
            }
            return;
        };
        std::cout << "Generating flags for files in '" << subdir << "'..." << std::endl;
        addFilesWithSuffixToList(rootdir + subdir, "\\*.c", cfileNames);
        addFilesWithSuffixToList(rootdir + subdir, "\\*.h", cfileNames);
        addFilesWithSuffixToList(rootdir + subdir, "\\*.cpp", cppfileNames);
        addFilesWithSuffixToList(rootdir + subdir, "\\*.hpp", cppfileNames);
        printJsonBlocks(jsonOut_vec, cfileNames, cflags);
        printJsonBlocks(jsonOut_vec, cppfileNames, cppflags);
    }
    jsonOut_vec.push_back("]");
    //Remove trailing ',' character so JSON is valid
    std::string& secondLastLine = jsonOut_vec[jsonOut_vec.size() - 2];
    secondLastLine.replace(secondLastLine.rfind("},"), 2, "}");
    for(auto&& line : jsonOut_vec)
    {
        jsonOut << line;
    }
    std::cout << "compile_commands.json generated." << std::endl;
    return 0;
}
