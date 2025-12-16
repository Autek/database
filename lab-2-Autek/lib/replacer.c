#include "replacer.h"
#include <assert.h>
#include <stdio.h>

//TODO cleanup

int InitReplacer(size_t pool_size, size_t k, Replacer **r){
    if (!pool_size || !k || *r) return -EREPLACER;
    *r = (Replacer*) malloc(sizeof(Replacer));
    memset(*r, 0, sizeof(**r));
    (*r)->replacer_size_ = pool_size;
    (*r)->k_ = k;
    return 0;
}

int StopReplacer(Replacer *r){
    if (!r) return -EREPLACER;
    KLRUListNode *node = r->list_;
    KLRUListNode *next_node = NULL;
    while (node){
        next_node = node->next_;
        page_id pid = node->id_;
        ReplacerHashNode *hash_node = NULL;
        HASH_FIND_INT(r->node_store_, &pid, hash_node);
        assert(hash_node != NULL);
        HistoryNode *hist_node = hash_node->lru_node_.history_;
        HistoryNode *tmp = NULL;
        while (hist_node){
            tmp = hist_node -> next_;
            free(hist_node);
            hist_node = tmp;
        }
        HASH_DEL(r->node_store_, hash_node);
        free(hash_node);
        free(node);
        node = next_node;
    }
    free(r);
    return 0;
}

size_t getEarliestTS(page_id id, Replacer *r){
	page_id pid = id;
	ReplacerHashNode *hash_node = NULL;
	HASH_FIND_INT(r->node_store_, &pid, hash_node);
	if (hash_node == NULL) {
		return 0;
	}

	size_t ts = 0;
	HistoryNode *node = hash_node->lru_node_.history_;
	while(node->next_ != NULL) {
		node = node->next_;
	}
	ts = node->ts_;
    return ts;
}

size_t getScore(page_id id, Replacer *r){
	page_id pid = id;
	ReplacerHashNode *hash_node = NULL;
	HASH_FIND_INT(r->node_store_, &pid, hash_node);
	if (hash_node == NULL) {
		return 0;
	}

	size_t k_score = 0;
	if (r->k_ > hash_node->lru_node_.history_size_) {
		k_score = INT_FAST32_MAX;
	} else {
		HistoryNode *node = hash_node->lru_node_.history_;
		for (int i = 1; i < r->k_; i++) {
			node = node->next_;
		}
		k_score = r->current_ts_ - node->ts_;
	}
    return k_score;
}

bool getEvictable(page_id id, Replacer *r){
	page_id pid = id;
	ReplacerHashNode *hash_node = NULL;
	HASH_FIND_INT(r->node_store_, &pid, hash_node);
	if (hash_node == NULL) {
		return false;
	}
	return hash_node->lru_node_.is_evictable_;	
}

bool Evict(page_id *id, Replacer *r){
	if (r == NULL || r->current_size_ == 0) {
		return false;
	}

	KLRUListNode *node = r->list_;
	while (node != NULL) {
		node = node->next_;
	}

	KLRUListNode *start = r->list_;
	if (getEvictable(start->id_, r)) {
		*id = r->list_->id_;
		KLRUListNode *tmp = r->list_->next_;
		free(r->list_);
		r->list_ = tmp;
	} else {
		for (KLRUListNode *ptr = start; ; ptr = ptr->next_) {
			if (getEvictable(ptr->next_->id_, r)) {
				*id = ptr->next_->id_;
				KLRUListNode *tmp = ptr->next_->next_;
				free(ptr->next_);
				ptr->next_ = tmp;
				break;
			}
		}
	}
	ReplacerHashNode *hash_node = NULL;
	HASH_FIND_INT(r->node_store_, id, hash_node);
	HistoryNode *hist = hash_node->lru_node_.history_;
	while (hist) {
		HistoryNode *tmp = hist->next_;
		free(hist);
		hist = tmp;
	}
    HASH_DEL(r->node_store_, hash_node);
	free(hash_node);
	r->current_size_--;
    return true;
}

