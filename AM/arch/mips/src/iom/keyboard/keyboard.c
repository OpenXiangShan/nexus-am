int *key_addr = NULL;

void init_keyboard(){
	key_addr = KMEM_ADDR;
}

int get_keycode(){
	int key_code = key_addr[0];
	return key_code;
}
