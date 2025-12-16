#include "buffer_manager.h"
#include "replacer.h"
#include "bp_error.h"
#include <assert.h>


int InitBufferManager(size_t pool_size, size_t k,  StorageManager *sm, BufferManager **bm){
	fprintf(stderr, "InitBufferManager, pool_size: %d, k: %d\n", pool_size, k);
    if (*bm || pool_size == 0 || sm == NULL) return -EBUFFERMANAGER;
    *bm = malloc(sizeof(BufferManager));
    memset(*bm, 0, sizeof(BufferManager));
    (*bm)->next_block_id_ = 0;
    (*bm)->pool_size_ = pool_size;
    (*bm)->pages_ = malloc(sizeof(Page) * pool_size);
    for (size_t i = 0; i < pool_size; i++){
        (*bm)->pages_[i].pin_count_ = 0;
        (*bm)->pages_[i].is_dirty_ = 0;
        (*bm)->pages_[i].block_id_ = INVALID_BLOCK_ID;
        (*bm)->pages_[i].data_ = malloc(sizeof(char) * PAGE_SIZE);
    }
    (*bm)->storage_manager_ = sm;
    (*bm)->replacer_ = NULL;
    int ret = InitReplacer(pool_size, k, &((*bm)->replacer_));
    if (ret) {
        for (size_t i = 0; i < pool_size; i++){
            free((*bm)->pages_[i].data_);
        }
        free((*bm)->pages_);
        free(bm);
        return -EBUFFERMANAGER;
    }
    FreeListNode *node = malloc(sizeof(FreeListNode));
    node->page_id_ = 0;
    node->next_ = NULL;
    (*bm)->free_list_  = node;
    for (size_t i = 1; i < pool_size; i++){
        FreeListNode *new_node = malloc(sizeof(FreeListNode));
        new_node->page_id_ = i;
        new_node->next_ = NULL;
        node->next_ = new_node;
        node = new_node;
    }
    return 0;
}

int StopBufferManager(BufferManager *bm){
	fprintf(stderr, "StopBufferManager\n");
    if (!bm) return -EBUFFERMANAGER;
    // Storage Manager will be handled separately.
    // Delete the page_table_ first    
    PageTableNode *current_node, *tmp;

    HASH_ITER(hh, bm->page_table_, current_node, tmp) {
        HASH_DEL(bm->page_table_, current_node);
        free(current_node); 
    }
    FreeListNode *node = bm->free_list_;
    FreeListNode *next_node;
    while(node) {
        next_node = node->next_;
        free(node);
        node = next_node;
    }
    for(size_t i = 0; i < bm->pool_size_; i++){
        free(bm->pages_[i].data_);
    }
    free(bm->pages_);
    StopReplacer(bm->replacer_);
    free(bm);
    return 0;
}

Page* FetchPage(block_id id, BufferManager *bm){
    fprintf(stderr, "FetchPage, id: %d\n", id);

    PageTableNode *node = NULL;
    HASH_FIND_INT(bm->page_table_, &id, node);

    if (node != NULL) {
        Page *page = &bm->pages_[node->pid_];
        page->pin_count_++;
        RecordAccess(node->pid_, bm->replacer_);
        SetEvictable(node->pid_, false, bm->replacer_);
        return page;
    }

    page_id pid;
    if (bm->free_list_ != NULL) {
        pid = bm->free_list_->page_id_;
        FreeListNode *tmp = bm->free_list_->next_;
        free(bm->free_list_);
        bm->free_list_ = tmp;
    } else if (Evict(&pid, bm->replacer_)) {
        block_id bid = bm->pages_[pid].block_id_;
        if (bm->pages_[pid].is_dirty_) {
            WritePage(bid, bm->pages_[pid].data_, bm->storage_manager_);
        }
        PageTableNode *node = NULL;
        HASH_FIND_INT(bm->page_table_, &bid, node);
		HASH_DEL(bm->page_table_, node);
		free(node);
    } else {
        return NULL;    }

    Page *page = &bm->pages_[pid];
    ReadPage(id, page->data_, bm->storage_manager_);
    page->block_id_ = id;
    page->pin_count_ = 1;
    page->is_dirty_ = false;

    RecordAccess(pid, bm->replacer_);
    SetEvictable(pid, false, bm->replacer_);

    node = malloc(sizeof(PageTableNode));
    *node = (PageTableNode){id, pid, 0};
    HASH_ADD_INT(bm->page_table_, bid_, node);

    return page;
}

