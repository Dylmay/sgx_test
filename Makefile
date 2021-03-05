all:
	$(MAKE) -f sgx_u.mk all
	$(MAKE) -f sgx_t.mk all
	$(MAKE) -f sgx_e.mk all

clean:
	$(MAKE) -f sgx_u.mk clean
	$(MAKE) -f sgx_t.mk clean
	$(MAKE) -f sgx_e.mk clean

update_empty:
	$(MAKE) -f sgx_u.mk all
	$(MAKE) -f sgx_e.mk all