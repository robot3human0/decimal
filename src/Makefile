SANIT = -O3
OS=$(shell uname)
ifeq ($(OS), Linux)
	LIBS=-lcheck -lpthread -lrt -lm -lsubunit
else
	LIBS=-lcheck
endif

all: test gcov_report

decimal.a: decimal.o
	ar rcs decimal.a decimal.o
	ranlib decimal.a

decimal.o: decimal.c
	gcc -c decimal.c

test: decimal.a
	gcc $(SANIT) --coverage tests.c decimal.a $(LIBS)
	./a.out

gcov_report:
	gcc --coverage decimal.c -c -o decimal_coverage.o
	gcc tests.c -c -o test_coverage.o
	gcc decimal_coverage.o test_coverage.o $(LIBS) -lgcov
	./a.out
	gcovr --html --html-details -o report.html
	open report.html

style_check:
	clang-format -style=google -i *.c
	clang-format -style=google -i *.h
	clang-format -style=google -n *.c
	clang-format -style=google -n *.h

clean:
	rm -rf *.o *.a *.gcno *.gcda *.out *.html *.css *.check

