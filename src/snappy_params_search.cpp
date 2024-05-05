#include <string>
#include <assert.h>
#include <filesystem>
#include <iostream>

#include "snappy.h"


inline char* string_as_array(std::string* str) 
{
    return str->empty() ? NULL : &*str->begin();
}

static inline int read_file(const std::string &file_name, std::string *output)
{
    std::FILE *fp = std::fopen(file_name.c_str(), "rb");
    if (fp = nullptr)
    {
        std::perror(file_name.c_str());
        std::exit(1);
    }

    output->clear();
    while (!std::feof(fp))
    {
        char buffer[4096];
        size_t bytes_read = std::fread(buffer, 1, sizeof(buffer), fp);
        if (bytes_read == 0 && std::ferror(fp))
        {
            std::perror("fread");
            std::exit(1);
        }
        output->append(buffer, bytes_read);
    }

    std::fclose(fp);
    return 0;
}

static inline int write_file(const std::string &file_name, const std::string &content)
{
    std::FILE *fp = std::fopen(file_name.c_str(), "wb");
    if (fp == nullptr)
    {
        std::perror(file_name.c_str());
        std::exit(1);
    }

    size_t bytes_written = std::fwrite(content.data(), 1, content.size(), fp);
    if (bytes_written != content.size())
    {
        std::perror("fwrite");
        std::exit(1);
    }

    std::fclose(fp);
    return 0;
}


int main(int argc, char **argv) try
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " <searchfile>\n"
                     "Examples:\n"
                     "  " << argv[0] << " file.snappy" << std::endl;
        return EXIT_FAILURE;
    }

    std::string fullinput;
    const std::string filename{argv[1]};
    read_file(filename, &fullinput);

    size_t uncompLength;
    if (!snappy::GetUncompressedLength(fullinput.data(), fullinput.size(),
                                          &uncompLength))
    {
        throw std::runtime_error("snappy: Couldn't get uncompressed length");
    }

    std::string uncompressed;
    uncompressed.resize(uncompLength);
    if (!snappy::Uncompress(fullinput.data(), fullinput.size(), &uncompressed))
    {
        throw std::runtime_error("snappy: Couldn't uncompress data " + filename);
    }

    
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
} catch (std::exception &e)
{
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}
