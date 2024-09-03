#include <string>
#include <assert.h>
#include <filesystem>
#include <iostream>

#include "snappy.h"


const int n_level = 2;

inline char* string_as_array(std::string* str) 
{
    return str->empty() ? NULL : &*str->begin();
}

static inline int read_file(const std::string &file_name, std::string *output)
{
    std::FILE *fp = std::fopen(file_name.c_str(), "rb");
    if (fp == nullptr)
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

/**
 * @brief 
 * 
 * @param compressed 
 * @param uncompressed 
 * @return int 1 ... n_level : search succeed return level
 *             0 : cannot find a level
 *            -1 : error
 */
static inline int search_level(const std::string &compressed, const std::string &uncompressed)
{
    for (int i_level = 1; i_level < n_level + 1; i_level++)
    {
        snappy::CompressionOptions level{i_level};
        std::string recompressed;
        recompressed.clear();
        recompressed.resize(snappy::MaxCompressedLength(uncompressed.size()));
        size_t destlen;
        snappy::RawCompress(uncompressed.data(), uncompressed.size(),
                            string_as_array(&recompressed),
                            &destlen, level);
        if (destlen > snappy::MaxCompressedLength(uncompressed.size()))
        {
            throw std::runtime_error("");
        }
        recompressed.resize(destlen);
        if (compressed == recompressed) return i_level;
    }

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

    int level = search_level(fullinput, uncompressed);
    assert(level >= 0 && level <= n_level);
    if (level > 0)
    {
        write_file(std::string(filename).append(".uncomp"), uncompressed);
        std::cout << "Compression Level is " << level << "." << std::endl;
    } else {
        std::cout << "Can not find the compression level " << level << std::endl;
    }
    
    return 0;
} catch (std::exception &e)
{
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}
