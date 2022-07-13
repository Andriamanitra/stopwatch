build: stopw
	gcc -O2 -Wall -Wextra -o stopw stopw.c

run: stopw
	./stopw

install: stopw
	cp stopw ~/.local/bin/stopw

uninstall:
	rm ~/.local/bin/stopw

clean:
	rm stopw
