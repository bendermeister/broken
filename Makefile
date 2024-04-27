FLAGS := -Wall -Werror -I include -std=c17
DEBUG_FLAGS := ${FLAGS} -g3 -fsanitize=address,leak,undefined,unreachable

EXAMPLES := example/unit.out

example: ${EXAMPLES}

example/%.out: example/%.c
	${CC} ${DEBUG_FLAGS} $< -o $@

clean:
	rm -f ${EXAMPLES}
