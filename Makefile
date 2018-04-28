
EXEC=fs2ged


TARGETS=${EXEC}

CFLAGS=-Wall -Wextra -O1

all: ${TARGETS}

${EXEC}: ${EXEC}.c
	${CC} ${CFLAGS} -o $@ $^

test: ${EXEC}
	./${EXEC} example.fs >example.gedcom

commit:
	git add *.c Makefile *.fs README.md
	git commit -m "new update"

push: commit
	git push

pull:
	git pull
	git submodule update --recursive --remote

