#pragma once

#include "Defines.hpp"
#include "string.hpp"
#include <stdio.h>

namespace wfe {
    typedef enum : uint8_t {
        NONE       = 0,
        BINARY     = 1,
        AT_THE_END = 2,
        APPEND     = 4,
        TRUNCATE   = 8
    } StreamType;
    typedef enum : uint8_t {
        BEGINNING = SEEK_SET,
        POSITION = SEEK_CUR,
        END = SEEK_END
    } SeekRelative;

    class FileInput {
    public:
        FileInput();
        FileInput(const FileInput& other) = delete;
        FileInput(FileInput&& other) noexcept = delete;
        FileInput(const string& fileLocation, StreamType type = StreamType::NONE);

        FileInput& operator=(const FileInput& other) = delete;
        FileInput& operator=(FileInput&& other) noexcept = delete;

        bool8_t operator!();
        operator bool8_t();

        FileInput& Open(const string& fileLocation, StreamType type = StreamType::NONE);
        FileInput& Close();

        FileInput& Get();
        FileInput& ReadBuffer(char_t& c);
        FileInput& ReadBuffer(char_t* buffer, size_t count);

        FileInput& ReadLine(string& str, size_t size, char_t sep = '\n');

        FileInput& Read(sint8_t  & num, const char_t* sep = " \n");
        FileInput& Read(sint16_t & num, const char_t* sep = " \n");
        FileInput& Read(sint32_t & num, const char_t* sep = " \n");
        FileInput& Read(sint64_t & num, const char_t* sep = " \n");
        FileInput& Read(uint8_t  & num, const char_t* sep = " \n");
        FileInput& Read(uint16_t & num, const char_t* sep = " \n");
        FileInput& Read(uint32_t & num, const char_t* sep = " \n");
        FileInput& Read(uint64_t & num, const char_t* sep = " \n");
        FileInput& Read(float32_t& num, const char_t* sep = " \n");
        FileInput& Read(float64_t& num, const char_t* sep = " \n");
        FileInput& Read(string   & str, const char_t* sep = " \n");

        FileInput& Seek(ptrdiff_t pos, SeekRelative relative = SeekRelative::BEGINNING);

        bool8_t IsOpen() const;
        bool8_t IsAtTheEnd() const;
        bool8_t IsBad() const;
        size_t Tell() const;
        size_t GetCount() const;
        char_t Peek() const;

        ~FileInput();
    private:
        FILE* file;
    };

    class FileOutput {
    public:
        FileOutput();
        FileOutput(const FileOutput&) = delete;
        FileOutput(FileOutput&&) noexcept = delete;
        FileOutput(const string& fileLocation, StreamType type = StreamType::NONE);

        bool8_t operator!();
        operator bool8_t();

        FileOutput& Open(const string& fileLocation, StreamType type = StreamType::NONE);
        FileOutput& Close();

        FileOutput& WriteBuffer(char_t c);
        FileOutput& WriteBuffer(char_t* buf, size_t size);
        
        template<class T>
        FileOutput& Write(const T& value) {
            return Write(ToString(value));
        }
        FileOutput& Write(char_t c);
        FileOutput& Write(const string& str);

        bool8_t IsOpen() const;
        bool8_t IsAtTheEnd() const;
        bool8_t IsBad() const;
        size_t Tell() const;
        size_t GetCount() const;
        char_t Peek() const;

        ~FileOutput();
    private:
        FILE* file;
    };
}