Page* NewPage(block_id *id, BufferManager *bm) {
	fprintf(stderr, "NewPage\n");
	page_id pid = 0;
	if (bm->free_list_ != NULL) {
		pid = bm->free_list_->page_id_;
		FreeListNode *tmp = bm->free_list_->next_;
		free(bm->free_list_);
		bm->free_list_ = tmp;
	} else if(Evict(&pid, bm->replacer_)){
		block_id b = bm->pages_[pid].block_id_;
		PageTableNode *node = NULL;
		HASH_FIND_INT(bm->page_table_, &b, node);
		HASH_DEL(bm->page_table_, node);
		free(node);
	} else {
		return NULL;
	}

	block_id bid = AllocateBlock(bm);
	Page *newPage = &bm->pages_[pid];
	if (newPage->is_dirty_) {
		WritePage(newPage->block_id_, newPage->data_, bm->storage_manager_);
	}

	memset(newPage->data_, 0, PAGE_SIZE);
	newPage->block_id_ = bid;
	newPage->pin_count_ = 1;
	newPage->is_dirty_ = false;

	RecordAccess(pid, bm->replacer_);
	SetEvictable(pid, false, bm->replacer_);

	PageTableNode *node = NULL;
	HASH_FIND_INT(bm->page_table_, &bid, node);
	if (node == NULL) {
		node = malloc(sizeof(PageTableNode));
		*node = (PageTableNode){bid, pid, 0};
		HASH_ADD_INT(bm->page_table_, bid_, node);
	}
	node->pid_ = pid;

	*id = bid;
    return newPage;
}

bool UnpinPage(block_id id, bool is_dirty, BufferManager *bm){
	fprintf(stderr, "UnpinPage, id: %d, is_dirty: %d\n", id, is_dirty);
	PageTableNode *node = NULL;
	HASH_FIND_INT(bm->page_table_, &id, node);
	if (node == NULL) {
		return false;
	}
	Page *page = &bm->pages_[node->pid_];
	if (page->pin_count_ == 0) {
		return false;
	}
	page->pin_count_--;
	if (page->pin_count_ == 0) {
		SetEvictable(node->pid_, true, bm->replacer_);
	}

	if (is_dirty) {
		page->is_dirty_ = true;
	}
    return true;
}

bool FlushPage(block_id id, BufferManager *bm) {
	fprintf(stderr, "FlushPage, id: %d\n", id);
	PageTableNode *node = NULL;
	HASH_FIND_INT(bm->page_table_, &id, node);
	if (node == NULL) {
		return false;
	}
	Page *page = &bm->pages_[node->pid_];
	WritePage(id, page->data_, bm->storage_manager_);
	page->is_dirty_ = false;
    return true;
}

bool DeletePage(block_id id, BufferManager *bm) {
	fprintf(stderr, "DeletePage, id: %d\n", id);
	PageTableNode *node = NULL;
	HASH_FIND_INT(bm->page_table_, &id, node);
	if (node == NULL) {
		return true;
	}
	Page *page = &bm->pages_[node->pid_];
	if (page->pin_count_ > 0) {
		return false;
	}
	Remove(node->pid_, bm->replacer_);
	FreeListNode *freeNode = malloc(sizeof(FreeListNode));
	*freeNode = (FreeListNode){node->pid_, bm->free_list_};
	bm->free_list_ = freeNode;

	memset(page->data_, 0, PAGE_SIZE);
	page->block_id_ = INVALID_BLOCK_ID;
	page->pin_count_ = 0;
	page->is_dirty_ = false;

	HASH_DEL(bm->page_table_, node);
	free(node);

    return true;
}

block_id AllocateBlock(BufferManager *bm) {
	fprintf(stderr, "AllocateBlock\n");
    return bm->next_block_id_++;
}
