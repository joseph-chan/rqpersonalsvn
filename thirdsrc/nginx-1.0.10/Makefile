
default:	build

clean:
	rm -rf Makefile objs

build:
	$(MAKE) -f objs/Makefile
	$(MAKE) -f objs/Makefile manpage

install:
	$(MAKE) -f objs/Makefile install

upgrade:
	/data4/ruqiang/projects/nginx//sbin/nginx -t

	kill -USR2 `cat /data4/ruqiang/projects/nginx//logs/nginx.pid`
	sleep 1
	test -f /data4/ruqiang/projects/nginx//logs/nginx.pid.oldbin

	kill -QUIT `cat /data4/ruqiang/projects/nginx//logs/nginx.pid.oldbin`
