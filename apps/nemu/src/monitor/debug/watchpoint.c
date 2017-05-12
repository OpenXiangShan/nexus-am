#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_list[NR_WP];
static WP *head, *free_;

void init_wp_list() {
	int i;
	for(i = 0; i < NR_WP; i ++) {
		wp_list[i].NO = i;
		wp_list[i].next = &wp_list[i + 1];
	}
	wp_list[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_list;
}

/* TODO: Implement the functionality of watchpoint */

WP *new_wp(){
	if(free_ != NULL)
	{
		WP *node = head;
		head = free_;
		free_ = free_->next;
		head->next = node;
		return head;
	}
	else
	{
		assert(0);
		return head;
	}
}

void free_wp(WP *wp){
	if(head == NULL)
	{
		assert(0);
		return;
	}
	WP *node = head;
	if(head->next == NULL || strcmp(head->expr,wp->expr)==0)
	{
		head = NULL;
		wp->next = free_;
		free_ = wp;
		return;
	}
	while(strcmp(node->next->expr,wp->expr)!= 0)
		node = node->next;
	WP *current = node->next;
	node->next = current->next;
	current->next = free_;
	free_ = current;
	return;
}

