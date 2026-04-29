#include <storage.h>

bool flash_storage_init(){
    esp_err_t err = nvs_flash_init(); //Initialize the default NVS partition
    if(err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND){ //if existing NVS paritions have something wrong
        ESP_ERROR_CHECK_WITHOUT_ABORT(nvs_flash_erase()); //erase the partition
        err = nvs_flash_init(); //reinit again
    }
    ESP_ERROR_CHECK_WITHOUT_ABORT(err);
    return (err == ESP_OK) ? true : false;
}

bool flash_write(const char *data_name, const void *write_buffer, size_t length){
    debug_printf("flash_write called\r\n");
    nvs_handle_t nvs_handle;
    esp_err_t err;

    //Open NVS handle
    err = nvs_open(STORAGE_NAME, NVS_READWRITE, &nvs_handle);
    if(err != ESP_OK){
        debug_printf("Error (%s) opening NVS handle!\r\n", esp_err_to_name(err));
        ESP_ERROR_CHECK_WITHOUT_ABORT(err);
        return false;
    }

    //Write data
    debug_printf("Saving test data blob...\r\n");
    err = nvs_set_blob(nvs_handle, data_name, write_buffer, length);
    if(err != ESP_OK){
        debug_printf("Failed to write test data blob!\r\n");
        ESP_ERROR_CHECK_WITHOUT_ABORT(err);
        nvs_close(nvs_handle);
        return false;
    }

    //Commit the change (which is so weird? why don't set blob just write it straight to flash and I have to commit like fucking git?)
    err = nvs_commit(nvs_handle);
    if(err != ESP_OK){
        debug_printf("Failed to commit data\r\n");
        ESP_ERROR_CHECK_WITHOUT_ABORT(err);
        nvs_close(nvs_handle);
        return false;
    }

    nvs_close(nvs_handle);
    return true;
}
bool flash_read(const char *data_name, void *read_buffer, size_t length){
    debug_printf("flash_read called\r\n");
    nvs_handle_t nvs_handle;
    esp_err_t err;

    err = nvs_open(STORAGE_NAME, NVS_READONLY, &nvs_handle);
    if(err != ESP_OK){
        debug_printf("Error (%s) opening NVS handle!\r\n", esp_err_to_name(err));
        ESP_ERROR_CHECK_WITHOUT_ABORT(err);
        return false;
    }

    // 1. Read test data blob
    debug_printf("Reading data blob:\r\n");
    err = nvs_get_blob(nvs_handle, data_name, read_buffer, &length);
    if(err != ESP_OK){
        if(err == ESP_ERR_NVS_NOT_FOUND){
            debug_printf("Data not found!\r\n");
        }
        else{
            ESP_ERROR_CHECK_WITHOUT_ABORT(err);
        }
        nvs_close(nvs_handle);
        return false;
    }

    nvs_close(nvs_handle);
    return true;
}