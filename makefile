.PHONY:all
all:
	@cd compile_server;\
	make clean;\
	cd -
	@cd oj_server;\
	make clean;\
	cd -
.PHONY: release
release:
	@mkdir -p output/compile_server;\
	mkdir -p output/oj_server;\
	cp -rf compile_server/test_code output/compile_server;\
	cp -rf compile_server/tmp output/compile_server;\
	cp -rf compile_server/cs output/compile_server;\
	cp -rf oj_server/conf output/oj_server;\
	cp -rf oj_server/mysql output/oj_server;\
	cp -rf oj_server/template_html output/oj_server;\
	cp -rf oj_server/wwwroot output/oj_server;\
	cp -rf oj_server/oj output/oj_server
	