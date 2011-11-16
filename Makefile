CC=g++
CFLAGS=-Wall -Wextra -g
CPPFLAGS=-Wall -Wextra -g

all: typescript2txt

typescript2txt: typescript2txt.o

tests/01_passed: ./typescript2txt tests/01_input.txt tests/01_expected_output.txt
	./typescript2txt < tests/01_input.txt > tests/01_actual_output.txt
	diff -q tests/01_expected_output.txt tests/01_actual_output.txt
	touch tests/01_passed

tests/02_passed: ./typescript2txt tests/02_tabs_input.txt tests/02_tabs_expected_output.txt
	./typescript2txt < tests/02_tabs_input.txt > tests/02_tabs_actual_output.txt
	diff -q tests/02_tabs_expected_output.txt tests/02_tabs_actual_output.txt
	touch tests/02_passed

tests/03_passed: ./typescript2txt tests/03_tabs_input.txt tests/03_tabs_expected_output.txt
	./typescript2txt < tests/03_tabs_input.txt > tests/03_tabs_actual_output.txt
	diff -q tests/03_tabs_expected_output.txt tests/03_tabs_actual_output.txt
	touch tests/03_passed

tests/04_passed: ./typescript2txt tests/04_esc_D_input.txt tests/04_esc_D_expected_output.txt
	./typescript2txt < tests/04_esc_D_input.txt > tests/04_esc_D_actual_output.txt
	diff -q tests/04_esc_D_expected_output.txt tests/04_esc_D_actual_output.txt
	touch tests/04_passed

tests/05_passed: ./typescript2txt tests/05_esc_E_input.txt tests/05_esc_E_expected_output.txt
	./typescript2txt < tests/05_esc_E_input.txt > tests/05_esc_E_actual_output.txt
	diff -q tests/05_esc_E_expected_output.txt tests/05_esc_E_actual_output.txt
	touch tests/05_passed

tests/06_passed: ./typescript2txt tests/06_esc_M_input.txt tests/06_esc_M_expected_output.txt
	./typescript2txt < tests/06_esc_M_input.txt > tests/06_esc_M_actual_output.txt
	diff -q tests/06_esc_M_expected_output.txt tests/06_esc_M_actual_output.txt
	touch tests/06_passed

tests/07_passed: ./typescript2txt tests/07_OSC_0_setwi_input.txt tests/07_OSC_0_setwi_expected_output.txt
	./typescript2txt < tests/07_OSC_0_setwi_input.txt > tests/07_OSC_0_setwi_actual_output.txt
	diff -q tests/07_OSC_0_setwi_expected_output.txt tests/07_OSC_0_setwi_actual_output.txt
	touch tests/07_passed

test: tests/04_passed tests/05_passed tests/06_passed 
test: tests/07_passed
test: tests/03_passed tests/02_passed tests/01_passed 

clean:
	-rm -f *.o typescript2txt 

.PHONY: all clean test