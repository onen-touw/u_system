#pragma once

#include "config.h"
#include "esp_spiffs.h"
#include "str.h"
#include "error.h"
#include "trace.h"
#include <unistd.h>

namespace ufo
{
        
    class file_t
    {
    private:
        FILE* _f = nullptr;
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
        dir_t(string_t path, string_t lbl) {
            esp_vfs_spiffs_conf_t conf = {}; 
            conf.base_path = path.c_str();
            conf.partition_label = lbl.c_str();
            conf.max_files = config::fs_max_open;
            conf.format_if_mount_failed = false;
            esp_err_t ret = esp_vfs_spiffs_register(&conf);

            if (ret != ESP_OK)
            {
                Error_t::GetInstance().Push(CriticalError_t(GenerateInfo_Code(error::codes_t::fs_dir, "cant op dir")));
                return;
            }

            ret = esp_spiffs_check(nullptr);
            if (ret != ESP_OK)
            {
                Error_t::GetInstance().Push(CriticalError_t(GenerateInfo_Code(error::codes_t::fs_dir, "cant op dir")));
                return;
            }
        }
        ~dir_t() {}

        // no copy, no move 
        copy
        file("name")
        path // always return spiffs/
        count   // of files
        size    // sum 
        remove
        file_t touch(string_t name) {
            fopen()
        }
        
        dir-memory-info

        plist(portable)

        file-iterator

    private:
        bool check_file_name(string_t& name) const {
            for (size_t i = 0; i < name.size(); i++)
            {
                // if (name[i]  'a' )
                {
                    /* code */
                }
                
            }
            
            //
            return true;
        }
    };

} // namespace ufo