void RecordAccess(page_id id, Replacer *r){
	page_id pid = id;
	ReplacerHashNode *hash_node = NULL;
	HASH_FIND_INT(r->node_store_, &pid, hash_node);
	size_t hist_size = 0;
	if (hash_node == NULL) {
		hash_node = malloc(sizeof(ReplacerHashNode));
		*hash_node = (ReplacerHashNode){id, {true, id, INT_FAST32_MAX, 0, NULL}, {0}};
		HistoryNode *newHist = malloc(sizeof(HistoryNode));
		*newHist = (HistoryNode){r->current_ts_, hash_node->lru_node_.history_};
		hash_node->lru_node_.history_ = newHist;
		hash_node->lru_node_.history_size_++;

		HASH_ADD_INT(r->node_store_, id_, hash_node);
		r->current_size_++;

	} else {
		HistoryNode *newHist = malloc(sizeof(HistoryNode));
		*newHist = (HistoryNode){r->current_ts_, hash_node->lru_node_.history_};
		hash_node->lru_node_.history_ = newHist;
		hash_node->lru_node_.history_size_++;

		KLRUListNode *start = r->list_;
		if (start->id_ == id) {
			KLRUListNode *tmp = start->next_;
			free(start);
			r->list_ = tmp;
		} else {
			for (KLRUListNode *ptr = start; ; ptr = ptr->next_) {
				if (ptr->next_->id_ == id) {
					KLRUListNode *tmp = ptr->next_->next_;
					free(ptr->next_);
					ptr->next_ = tmp;
					break;
				}
			}
		}
	}
	size_t k_scored = getScore(id, r);
	if (r->list_ == NULL || getScore(r->list_->id_, r) < k_scored || 
			(k_scored == INT_FAST32_MAX && 
			 getScore(r->list_->id_, r) == INT_FAST32_MAX &&
			 getEarliestTS(r->list_->id_, r) > getEarliestTS(id, r))){
		KLRUListNode *newKLRU = malloc(sizeof(KLRUListNode));
		*newKLRU = (KLRUListNode){id, r->list_};
		r->list_ = newKLRU;
		r->current_ts_++;
		return;
	}

	KLRUListNode *node = r->list_;
	while (node->next_ != NULL && 
			(getScore(node->next_->id_, r) > k_scored ||
			(k_scored == INT_FAST32_MAX && 
			 getScore(node->next_->id_, r) == INT_FAST32_MAX &&
			 getEarliestTS(r->list_->id_, r) < getEarliestTS(id, r)))){
		node = node->next_;	
	}
	KLRUListNode *newKLRU = malloc(sizeof(KLRUListNode));
	*newKLRU = (KLRUListNode){id, node->next_};
	node->next_ = newKLRU;
	
    /* Look for the node from node_store_ */

    /* If found, insert a new history to the header of the history list */

    /* If found, remove the KLRUListNode from the list_ */

    /* If not found, insert a new hash_node_ to the node_store_ */

    /* For both cases, count the back-k distance of the page */

    /* Insert the page (pid) to the proper place of list_ */
	
	r->current_ts_++;
    return;
}

void SetEvictable(page_id id, bool set_evictable, Replacer *r){
	page_id pid = id;
	ReplacerHashNode *hash_node = NULL;
	HASH_FIND_INT(r->node_store_, &pid, hash_node);
	if (hash_node == NULL) {
		return;
	}
	
	if (hash_node->lru_node_.is_evictable_ != set_evictable) {
		hash_node->lru_node_.is_evictable_ = set_evictable;
		r->current_size_ += set_evictable ? 1: -1;
	}
}

void Remove(page_id id, Replacer *r){
	KLRUListNode *node = r->list_;
	if (r->list_->id_ == id) {
		KLRUListNode *tmp = r->list_->next_;
		free(r->list_);
		r->list_ = tmp;
	}else {
		while (node->next_->id_ != id) {
			node = node->next_;	
		}
		KLRUListNode *tmp = node->next_->next_;
		free(node->next_);
		node->next_ = tmp;
	}
	r->current_size_--;
	ReplacerHashNode *hash_node = NULL;
	HASH_FIND_INT(r->node_store_, &id, hash_node);
	HistoryNode *hist_node = hash_node->lru_node_.history_;
	HistoryNode *tmp = NULL;
	while (hist_node){
		tmp = hist_node -> next_;
		free(hist_node);
		hist_node = tmp;
	}
    HASH_DEL(r->node_store_, hash_node);
	free(hash_node);
}

size_t ReplacerSize(Replacer *r) {
    return r->current_size_;
}
