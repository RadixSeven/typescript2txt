CC=g++
CFLAGS=-Wall -Wextra -g
CPPFLAGS=-Wall -Wextra -g

all: typescript2txt

typescript2txt: typescript2txt.o

tests/01_passed: ./typescript2txt tests/test_01_input.txt tests/test_01_expected_output.txt
	./typescript2txt < tests/test_01_input.txt > tests/test_01_actual_output.txt
	diff -q tests/test_01_expected_output.txt tests/test_01_actual_output.txt
	touch tests/01_passed

tests/02_passed: ./typescript2txt tests/test_02_tabs_input.txt tests/test_02_tabs_expected_output.txt
	./typescript2txt < tests/test_02_tabs_input.txt > tests/test_02_tabs_actual_output.txt
	diff -q tests/test_02_tabs_expected_output.txt tests/test_02_tabs_actual_output.txt
	touch tests/02_passed

tests/03_passed: ./typescript2txt tests/test_03_tabs_input.txt tests/test_03_tabs_expected_output.txt
	./typescript2txt < tests/test_03_tabs_input.txt > tests/test_03_tabs_actual_output.txt
	diff -q tests/test_03_tabs_expected_output.txt tests/test_03_tabs_actual_output.txt
	touch tests/03_passed

tests/04_passed: ./typescript2txt tests/test_04_esc_D_input.txt tests/test_04_esc_D_expected_output.txt
	./typescript2txt < tests/test_04_esc_D_input.txt > tests/test_04_esc_D_actual_output.txt
	diff -q tests/test_04_esc_D_expected_output.txt tests/test_04_esc_D_actual_output.txt
	touch tests/04_passed

tests/05_passed: ./typescript2txt tests/test_05_esc_E_input.txt tests/test_05_esc_E_expected_output.txt
	./typescript2txt < tests/test_05_esc_E_input.txt > tests/test_05_esc_E_actual_output.txt
	diff -q tests/test_05_esc_E_expected_output.txt tests/test_05_esc_E_actual_output.txt
	touch tests/05_passed

tests/06_passed: ./typescript2txt tests/test_06_esc_M_input.txt tests/test_06_esc_M_expected_output.txt
	./typescript2txt < tests/test_06_esc_M_input.txt > tests/test_06_esc_M_actual_output.txt
	diff -q tests/test_06_esc_M_expected_output.txt tests/test_06_esc_M_actual_output.txt
	touch tests/06_passed

test: tests/04_passed tests/05_passed tests/06_passed tests/03_passed 
test: tests/02_passed tests/01_passed 

clean:
	-rm -f *.o typescript2txt 

.PHONY: all clean test