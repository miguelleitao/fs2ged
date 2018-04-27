
EXEC=fs2ged


TARGETS=${EXEC}

all: ${TARGETS}

${EXEC}: ${EXEC}.c
	${CC} ${CFLAGS} -o $@ $^

commit:
	git add *.c Makefile *.fs README.md
	git commit -m "new update"

push: commit
	git push

pull:
	git pull
	git submodule update --recursive --remote

