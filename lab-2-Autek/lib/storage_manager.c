#include "storage_manager.h"
#include "bp_error.h"

#include "stdlib.h"
#include <string.h>
#include <assert.h>

int InitStorageManager(const char* filename, StorageManager **storage_manager){
    // Don't initialize it twice
    if (*storage_manager) return -ESTORAGEMANAGER;
    *storage_manager = malloc(sizeof(StorageManager));
    FILE* fptr = NULL;
    fptr = fopen(filename, "r+");

    if (fptr == NULL){
        fptr = fopen(filename, "w+");
        if (fptr == NULL) return -ESTORAGEMANAGER;
    }

    (*storage_manager)->file_ptr_ = fptr;
    (*storage_manager)->filename_ = malloc(strlen(filename) + 1);
    strcpy((*storage_manager)->filename_, filename); 
    return 0;
}

int StopStorageManager(StorageManager *storage_manager){
    if (storage_manager){
        int ret = fclose(storage_manager->file_ptr_);
        free(storage_manager->filename_);
        free(storage_manager);
        return ret;
    }
    return -ESTORAGEMANAGER;
}

int WritePage(block_id id, const char* page_data, StorageManager *storage_manager){
	if (storage_manager == NULL) {
		return -ESTORAGEMANAGER;
	} 
	if (id < 0 || page_data == NULL) {
		return -ESTORAGEWRITE;
	}
	if (fseek(storage_manager->file_ptr_, id * PAGE_SIZE, SEEK_SET)) {
		return -ESTORAGEWRITE;
	}
    if (PAGE_SIZE != fwrite(page_data, 1, PAGE_SIZE, storage_manager->file_ptr_)){
		return -ESTORAGEWRITE;
	}
	return PAGE_SIZE;
}

int ReadPage(block_id id, char* page_data, StorageManager *storage_manager){
	if (storage_manager == NULL) {
		return -ESTORAGEMANAGER;
	} 
	if (id < 0 || page_data == NULL) {
		return -ESTORAGEREAD;
	}

	fseek(storage_manager->file_ptr_, 0, SEEK_END);
	long file_size = ftell(storage_manager->file_ptr_);
	if (id * PAGE_SIZE >= file_size) {
		return -ESTORAGEREAD;
	}

	if (fseek(storage_manager->file_ptr_, id * PAGE_SIZE, SEEK_SET)) {
		return -ESTORAGEREAD;
	}

	size_t read = fread(page_data, 1, PAGE_SIZE, storage_manager->file_ptr_);
	if (read == 0) {
		return -ESTORAGEREAD;
	}

    if (PAGE_SIZE != read){
		memset(page_data + read, 0, PAGE_SIZE - read);
	}
	return read;
}
