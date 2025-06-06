#pragma once

#include "config.h"
#include "esp_spiffs.h"

namespace ufo
{
        
    class fs_t
    {
    private:

    public:
        fs_t(/* args */) {}
        ~fs_t() {}

        // no copy?
        read 
        write
        open
        close
        rename
        seek
        eof
        size
        path
        name
        exist
        type
        perm    // permition (r/w)
    };

    class dir_t
    {
    private:
    
    public:
        dir_t(/* args */) {}
        ~dir_t() {}

        // no copy, no move 
        copy
        file("name")
        path // always return spiffs/
        count   // of files
        size    // sum 
        remove
        touch
        
        dir-memory-info

        plist(portable)

        file-iterator
    };

} // namespace ufo


