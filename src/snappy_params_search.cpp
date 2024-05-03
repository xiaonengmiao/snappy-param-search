#include <string>
#include <assert.h>

#include "snappy.h"

inline char* string_as_array(std::string* str) 
{
    return str->empty() ? NULL : &*str->begin();
}


int main(int argc, char **argv)
{
    for (int arg = 1; arg < argc; ++arg)
    {
        std::string fullinput;
        std::FILE *fp = std::fopen(argv[arg], "rb");
        if (fp == nullptr) 
        {
            std::perror(argv[arg]);
            std::exit(1);
        }

        fullinput.clear();
        while (!std::feof(fp)) 
        {
            char buffer[4096];
            size_t bytes_read = std::fread(buffer, 1, sizeof(buffer), fp);
            if (bytes_read == 0 && std::ferror(fp)) 
            {
                std::perror("fread");
                std::exit(1);
            }
            fullinput.append(buffer, bytes_read);
        }
        std::fclose(fp);

        std::string compressed;
        compressed.resize(snappy::MaxCompressedLength(fullinput.size()));
        size_t destlen;
        snappy::RawCompress(fullinput.data(), fullinput.size(),
                            string_as_array(&compressed),
                            &destlen);
        assert(destlen <= snappy::MaxCompressedLength(fullinput.size()));
        compressed.resize(destlen);

        std::string file_name = std::string(argv[arg]).append(".comp");
        std::FILE *fpo = std::fopen(file_name.c_str(), "wb");
        if (fpo == nullptr) {
            std::perror(file_name.c_str());
            std::exit(1);
        }

        size_t bytes_written = std::fwrite(compressed.data(), 1, compressed.size(), fpo);
        if (bytes_written != compressed.size()) {
            std::perror("fwrite");
            std::exit(1);
        }

        std::fclose(fpo);
    }
    
    return 0;
}
